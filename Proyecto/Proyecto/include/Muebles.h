// Muebles.h - Muebles, plantas y objetos de Romero (carpeta muebles/) para el programa de la casa.
//
// Carga muebles/Muebles.fbx con la clase Model del equipo y le agrega:
//   * animaciones por pieza (puertas de clóset y refri, cajones, tapa de laptop, puerta de estufa, plato
//     giratorio del microondas), leídas de muebles/animaciones.csv. Cada fila trae el pivote y el eje
//     en ejes de OpenGL (Y arriba), generados desde Blender con exportar_muebles.py.
//   * cajas de interacción (grupos.csv) para apuntarles con la mira y abrir/cerrar con clic o E.
//   * cajas de colisión (colisiones.csv) que se suman a las de la casa.
#ifndef MUEBLES_H
#define MUEBLES_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <stb_image.h>

#include "model.h"
#include "AABB.h"

// Una pieza que se mueve (por ejemplo "Closet_H1__Closet_PuertaIzq")
struct PiezaAnimada {
    std::string nodo;       // Nombre del nodo en el FBX
    std::string grupo;      // Mueble al que pertenece (todas sus piezas se mueven juntas)
    std::string tipo;       // "rotacion", "traslacion" o "giro_continuo"
    glm::vec3 pivote;       // Bisagra, en el mundo
    glm::vec3 eje;          // Eje de giro o dirección de deslizamiento
    float valor;            // Grados (rotación) o metros (traslación) con el mueble "abierto"
};

// Un mueble con animación (su caja sirve para apuntarle y para saber si está abierto)
struct GrupoMueble {
    std::string nombre;
    AABB caja;
    bool abierto = false;
    float progreso = 0.0f;  // 0 = estado inicial, 1 = estado alterno
};

// Planta con tarjeta informativa (imagen en muebles/tarjetas/N.png)
struct PlantaTarjeta {
    std::string nombre;     // Por ejemplo "Planta_Rosal"
    int tarjeta;            // Numero de la imagen
    AABB caja;              // Para apuntarle con la mira
    GLuint textura = 0;     // Se carga al iniciar
    float aspecto = 0.45f;  // ancho / alto de la imagen
};

// Una pista de keyframes: valores (grados o escala) en tiempos fijos, interpolados linealmente
struct PistaKeyframes {
    std::string tipo;                           // "rotacion" o "escala"
    glm::vec3 pivote, eje;
    std::vector<std::pair<float, float>> claves; // (segundos, valor)
};

// Movimiento por fórmula (seno) de una pieza: plantas con viento, silla que se balancea
struct Procedural {
    std::string tipo;       // "viento" o "balanceo"
    glm::vec3 pivote;       // Base de la pieza
    float altura, fase;
};

// Secuencia por keyframes que se dispara con clic (grifo del fregadero)
struct ClipKeyframes {
    std::string nombre;
    AABB caja;
    float duracion = 0.0f;
    bool activo = false;
    float t = 0.0f;
};

class Muebles {
public:
    std::map<std::string, PistaKeyframes> pistas;     // Por nombre de nodo
    std::map<std::string, Procedural> procedurales;   // Por nombre de nodo
    std::vector<ClipKeyframes> clips;

    std::vector<PlantaTarjeta> plantas;
    int tarjetaActiva = -1;         // Indice de la planta cuya tarjeta se muestra (-1 = ninguna)
    GLuint tarjetaVAO = 0, tarjetaVBO = 0;

    Model* modelo = nullptr;
    std::vector<PiezaAnimada> piezas;
    std::vector<GrupoMueble> grupos;
    std::vector<AABB> colisiones;
    float tiempo = 0.0f;

