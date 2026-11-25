#pragma once
// ============================================================================
// Aura.h - Circulos luminosos en el piso (usados por el minijuego de plantas:
// verde = bien colocada, roja = mal colocada, crema = lugar donde va la planta que cargas).
// ============================================================================
#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"

struct AuraInfo {
    glm::vec3 pos;       // Centro, en el piso
    float radio;         // m
    glm::vec4 color;
};

class Auras {
public:
    bool Init() {
        shader = new Shader("shaders/aura.vs", "shaders/aura.fs");
        float quad[] = { 0,0,  1,0,  1,1,   0,0,  1,1,  0,1 };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);
        return true;
    }

    // Dibuja las auras en el mundo 3D (llamar despues de dibujar la casa). No escriben profundidad.
    void Draw(const glm::mat4& projection, const glm::mat4& view, const std::vector<AuraInfo>& lista, float tiempo) {
        if (!shader || lista.empty()) return;
        GLboolean habiaCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);                              // El cuadro horizontal se ve por ambos lados
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setFloat("uTime", tiempo);
        glBindVertexArray(vao);
        for (const AuraInfo& a : lista) {
            shader->setVec3("uCenter", a.pos + glm::vec3(0.0f, 0.03f, 0.0f));   // Un poco arriba del piso para que no parpadee
            shader->setFloat("uRadio", a.radio);
            shader->setVec4("uColor", a.color);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        if (habiaCull) glEnable(GL_CULL_FACE);
    }

private:
    Shader* shader = nullptr;
    GLuint vao = 0, vbo = 0;
};
