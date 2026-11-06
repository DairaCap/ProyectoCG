#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader.h>
#include "material.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;

// Funciones auxiliares para discriminación estricta de materiales
inline bool IsContactMaterial(const std::string& matName, const std::string& nodeName = "") {
    std::string s = matName;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    std::string n = nodeName;
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return (s.find("contacto") != std::string::npos || n.find("contacto") != std::string::npos);
}

inline bool IsCollisionMaterial(const std::string& matName) {
    if (IsContactMaterial(matName)) return false;
    std::string s = matName;
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    if (s.find("colisi") != std::string::npos || s.find("colici") != std::string::npos) return true;
    if (s.find("caja") != std::string::npos && (s.find("col") != std::string::npos)) return true;
    if (s == "mat_colision" || s == "caja_colision" || s == "caja_colisión") return true;
    return false;
}

#define MAX_NUM_BONES 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    glm::vec4 IDs1;
    glm::vec4 IDs2;
    glm::vec4 IDs3;
    glm::vec4 Weights1; 
    glm::vec4 Weights2;
    glm::vec4 Weights3;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    /* Mesh Data */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    Material material;
    string materialName;
    string name;
    string nodeName;
    glm::mat4 nodeTransform;
    bool isCollision;
    unsigned int VAO;

    /* Constructor */
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, 
         Material mat = Material(), string matName = "", string meshName = "", 
         string nName = "", glm::mat4 transform = glm::mat4(1.0f), bool collision = false)
        : vertices(vertices), indices(indices), textures(textures), material(mat),
          materialName(matName), name(meshName), nodeName(nName), nodeTransform(transform),
          isCollision(collision)
    {
        // Solo configurar buffers VAO/VBO si NO es colisionador ni cubo de contacto
        if (!isCollision && !IsCollisionMaterial(materialName) && !IsContactMaterial(materialName, nName)) {
            setupMesh();
        } else {
            VAO = 0;
            VBO = 0;
            EBO = 0;
        }
    }

    // Renderizado de la malla
    void UpdateVertices(const vector<Vertex>& newVertices)
    {
        if (VAO == 0 || VBO == 0) return;
        this->vertices = newVertices;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Draw(Shader shader) 
    {
        // Evaluación estricta: Bajo ninguna circunstancia se debe ejecutar
        // glBindVertexArray ni glDrawElements para los colisionadores ni contactos invisibles
        if (isCollision || IsCollisionMaterial(materialName) || IsContactMaterial(materialName, nodeName) || VAO == 0)
            return;

        // 1. Configurar propiedades del material en los uniforms del shader
        shader.setVec4("MaterialAmbientColor", material.ambient);
        shader.setVec4("MaterialDiffuseColor", material.diffuse);
        shader.setVec4("MaterialSpecularColor", material.specular);
        shader.setFloat("transparency", material.transparency);
        shader.setInt("materialType", material.type);
        shader.setFloat("shininess", material.shininess);

        // 2. Textura 1x1 blanca estática de respaldo para mallas sin textura difusa
        static unsigned int defaultWhiteTex = 0;
        if (defaultWhiteTex == 0) {
            glGenTextures(1, &defaultWhiteTex);
            glBindTexture(GL_TEXTURE_2D, defaultWhiteTex);
            unsigned char white[4] = { 255, 255, 255, 255 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        // 3. Vincular texturas apropiadas
        bool hasDiffuse = false;
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            string number;
            string type = textures[i].type;
            if(type == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
                hasDiffuse = true;
            }
            else if(type == "texture_specular")
                number = std::to_string(specularNr++);
            else if(type == "texture_normal")
                number = std::to_string(normalNr++);
            else if(type == "texture_height")
                number = std::to_string(heightNr++);

            glUniform1i(glGetUniformLocation(shader.ID, (type + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        
        // Si el objeto no tiene textura difusa (paredes, marcos, metales),
        // vinculamos una textura 1x1 blanca a GL_TEXTURE0
        if (!hasDiffuse)
        {
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse1"), 0);
            glBindTexture(GL_TEXTURE_2D, defaultWhiteTex);
        }

        // Dibujar elementos
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
    }
    // Función pública para actualizar los vértices deformados (CPU Skinning)
    void UpdateSkinningVBO(const vector<Vertex>& animatedVertices) {
        if (VBO != 0 && animatedVertices.size() == vertices.size()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, animatedVertices.size() * sizeof(Vertex), &animatedVertices[0]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

private:
    /* Render data */
    unsigned int VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Atributos de vértice
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, IDs1));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, IDs2));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, IDs3));

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights1));
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights2));
        glEnableVertexAttribArray(10);
        glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights3));

        glBindVertexArray(0);
    }
};

#endif
