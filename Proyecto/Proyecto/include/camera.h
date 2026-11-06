#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cmath>
#include <algorithm>
#include "AABB.h"

// Direcciones posibles de movimiento de la cámara
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Valores por defecto
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 3.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 68.0f; // Campo de visión amplio (arquitectónico)

class Camera
{
public:
    // Atributos de cámara
    glm::vec3 Position;      // Posición final de la vista (incluye Head Bobbing en 1ra persona)
    glm::vec3 BasePosition;  // Posición física del jugador (usada para colisiones y física)
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Ángulos de Euler
    float Yaw;
    float Pitch;

    // Opciones de cámara
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Física y dimensiones humanas ajustadas (Módulo 2)
    float velocidadY;
    float gravedad;
    bool  enSuelo;
    float fuerzaSalto;
    float playerRadius;     // Radio de colisión cilíndrica horizontal (~0.22m para paso fácil por puertas)
    float playerHeight;     // Altura total del estudiante (~1.35m)
    float eyeHeight;        // Altura de los ojos desde los pies (~1.25m)

    // Animación Procedural: Head Bobbing (Módulo 3)
    float velocidadBobbing; // Frecuencia del paso
    float amplitudBobbing;  // Amplitud vertical máxima
    float bobbingTimer;
    bool isMoving;

    // Sistema de Perspectiva: 1ra Persona vs 3ra Persona
    bool  isThirdPerson;
    float thirdPersonDistance;
    float thirdPersonHeight;