    // Carga todo desde una carpeta (por ejemplo "muebles"). Regresa false si no la encuentra.
    bool Cargar(const std::string& carpeta) {
        std::ifstream prueba(carpeta + "/Muebles.fbx", std::ios::binary);
        if (!prueba) { std::cout << "[Muebles] No se encontro " << carpeta << "/Muebles.fbx" << std::endl; return false; }
        prueba.close();

        modelo = new Model(carpeta + "/Muebles.fbx", true);      // true = el FBX viene con Z arriba, igual que la casa

        for (auto& f : LeerCSV(carpeta + "/animaciones.csv")) {
            if (f.size() < 10) continue;
            PiezaAnimada p;
            p.nodo = f[0]; p.grupo = f[1]; p.tipo = f[2];
            p.pivote = glm::vec3(std::stof(f[3]), std::stof(f[4]), std::stof(f[5]));
            p.eje = glm::vec3(std::stof(f[6]), std::stof(f[7]), std::stof(f[8]));
            p.valor = std::stof(f[9]);
            piezas.push_back(p);
        }
        for (auto& f : LeerCSV(carpeta + "/grupos.csv")) {
            if (f.size() < 7) continue;
            GrupoMueble g;
            g.nombre = f[0];
            g.caja = AABB(glm::vec3(std::stof(f[1]), std::stof(f[2]), std::stof(f[3])),
                          glm::vec3(std::stof(f[4]), std::stof(f[5]), std::stof(f[6])));
            grupos.push_back(g);
        }
        for (auto& f : LeerCSV(carpeta + "/colisiones.csv")) {
            if (f.size() < 7) continue;
            colisiones.push_back(AABB(glm::vec3(std::stof(f[1]), std::stof(f[2]), std::stof(f[3])),
                                      glm::vec3(std::stof(f[4]), std::stof(f[5]), std::stof(f[6]))));
        }
        for (auto& f : LeerCSV(carpeta + "/plantas.csv")) {
            if (f.size() < 8) continue;
            PlantaTarjeta pl;
            pl.nombre = f[0]; pl.tarjeta = std::stoi(f[1]);
            pl.caja = AABB(glm::vec3(std::stof(f[2]), std::stof(f[3]), std::stof(f[4])),
                           glm::vec3(std::stof(f[5]), std::stof(f[6]), std::stof(f[7])));
            pl.textura = CargarImagen(carpeta + "/tarjetas/" + f[1] + ".png", pl.aspecto);
            plantas.push_back(pl);
        }
        for (auto& f : LeerCSV(carpeta + "/keyframes.csv")) {      // Keyframes del grifo
            if (f.size() < 10) continue;
            PistaKeyframes& pk = pistas[f[0]];
            pk.tipo = f[1];
            pk.pivote = glm::vec3(std::stof(f[2]), std::stof(f[3]), std::stof(f[4]));
            pk.eje = glm::vec3(std::stof(f[5]), std::stof(f[6]), std::stof(f[7]));
            pk.claves.push_back({ std::stof(f[8]), std::stof(f[9]) });
        }
        for (auto& f : LeerCSV(carpeta + "/clips.csv")) {
            if (f.size() < 8) continue;
            ClipKeyframes c;
            c.nombre = f[0]; c.duracion = std::stof(f[1]);
            c.caja = AABB(glm::vec3(std::stof(f[2]), std::stof(f[3]), std::stof(f[4])),
                          glm::vec3(std::stof(f[5]), std::stof(f[6]), std::stof(f[7])));
            clips.push_back(c);
        }
        for (auto& f : LeerCSV(carpeta + "/procedurales.csv")) {   // Viento en plantas y balanceo de sillas
            if (f.size() < 7) continue;
            Procedural pr;
            pr.tipo = f[1];
            pr.pivote = glm::vec3(std::stof(f[2]), std::stof(f[3]), std::stof(f[4]));
            pr.altura = std::stof(f[5]); pr.fase = std::stof(f[6]);
            procedurales[f[0]] = pr;
        }
        // Rectangulo unitario (2 triangulos) para dibujar la tarjeta en pantalla
        float quad[] = { 0,0, 1,0, 1,1,  0,0, 1,1, 0,1 };
        glGenVertexArrays(1, &tarjetaVAO);
        glGenBuffers(1, &tarjetaVBO);
        glBindVertexArray(tarjetaVAO);
        glBindBuffer(GL_ARRAY_BUFFER, tarjetaVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);

        std::cout << "[Muebles] " << plantas.size() << " plantas con tarjeta | " << modelo->meshes.size() << " mallas | " << piezas.size()
                  << " piezas animadas | " << grupos.size() << " muebles interactivos | "
                  << colisiones.size() << " colisiones" << std::endl;
        return true;
    }

