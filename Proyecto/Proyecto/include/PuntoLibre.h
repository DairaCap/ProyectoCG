#pragma once
// ============================================================================
// PuntoLibre.h - Ayudas para saber si un lugar del piso esta libre de muebles y paredes.
// Las usan los checkpoints (para no quedar dentro de un mueble) y el minijuego de plantas
// (para repartir las plantas en el patio y para que la planta que se carga no atraviese paredes).
// ============================================================================
#include <vector>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>

#include "AABB.h"

// ¿El circulo de radio "radio" centrado en p (en el piso) esta libre de muebles y paredes?
// Se ignoran las cajas que son solo piso (muy bajas) o techo (muy altas).
inline bool PuntoLibre(const glm::vec3& p, float radio, const std::vector<AABB>& cajas)
{
    for (const AABB& c : cajas) {
        if (c.maxPoint.y < 0.3f || c.minPoint.y > 1.6f) continue;
        float cx = std::min(std::max(p.x, c.minPoint.x), c.maxPoint.x);     // Punto de la caja mas cercano a p
        float cz = std::min(std::max(p.z, c.minPoint.z), c.maxPoint.z);
        if (glm::length(glm::vec2(p.x - cx, p.z - cz)) < radio) return false;
    }
    return true;
}

// Busca desde "deseado" hacia afuera (en anillos de 0.25 m) el primer punto libre de radio "radio"
inline glm::vec3 BuscarPuntoLibre(const glm::vec3& deseado, const std::vector<AABB>& cajas, float radio = 1.0f)
{
    for (float r = 0.0f; r <= 4.0f; r += 0.25f) {
        int pasos = (r == 0.0f) ? 1 : 16;
        for (int k = 0; k < pasos; ++k) {
            float ang = glm::radians(360.0f * k / pasos);
            glm::vec3 cand = deseado + glm::vec3(std::cos(ang), 0.0f, std::sin(ang)) * r;
            if (PuntoLibre(cand, radio, cajas)) return cand;
        }
    }
    return deseado;
}
