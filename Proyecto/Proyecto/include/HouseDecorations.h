#ifndef HOUSE_DECORATIONS_H
#define HOUSE_DECORATIONS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "shader_m.h"
#include "material.h"

class HouseDecorations {
public:
    unsigned int boxVAO = 0, boxVBO = 0;
    unsigned int sphereVAO = 0, sphereVBO = 0, sphereIndexCount = 0;

    HouseDecorations() {}

    void Init() {
        initBoxGeometry();
        initSphereGeometry();
        std::cout << "[HouseDecorations] Modulo de Mobiliario y Decoracion cargado con exito." << std::endl;
    }

    void initBoxGeometry() {
        float boxVertices[] = {
            // Posiciones          // Normales          // UV
            // Trasera
            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
            // Frontal
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
            // Izquierda
            -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            // Derecha
             0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            // Abajo
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
            // Arriba
            -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 1.0f
        };
        glGenVertexArrays(1, &boxVAO);
        glGenBuffers(1, &boxVBO);
        glBindVertexArray(boxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);
    }

    void initSphereGeometry() {
        std::vector<float> sphVerts;
        std::vector<unsigned int> sphIndices;
        const int rings = 12, sectors = 16;
        for (int r = 0; r <= rings; ++r) {
            float phi = (float)r / (float)rings * 3.14159265f;
            for (int s = 0; s <= sectors; ++s) {
                float theta = (float)s / (float)sectors * 2.0f * 3.14159265f;
                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);
                sphVerts.push_back(x * 0.5f); sphVerts.push_back(y * 0.5f); sphVerts.push_back(z * 0.5f);
                sphVerts.push_back(x); sphVerts.push_back(y); sphVerts.push_back(z);
                sphVerts.push_back((float)s / (float)sectors); sphVerts.push_back((float)r / (float)rings);
            }
        }
        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                int cur = r * (sectors + 1) + s;
                int next = cur + sectors + 1;
                sphIndices.push_back(cur); sphIndices.push_back(next); sphIndices.push_back(cur + 1);
                sphIndices.push_back(cur + 1); sphIndices.push_back(next); sphIndices.push_back(next + 1);
            }
        }
        sphereIndexCount = (unsigned int)sphIndices.size();

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        unsigned int sphEBO;
        glGenBuffers(1, &sphEBO);
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, sphVerts.size() * sizeof(float), sphVerts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphIndices.size() * sizeof(unsigned int), sphIndices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);
    }

    void drawBox(Shader& shader, const glm::mat4& model, const glm::vec4& diffColor, int matType = 0, float shininess = 32.0f, float transparency = 1.0f) {
        shader.setMat4("model", model);
        shader.setVec4("MaterialAmbientColor", diffColor * 0.35f);
        shader.setVec4("MaterialDiffuseColor", diffColor);
        shader.setVec4("MaterialSpecularColor", (matType == 2) ? (diffColor * 1.5f) : glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        shader.setInt("materialType", matType);
        shader.setFloat("shininess", shininess);
        shader.setFloat("transparency", transparency);

        glBindVertexArray(boxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void drawSphere(Shader& shader, const glm::mat4& model, const glm::vec4& diffColor, int matType = 0, float shininess = 32.0f) {
        shader.setMat4("model", model);
        shader.setVec4("MaterialAmbientColor", diffColor * 0.4f);
        shader.setVec4("MaterialDiffuseColor", diffColor);
        shader.setVec4("MaterialSpecularColor", glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
        shader.setInt("materialType", matType);
        shader.setFloat("shininess", shininess);
        shader.setFloat("transparency", 1.0f);

        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Draw(Shader& shader, float globalTime, const std::vector<bool>& roomLightStates) {
        // =========================================================================
        // 1. SALA PRINCIPAL
        // =========================================================================
        // Sofá seccional chaise longue
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 0.22f, 2.8f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.8f, 0.44f, 0.85f)),
                glm::vec4(0.35f, 0.42f, 0.48f, 1.0f), 0, 16.0f);
        // Respaldo de sofá
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 0.60f, 3.15f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.8f, 0.45f, 0.20f)),
                glm::vec4(0.30f, 0.38f, 0.44f, 1.0f), 0, 16.0f);
        // Mesa de centro con superficie de madera (TIPO_OPACO)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.1f, 0.20f, 1.9f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.95f, 0.08f, 0.60f)),
                glm::vec4(0.55f, 0.32f, 0.18f, 1.0f), 0, 24.0f);
        // Patas metálicas de la mesa (TIPO_METALICO)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.45f, 0.10f, 1.70f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.20f, 0.04f)),
                glm::vec4(0.20f, 0.20f, 0.22f, 1.0f), 2, 128.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.75f, 0.10f, 2.10f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.20f, 0.04f)),
                glm::vec4(0.20f, 0.20f, 0.22f, 1.0f), 2, 128.0f);
        // Alfombra gris de sala
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.1f, 0.005f, 2.1f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.2f, 0.01f, 1.6f)),
                glm::vec4(0.68f, 0.68f, 0.65f, 1.0f), 0, 8.0f);

        // Estantería de libros en la sala
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.85f, 0.90f, 2.50f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 1.60f, 0.90f)),
                glm::vec4(0.40f, 0.24f, 0.14f, 1.0f), 0, 16.0f);
        // Libros decorativos de colores (Rojo, Azul, Verde)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.80f, 0.85f, 2.30f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.18f, 0.22f, 0.08f)),
                glm::vec4(0.85f, 0.18f, 0.18f, 1.0f), 1, 64.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.80f, 0.85f, 2.45f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.18f, 0.20f, 0.08f)),
                glm::vec4(0.18f, 0.40f, 0.85f, 1.0f), 1, 64.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.80f, 0.85f, 2.60f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.18f, 0.24f, 0.08f)),
                glm::vec4(0.20f, 0.75f, 0.25f, 1.0f), 1, 64.0f);

        // =========================================================================
        // 2. COMEDOR
        // =========================================================================
        // Mesa de comedor amplia
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.8f, 0.74f, -0.6f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.6f, 0.08f, 0.95f)),
                glm::vec4(0.50f, 0.30f, 0.16f, 1.0f), 0, 24.0f);
        // 4 Patas de la mesa de comedor
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.50f, 0.35f, -1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.70f, 0.08f)),
                glm::vec4(0.40f, 0.24f, 0.14f, 1.0f), 0, 16.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.10f, 0.35f, -1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.70f, 0.08f)),
                glm::vec4(0.40f, 0.24f, 0.14f, 1.0f), 0, 16.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.50f, 0.35f, -0.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.70f, 0.08f)),
                glm::vec4(0.40f, 0.24f, 0.14f, 1.0f), 0, 16.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.10f, 0.35f, -0.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.70f, 0.08f)),
                glm::vec4(0.40f, 0.24f, 0.14f, 1.0f), 0, 16.0f);
        // Sillas de comedor
        for (int c = 0; c < 2; ++c) {
            float zOffset = (c == 0) ? -1.30f : 0.10f;
            drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.2f, 0.44f, zOffset)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.42f, 0.05f, 0.42f)),
                    glm::vec4(0.60f, 0.55f, 0.50f, 1.0f), 1, 64.0f);
            drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.4f, 0.44f, zOffset)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.42f, 0.05f, 0.42f)),
                    glm::vec4(0.60f, 0.55f, 0.50f, 1.0f), 1, 64.0f);
        }

        // =========================================================================
        // 3. SALA DE TELEVISIÓN
        // =========================================================================
        // Mueble credenza / Centro de entretenimiento
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.85f, 0.35f, -2.60f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.28f, 0.65f, 1.60f)),
                glm::vec4(0.32f, 0.20f, 0.12f, 1.0f), 0, 24.0f);
        // Pantalla de TV plana (TIPO_PLASTICO negro brillante)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.80f, 1.05f, -2.60f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.70f, 1.30f)),
                glm::vec4(0.05f, 0.05f, 0.06f, 1.0f), 1, 128.0f);
        // Dos bocinas de torre laterales
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.85f, 0.50f, -3.55f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.22f, 0.95f, 0.22f)),
                glm::vec4(0.10f, 0.10f, 0.12f, 1.0f), 1, 80.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.85f, 0.50f, -1.65f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.22f, 0.95f, 0.22f)),
                glm::vec4(0.10f, 0.10f, 0.12f, 1.0f), 1, 80.0f);

        // =========================================================================
        // 4. COCINA
        // =========================================================================
        // Isla de cocina con superficie metálica de acero inoxidable (TIPO_METALICO)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.4f, 0.42f, -2.10f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.10f, 0.84f, 0.65f)),
                glm::vec4(0.25f, 0.26f, 0.28f, 1.0f), 0, 16.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.4f, 0.85f, -2.10f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.15f, 0.04f, 0.70f)),
                glm::vec4(0.75f, 0.75f, 0.78f, 1.0f), 2, 128.0f); // Acero inoxidable
        // Refrigerador alto moderno
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.65f, 1.05f, -4.40f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.65f, 2.05f, 0.65f)),
                glm::vec4(0.85f, 0.85f, 0.87f, 1.0f), 2, 96.0f);
        // Manija vertical cromada del refrigerador
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.30f, 1.10f, -4.20f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.50f, 0.04f)),
                glm::vec4(0.95f, 0.95f, 0.95f, 1.0f), 2, 128.0f);
        // Horno de microondas sobre mostrador
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-2.60f, 0.98f, -4.45f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.55f, 0.32f, 0.35f)),
                glm::vec4(0.18f, 0.18f, 0.20f, 1.0f), 1, 96.0f);

        // =========================================================================
        // 5. HABITACIÓN 1 (Principal)
        // =========================================================================
        // Cama matrimonial: base de madera
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.22f, 3.1f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.7f, 0.44f, 1.9f)),
                glm::vec4(0.42f, 0.25f, 0.15f, 1.0f), 0, 16.0f);
        // Colchón blanco suave
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.48f, 3.1f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.6f, 0.18f, 1.8f)),
                glm::vec4(0.92f, 0.92f, 0.94f, 1.0f), 0, 12.0f);
        // Almohadas
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(2.25f, 0.60f, 3.75f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.55f, 0.10f, 0.35f)),
                glm::vec4(0.95f, 0.95f, 0.95f, 1.0f), 0, 8.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(2.95f, 0.60f, 3.75f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.55f, 0.10f, 0.35f)),
                glm::vec4(0.95f, 0.95f, 0.95f, 1.0f), 0, 8.0f);
        // Escritorio de estudio con laptop frente a la ventana
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.72f, 3.4f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.10f, 0.06f, 0.60f)),
                glm::vec4(0.52f, 0.34f, 0.20f, 1.0f), 0, 24.0f);
        // Base de laptop
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.77f, 3.35f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.32f, 0.02f, 0.22f)),
                glm::vec4(0.75f, 0.75f, 0.78f, 1.0f), 2, 128.0f);

        // =========================================================================
        // 6. HABITACIÓN 2
        // =========================================================================
        // Cama individual con colchón gris claro
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(3.6f, 0.22f, -3.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 0.44f, 1.8f)),
                glm::vec4(0.38f, 0.22f, 0.14f, 1.0f), 0, 16.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(3.6f, 0.48f, -3.2f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.1f, 0.18f, 1.7f)),
                glm::vec4(0.78f, 0.80f, 0.84f, 1.0f), 0, 12.0f);
        // Clóset armario de madera
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(2.15f, 1.05f, -4.20f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.95f, 2.05f, 0.55f)),
                glm::vec4(0.44f, 0.28f, 0.16f, 1.0f), 0, 20.0f);

        // =========================================================================
        // 7. BAÑO
        // =========================================================================
        // Bañera de porcelana blanca (TIPO_PLASTICO / CERAMICA)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(4.25f, 0.28f, 0.35f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.40f, 0.55f, 0.75f)),
                glm::vec4(0.94f, 0.94f, 0.96f, 1.0f), 1, 96.0f);
        // Superficie de agua en la bañera con Fresnel y simulación de ondas procedurales (Rúbrica: Traslúcido con Fresnel)
        float waveOffset = sin(globalTime * 2.5f) * 0.008f;
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(4.25f, 0.44f + waveOffset, 0.35f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 0.02f, 0.62f)),
                glm::vec4(0.20f, 0.60f, 0.85f, 0.45f), 3, 128.0f, 0.45f); // TRASLUCIDO FRESNEL
        // Tocador de madera y lavabo
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(4.55f, 0.42f, -0.65f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.55f, 0.84f, 0.80f)),
                glm::vec4(0.88f, 0.88f, 0.90f, 1.0f), 1, 64.0f);
        // Grifo metálico cromado (TIPO_METALICO)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(4.70f, 0.92f, -0.65f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.06f, 0.16f, 0.06f)),
                glm::vec4(0.90f, 0.90f, 0.95f, 1.0f), 2, 128.0f);
        // Espejo alto en la pared (Reflectivo metálico / espejo)
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(4.90f, 1.45f, -0.65f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.03f, 0.75f, 0.60f)),
                glm::vec4(0.85f, 0.90f, 0.95f, 1.0f), 2, 128.0f);
        // Taza de inodoro completa con tanque y base
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(3.60f, 0.22f, -0.75f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.40f, 0.44f, 0.50f)),
                glm::vec4(0.95f, 0.95f, 0.96f, 1.0f), 1, 96.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(3.60f, 0.55f, -0.92f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.40f, 0.45f, 0.18f)),
                glm::vec4(0.95f, 0.95f, 0.96f, 1.0f), 1, 96.0f);

        // =========================================================================
        // 8. PASILLO INTERIOR Y JARDÍN
        // =========================================================================
        // Cuadro artístico en la pared del pasillo
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-4.95f, 1.45f, -0.60f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.65f, 0.90f)),
                glm::vec4(0.48f, 0.32f, 0.18f, 1.0f), 0, 32.0f); // Marco
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-4.92f, 1.45f, -0.60f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.55f, 0.80f)),
                glm::vec4(0.20f, 0.55f, 0.45f, 1.0f), 0, 16.0f); // Lienzo
        // Banca de jardín con estructura metálica y listones de madera
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.20f, 0.42f, -6.20f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.30f, 0.08f, 0.45f)),
                glm::vec4(0.65f, 0.38f, 0.18f, 1.0f), 0, 24.0f);
        drawBox(shader, glm::translate(glm::mat4(1.0f), glm::vec3(-3.20f, 0.70f, -6.40f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.30f, 0.45f, 0.08f)),
                glm::vec4(0.65f, 0.38f, 0.18f, 1.0f), 0, 24.0f);

        // =========================================================================
        // 9. LÁMPARAS DE TECHO Y FOCOS COLGANTES (Con bombilla brillante reactiva)
        // =========================================================================
        struct CeilingLampInfo {
            glm::vec3 pos;
            size_t lightIdx;
        };
        CeilingLampInfo lamps[] = {
            { glm::vec3(0.0f, 2.45f, 0.0f), 1 },     // Sala
            { glm::vec3(-1.5f, 2.45f, 2.8f), 2 },    // Cocina
            { glm::vec3(5.0f, 2.45f, 0.0f), 3 },     // Baño
            { glm::vec3(5.0f, 2.45f, 3.2f), 4 },     // Recámara
            { glm::vec3(-5.0f, 2.45f, -2.0f), 5 }    // Terraza
        };

        for (int l = 0; l < 5; ++l) {
            bool isOn = (l + 1 < roomLightStates.size()) ? roomLightStates[l + 1] : true;
            // Cable negro
            drawBox(shader, glm::translate(glm::mat4(1.0f), lamps[l].pos + glm::vec3(0.0f, 0.20f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.40f, 0.02f)),
                    glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), 0, 8.0f);
            // Campana / pantalla metálica
            drawBox(shader, glm::translate(glm::mat4(1.0f), lamps[l].pos + glm::vec3(0.0f, 0.02f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.24f, 0.08f, 0.24f)),
                    glm::vec4(0.18f, 0.18f, 0.20f, 1.0f), 2, 96.0f);
            // Bombilla emisiva
            glm::vec4 bulbCol = isOn ? glm::vec4(1.0f, 0.95f, 0.65f, 1.0f) : glm::vec4(0.25f, 0.25f, 0.25f, 1.0f);
            glm::mat4 bulbMat = glm::translate(glm::mat4(1.0f), lamps[l].pos - glm::vec3(0.0f, 0.06f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.14f));
            drawSphere(shader, bulbMat, bulbCol, 1, 128.0f);
        }
    }
};

#endif