    // Abre o cierra un mueble (todas sus piezas)
    void Toggle(size_t g) {
        if (g >= grupos.size()) return;
        grupos[g].abierto = !grupos[g].abierto;
        std::cout << "[Interaccion] " << grupos[g].nombre << ": " << (grupos[g].abierto ? "ABIERTO" : "CERRADO") << std::endl;
    }

    // Avanza las animaciones suavemente hacia su estado objetivo
    void Update(float dt) {
        tiempo += dt;
        for (auto& g : grupos) {
            float objetivo = g.abierto ? 1.0f : 0.0f;
            g.progreso += (objetivo - g.progreso) * std::min(dt * 5.0f, 1.0f);
        }
        for (auto& c : clips) {                     // Avanza la secuencia de keyframes hasta el final
            if (!c.activo) continue;
            c.t += dt;
            if (c.t >= c.duracion) { c.activo = false; c.t = 0.0f; }
        }
    }

    // Texto para mostrar al apuntar (por ejemplo "Closet H1")
    std::string Nombre(size_t g) const {
        std::string n = grupos[g].nombre;
        std::replace(n.begin(), n.end(), '_', ' ');
        return n;
    }

    void Draw(Shader& shader) {
        if (!modelo) return;
        for (auto& m : modelo->meshes) {
            glm::mat4 transformacion = m.nodeTransform;
            for (const auto& p : piezas) {
                if (p.nodo != m.nodeName) continue;
                float t = ProgresoDe(p.grupo);
                glm::mat4 anim(1.0f);
                if (p.tipo == "rotacion")
                    anim = glm::translate(glm::mat4(1.0f), p.pivote)
                         * glm::rotate(glm::mat4(1.0f), glm::radians(p.valor * t), p.eje)
                         * glm::translate(glm::mat4(1.0f), -p.pivote);
                else if (p.tipo == "traslacion")
                    anim = glm::translate(glm::mat4(1.0f), p.eje * (p.valor * t));
                else if (p.tipo == "giro_continuo" && ProgresoDe(p.grupo) < 0.1f)   // El plato gira mientras la puerta está cerrada
                    anim = glm::translate(glm::mat4(1.0f), p.pivote)
                         * glm::rotate(glm::mat4(1.0f), tiempo * glm::radians(p.valor), p.eje)
                         * glm::translate(glm::mat4(1.0f), -p.pivote);
                transformacion = anim * transformacion;
                break;
            }
            auto pk = pistas.find(m.nodeName);                // Keyframes (grifo)
            if (pk != pistas.end() && !clips.empty() && clips[0].activo)
                transformacion = MatrizKeyframes(pk->second, clips[0].t) * transformacion;
            auto pr = procedurales.find(m.nodeName);          // Viento / balanceo (procedural)
            if (pr != procedurales.end())
                transformacion = MatrizProcedural(pr->second) * transformacion;
            shader.setMat4("model", transformacion);
            m.Draw(shader);
        }
    }

    // Carga una imagen como textura (por ejemplo la pantalla de inicio); regresa 0 si no la encuentra
    GLuint CargarTextura(const std::string& ruta) { float a; return CargarImagen(ruta, a); }