    // Constructor con vectores
    Camera(glm::vec3 position = glm::vec3(0.0f, 1.4f, 2.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY),
        Zoom(ZOOM),
        velocidadY(0.0f),
        gravedad(-9.81f),
        enSuelo(true),
        fuerzaSalto(4.5f),
        playerRadius(0.22f),
        playerHeight(1.35f),
        eyeHeight(1.25f),
        velocidadBobbing(10.0f),
        amplitudBobbing(0.035f),
        bobbingTimer(0.0f),
        isMoving(false),
        isThirdPerson(false),
        thirdPersonDistance(2.4f),
        thirdPersonHeight(0.55f)
    {
        BasePosition = position;
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Alternar entre 1ra persona y 3ra persona (Tecla P)
    void TogglePerspective()
    {
        isThirdPerson = !isThirdPerson;
        std::cout << "[Perspectiva] Modo cambiado a: "
            << (isThirdPerson ? "TERCERA PERSONA (Estilo Roblox)" : "PRIMERA PERSONA") << std::endl;
    }

    // Retorna la posición real de la cámara en el mundo (para cálculo de shaders y reflejos)
    glm::vec3 GetEyePosition() const
    {
        if (!isThirdPerson)
        {
            return Position;
        }
        else
        {
            glm::vec3 charCenter = BasePosition + glm::vec3(0.0f, eyeHeight, 0.0f);
            return charCenter - Front * thirdPersonDistance + glm::vec3(0.0f, thirdPersonHeight, 0.0f);
        }
    }

    // Retorna la matriz de vista (1ra persona o 3ra persona orbitando)
    glm::mat4 GetViewMatrix() const
    {
        if (!isThirdPerson)
        {
            return glm::lookAt(Position, Position + Front, Up);
        }
        else
        {
            glm::vec3 charCenter = BasePosition + glm::vec3(0.0f, eyeHeight, 0.0f);
            glm::vec3 camPos = charCenter - Front * thirdPersonDistance + glm::vec3(0.0f, thirdPersonHeight, 0.0f);
            return glm::lookAt(camPos, charCenter, Up);
        }
    }

    // Procesa el movimiento horizontal (W, A, S, D) sobre el plano XZ (sin volar)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        glm::vec3 frontHorizontal = glm::normalize(glm::vec3(Front.x, 0.0f, Front.z));
        glm::vec3 rightHorizontal = glm::normalize(glm::vec3(Right.x, 0.0f, Right.z));

        if (direction == FORWARD)
            BasePosition += frontHorizontal * velocity;
        if (direction == BACKWARD)
            BasePosition -= frontHorizontal * velocity;
        if (direction == LEFT)
            BasePosition -= rightHorizontal * velocity;
        if (direction == RIGHT)
            BasePosition += rightHorizontal * velocity;

        isMoving = true;
    }

    // Ejecuta el salto si el jugador está en el suelo
    void Jump()
    {
        if (enSuelo)
        {
            velocidadY = fuerzaSalto;
            enSuelo = false;
        }
    }

    // Corrección 3: Física de penetración 3D (Círculo-AABB contra muros, soporte de piso y techo)
    void UpdatePhysics(float deltaTime, const std::vector<AABB>& cajasColision)
    {
        // ---------------------------------------------------------------------
        // 1. RESOLUCIÓN DE COLISIÓN HORIZONTAL (Círculo 2D vs AABB de Muros)
        // ---------------------------------------------------------------------
        const float r = playerRadius;
        float feetY = BasePosition.y - eyeHeight;
        float headY = feetY + playerHeight;

        for (int iter = 0; iter < 3; ++iter)
        {
            glm::vec2 playerPos(BasePosition.x, BasePosition.z);

            for (const auto& caja : cajasColision)
            {
                if (caja.maxPoint.y <= 0.05f || caja.minPoint.y >= 2.80f)
                    continue;

                if (headY <= caja.minPoint.y + 0.1f || feetY >= caja.maxPoint.y - 0.1f)
                    continue;

                float closestX = std::clamp(playerPos.x, caja.minPoint.x, caja.maxPoint.x);
                float closestZ = std::clamp(playerPos.y, caja.minPoint.z, caja.maxPoint.z);

                float dx = playerPos.x - closestX;
                float dz = playerPos.y - closestZ;
                float distSq = dx * dx + dz * dz;

                if (distSq > 0.000001f && distSq < (r * r))
                {
                    float dist = std::sqrt(distSq);
                    float pen = r - dist;
                    float nx = dx / dist;
                    float nz = dz / dist;

                    BasePosition.x += nx * pen;
                    BasePosition.z += nz * pen;
                    playerPos.x = BasePosition.x;
                    playerPos.y = BasePosition.z;
                }
                else if (distSq <= 0.000001f)
                {
                    float dLeft = playerPos.x - caja.minPoint.x;
                    float dRight = caja.maxPoint.x - playerPos.x;
                    float dFront = playerPos.y - caja.minPoint.z;
                    float dBack = caja.maxPoint.z - playerPos.y;

                    float minDist = std::min({ dLeft, dRight, dFront, dBack });

                    if (minDist == dLeft)        BasePosition.x = caja.minPoint.x - r;
                    else if (minDist == dRight)  BasePosition.x = caja.maxPoint.x + r;
                    else if (minDist == dFront)  BasePosition.z = caja.minPoint.z - r;
                    else                         BasePosition.z = caja.maxPoint.z + r;

                    playerPos.x = BasePosition.x;
                    playerPos.y = BasePosition.z;
                }
            }
        }

        // ---------------------------------------------------------------------
        // 2. FÍSICA VERTICAL (Gravedad, Salto y Soporte de Piso)
        // ---------------------------------------------------------------------
        velocidadY += gravedad * deltaTime;
        BasePosition.y += velocidadY * deltaTime;
        feetY = BasePosition.y - eyeHeight;

        enSuelo = false;

        // Comprobar colisión vertical contra el suelo (cajas debajo de los pies)
        for (const auto& caja : cajasColision)
        {
            if (caja.maxPoint.y <= 0.20f && caja.maxPoint.y >= -1.0f)
            {
                if (BasePosition.x >= (caja.minPoint.x - 0.2f) && BasePosition.x <= (caja.maxPoint.x + 0.2f) &&
                    BasePosition.z >= (caja.minPoint.z - 0.2f) && BasePosition.z <= (caja.maxPoint.z + 0.2f))
                {
                    // CORRECCIÓN DE SALTO: Solo encajar en el piso si el personaje está CAYENDO (velocidadY <= 0)
                    if (velocidadY <= 0.0f && feetY <= caja.maxPoint.y + 0.05f && (feetY + 0.40f) >= caja.maxPoint.y)
                    {
                        BasePosition.y = caja.maxPoint.y + eyeHeight;
                        velocidadY = 0.0f;
                        enSuelo = true;
                        break;
                    }
                }
            }
        }

        // Si salta y golpea el techo
        float headCheck = BasePosition.y - eyeHeight + playerHeight;
        for (const auto& caja : cajasColision)
        {
            if (caja.minPoint.y >= 2.50f)
            {
                if (BasePosition.x >= caja.minPoint.x && BasePosition.x <= caja.maxPoint.x &&
                    BasePosition.z >= caja.minPoint.z && BasePosition.z <= caja.maxPoint.z)
                {
                    if (headCheck >= caja.minPoint.y && velocidadY > 0.0f)
                    {
                        velocidadY = 0.0f;
                        BasePosition.y = caja.minPoint.y - playerHeight + eyeHeight - 0.02f;
                    }
                }
            }
        }

        // Respawn de seguridad si cae fuera del mapa
        if (BasePosition.y < -30.0f)
        {
            BasePosition = glm::vec3(0.0f, 1.8f, 2.0f);
            velocidadY = 0.0f;
            enSuelo = true;
        }

        // ---------------------------------------------------------------------
        // 3. ANIMACIÓN PROCEDURAL (Head Bobbing)
        // ---------------------------------------------------------------------
        float bobbingOffset = 0.0f;
        if (isMoving && enSuelo)
        {
            bobbingTimer += deltaTime * velocidadBobbing;
            bobbingOffset = std::sin(bobbingTimer) * amplitudBobbing;
        }
        else
        {
            bobbingTimer = 0.0f;
        }

        // Posición final de la cámara
        Position = BasePosition;
        Position.y += bobbingOffset;

        // Reiniciar bandera de movimiento
        isMoving = false;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif