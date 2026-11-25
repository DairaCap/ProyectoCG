#pragma once
// ============================================================================
// Checkpoint.h - Pins visuales (como los de Google Maps) que marcan donde se puede
// interactuar. Cada checkpoint es un pin que flota y siempre mira al jugador
// (imagen minijuegos/pin.png) que flota y sube y baja suavemente.
//
// Cuando el jugador esta cerca pero todavia fuera de la zona de interaccion, se muestra el aviso
// "Acercate al checkpoint para interactuar".
// ============================================================================
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cmath>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "UI.h"

struct Checkpoint {
    std::string id;                    // Nombre interno ("trivia", "plantas"...)
    glm::vec3 pos;                     // Punto en el piso bajo el pin (x, y del piso, z)
    float radioInteraccion = 1.5f;     // Dentro de este radio (m, invisible) se puede interactuar
    float radioAviso = 6.0f;           // Entre este radio y el anterior sale el aviso "Acercate"
};

class Checkpoints {
public:
    std::vector<Checkpoint> lista;

    // Carga el shader del pin, la imagen y el cuadro unitario
    bool Init(const std::string& carpetaRecursos) {
        shader = new Shader("shaders/pin.vs", "shaders/pin.fs");
        texPin = UI::CargarTextura(carpetaRecursos + "/pin.png", &aspectoPin);
        float quad[] = { 0,0,  1,0,  1,1,   0,0,  1,1,  0,1 };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);
        return texPin != 0;
    }

    // Agrega un checkpoint y regresa su indice en la lista
    int Agregar(const std::string& id, const glm::vec3& pos, float radioInteraccion = 1.5f, float radioAviso = 6.0f) {
        Checkpoint c; c.id = id; c.pos = pos; c.radioInteraccion = radioInteraccion; c.radioAviso = radioAviso;
        lista.push_back(c);
        return (int)lista.size() - 1;
    }

    // Distancia horizontal (m) del jugador al checkpoint i
    float Distancia(size_t i, const glm::vec3& jugador) const {
        glm::vec2 d(jugador.x - lista[i].pos.x, jugador.z - lista[i].pos.z);
        return glm::length(d);
    }

    // Dibuja todos los pins en el mundo 3D (llamar despues de dibujar la casa)
    void Draw(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& derechaCamara, float tiempo) {
        if (!shader || lista.empty()) return;
        GLboolean habiaCull = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);                              // Los cuadros se ven por ambos lados
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setInt("uTex", 0);
        glm::vec3 derecha(derechaCamara.x, 0.0f, derechaCamara.z);       // Solo giro horizontal: el pin no se acuesta
        derecha = (glm::length(derecha) > 1e-4f) ? glm::normalize(derecha) : glm::vec3(1, 0, 0);
        shader->setVec3("uRight", derecha);
        glBindVertexArray(vao);

        for (const Checkpoint& c : lista) {
            // Pin: flota y sube y baja suavemente
            float flota = 0.95f + 0.07f * std::sin(tiempo * 2.2f);
            float alto = 0.60f, ancho = alto * aspectoPin;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texPin);
            shader->setVec3("uCenter", c.pos + glm::vec3(0.0f, flota, 0.0f));
            shader->setVec2("uSize", glm::vec2(ancho, alto));
            shader->setVec4("uColor", glm::vec4(1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);
        if (habiaCull) glEnable(GL_CULL_FACE);
    }

    // Busca un checkpoint cercano que valga la pena avisar: dentro del radio de aviso, fuera del radio de
    // interaccion y (si esta lejos) hacia donde mira el jugador. Regresa su indice o -1.
    int CheckpointParaAvisar(const glm::vec3& jugador, const glm::vec3& frente, float& distancia) const {
        int mejor = -1;
        float mejorDist = 1e9f;
        glm::vec2 f(frente.x, frente.z);
        f = (glm::length(f) > 1e-4f) ? glm::normalize(f) : glm::vec2(0, -1);
        for (size_t i = 0; i < lista.size(); ++i) {
            float d = Distancia(i, jugador);
            if (d <= lista[i].radioInteraccion || d > lista[i].radioAviso) continue;
            glm::vec2 haciaPin = glm::normalize(glm::vec2(lista[i].pos.x - jugador.x, lista[i].pos.z - jugador.z));
            bool loVe = glm::dot(f, haciaPin) > 0.25f;                   // Esta delante del jugador
            if (!loVe && d > 3.5f) continue;                             // Muy cerca se avisa aunque no lo mire
            if (d < mejorDist) { mejorDist = d; mejor = (int)i; }
        }
        distancia = mejorDist;
        return mejor;
    }

    // Aviso a la derecha de la pantalla: "Acercate al checkpoint para interactuar"
    void DibujarAviso(UI& ui, float distancia) {
        const float pw = 430.0f, margen = 24.0f;
        float x = ui.W - pw - margen, y = 150.0f;
        float xTexto = x + 90.0f, anchoTexto = pw - 90.0f - 18.0f;
        // Primero se mide el texto (sin dibujar) para saber qué tan alto debe ser el panel
        float fin = ui.TextoEnvuelto("¡ACÉRCATE AL CHECKPOINT PARA INTERACTUAR!", xTexto, y + 16.0f, anchoTexto, 3.0f, UI::Crema(), false);
        float alto = (fin - y) + 4.0f + 14.0f + 16.0f;                   // Texto + renglon de distancia + margen
        ui.Panel(x, y, pw, alto);
        ui.Imagen(texPin, x + 18.0f, y + (alto - 63.0f) * 0.5f, 42.0f, 63.0f);            // Mini pin (aspecto 2:3)
        ui.TextoEnvuelto("¡ACÉRCATE AL CHECKPOINT PARA INTERACTUAR!", xTexto, y + 16.0f, anchoTexto, 3.0f, UI::Crema());
        char buf[48];
        snprintf(buf, sizeof(buf), "A %.1f METROS", distancia);
        ui.Texto(buf, xTexto, fin + 4.0f, 2.0f, UI::Verde());
    }

private:
    Shader* shader = nullptr;
    GLuint vao = 0, vbo = 0, texPin = 0;
    float aspectoPin = 0.667f;                                           // Ancho / alto de pin.png (587 x 880)
};