    // Dibuja una textura en un rectángulo de pantalla (coordenadas -1 a 1) con el shader tarjeta.vs/.fs
    void DibujarRect(Shader& shader, GLuint textura, float x0, float y0, float x1, float y1) {
        if (textura == 0) return;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        shader.use();
        shader.setVec4("uRect", glm::vec4(x0, y0, x1, y1));
        shader.setInt("uTarjeta", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textura);
        glBindVertexArray(tarjetaVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

    // Dispara la secuencia de keyframes i (si ya corre, no hace nada)
    void ReproducirClip(size_t i) {
        if (i >= clips.size() || clips[i].activo) return;
        clips[i].activo = true; clips[i].t = 0.0f;
        std::cout << "[Keyframes] " << clips[i].nombre << ": INICIA" << std::endl;
    }

    // Muestra la tarjeta de la planta i; si ya estaba abierta, la cierra
    void ToggleTarjeta(size_t i) {
        if (i >= plantas.size()) return;
        tarjetaActiva = (tarjetaActiva == (int)i) ? -1 : (int)i;
        std::cout << "[Tarjeta] " << plantas[i].nombre << (tarjetaActiva >= 0 ? ": MOSTRADA" : ": CERRADA") << std::endl;
    }

    // Dibuja la tarjeta activa a la derecha de la pantalla (ocupa 90 % del alto). shader = tarjeta.vs/.fs
    void DibujarTarjeta(Shader& shader, float anchoPantalla, float altoPantalla) {
        if (tarjetaActiva < 0 || tarjetaActiva >= (int)plantas.size() || plantas[tarjetaActiva].textura == 0) return;
        const PlantaTarjeta& pl = plantas[tarjetaActiva];
        float altoNDC = 1.8f;                                         // 90 % del alto (la pantalla mide 2 en NDC)
        float anchoNDC = altoNDC * pl.aspecto * altoPantalla / anchoPantalla * 1.0f;
        float x1 = 0.97f, x0 = x1 - anchoNDC, y0 = -0.9f, y1 = 0.9f;
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        shader.use();
        shader.setVec4("uRect", glm::vec4(x0, y0, x1, y1));
        shader.setInt("uTarjeta", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pl.textura);
        glBindVertexArray(tarjetaVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

private:
    // Interpola la pista en el segundo t y arma la matriz (rotación o escala en el pivote)
    static glm::mat4 MatrizKeyframes(const PistaKeyframes& pk, float t) {
        const auto& k = pk.claves;
        float v = k.empty() ? 0.0f : k.front().second;
        for (size_t i = 0; i + 1 < k.size(); ++i) {
            if (t >= k[i].first && t <= k[i + 1].first) {
                float u = (t - k[i].first) / std::max(k[i + 1].first - k[i].first, 1e-5f);
                v = k[i].second + (k[i + 1].second - k[i].second) * u;     // Interpolación lineal
                break;
            }
            if (t > k[i + 1].first) v = k[i + 1].second;
        }
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pk.pivote), Ti = glm::translate(glm::mat4(1.0f), -pk.pivote);
        if (pk.tipo == "escala")                                           // El chorro se alarga a lo largo del eje vertical
            return T * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, std::max(v, 0.001f), 1.0f)) * Ti;
        return T * glm::rotate(glm::mat4(1.0f), glm::radians(v), pk.eje) * Ti;
    }

    // Movimiento por seno: plantas mecidas por el viento (inclinación leve) y sillas que se balancean
    glm::mat4 MatrizProcedural(const Procedural& p) const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), p.pivote), Ti = glm::translate(glm::mat4(1.0f), -p.pivote);
        if (p.tipo == "balanceo")
            return T * glm::rotate(glm::mat4(1.0f), glm::radians(25.0f * std::sin(tiempo * 1.1f + p.fase)), glm::vec3(0, 1, 0)) * Ti;
        float amp = glm::radians(1.0f + 2.0f * std::min(p.altura, 1.5f));  // Más alta = se mece más
        return T * glm::rotate(glm::mat4(1.0f), amp * std::sin(tiempo * 1.7f + p.fase), glm::vec3(1, 0, 0))
                 * glm::rotate(glm::mat4(1.0f), 0.6f * amp * std::sin(tiempo * 1.3f + 2.0f * p.fase), glm::vec3(0, 0, 1)) * Ti;
    }

    static GLuint CargarImagen(const std::string& ruta, float& aspecto) {
        int w, h, c;
        unsigned char* px = stbi_load(ruta.c_str(), &w, &h, &c, 4);       // Siempre RGBA
        if (!px) { std::cout << "[Muebles] No se pudo cargar " << ruta << std::endl; return 0; }
        GLuint t; glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        stbi_image_free(px);
        aspecto = (float)w / (float)h;
        return t;
    }

    float ProgresoDe(const std::string& grupo) const {
        for (const auto& g : grupos) if (g.nombre == grupo) return g.progreso;
        return 0.0f;
    }

    static std::vector<std::vector<std::string>> LeerCSV(const std::string& ruta) {
        std::vector<std::vector<std::string>> filas;
        std::ifstream f(ruta);
        std::string linea;
        bool primera = true;
        while (std::getline(f, linea)) {
            if (primera) { primera = false; continue; }     // Encabezado
            if (!linea.empty() && linea.back() == '\r') linea.pop_back();
            std::vector<std::string> campos; std::stringstream ss(linea); std::string c;
            while (std::getline(ss, c, ',')) campos.push_back(c);
            if (!campos.empty()) filas.push_back(campos);
        }
        return filas;
    }
};

#endif
