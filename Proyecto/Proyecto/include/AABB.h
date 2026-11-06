#pragma once

#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>

// Estructura AABB (Axis-Aligned Bounding Box) alineada a los ejes
struct AABB {
    glm::vec3 minPoint;
    glm::vec3 maxPoint;

    AABB() 
        : minPoint(glm::vec3(0.0f)), maxPoint(glm::vec3(0.0f)) {}

    AABB(const glm::vec3& minP, const glm::vec3& maxP) 
        : minPoint(minP), maxPoint(maxP) {}

    // Verifica si hay solapamiento (intersección) con otra caja AABB
    bool Intersects(const AABB& other) const {
        return (minPoint.x <= other.maxPoint.x && maxPoint.x >= other.minPoint.x) &&
               (minPoint.y <= other.maxPoint.y && maxPoint.y >= other.minPoint.y) &&
               (minPoint.z <= other.maxPoint.z && maxPoint.z >= other.minPoint.z);
    }

    // Calcula el vector de penetración mínimo para empujar al objeto fuera de la caja
    // Retorna true si hay colisión, y almacena en 'resolution' el vector de desplazamiento
    bool ResolveCollision(const AABB& playerBox, glm::vec3& resolution, bool& isFloorCollision, bool& isCeilingCollision) const {
        if (!Intersects(playerBox)) {
            resolution = glm::vec3(0.0f);
            isFloorCollision = false;
            isCeilingCollision = false;
            return false;
        }

        // Distancias de penetración en cada cara
        float overlapX1 = otherMaxDist(playerBox.maxPoint.x, minPoint.x); // jugador entra por la izquierda de la caja
        float overlapX2 = otherMaxDist(maxPoint.x, playerBox.minPoint.x); // jugador entra por la derecha de la caja

        float overlapY1 = otherMaxDist(playerBox.maxPoint.y, minPoint.y); // jugador entra por abajo
        float overlapY2 = otherMaxDist(maxPoint.y, playerBox.minPoint.y); // jugador se apoya encima (piso)

        float overlapZ1 = otherMaxDist(playerBox.maxPoint.z, minPoint.z);
        float overlapZ2 = otherMaxDist(maxPoint.z, playerBox.minPoint.z);

        // Elegir la menor penetración por eje
        float pushX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;
        float pushY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;
        float pushZ = (overlapZ1 < overlapZ2) ? -overlapZ1 : overlapZ2;

        float absX = std::abs(pushX);
        float absY = std::abs(pushY);
        float absZ = std::abs(pushZ);

        resolution = glm::vec3(0.0f);
        isFloorCollision = false;
        isCeilingCollision = false;

        // Empujar en el eje de menor penetración
        if (absY <= absX && absY <= absZ) {
            resolution.y = pushY;
            if (pushY > 0.0f) {
                isFloorCollision = true; // Empujado hacia arriba -> Piso
            } else {
                isCeilingCollision = true; // Empujado hacia abajo -> Techo
            }
        } else if (absX <= absZ) {
            resolution.x = pushX;
        } else {
            resolution.z = pushZ;
        }

        return true;
    }

private:
    static float otherMaxDist(float a, float b) {
        return a - b;
    }
};
