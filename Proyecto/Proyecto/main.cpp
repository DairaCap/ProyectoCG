/*
* Proyecto Final: Hogar Botánico 3D (Casa Interactiva)
* Laboratorio de Computación Gráfica e Interacción Humano-Computadora
* UNAM - Semestre 2027-1
* Profesor: Dr. Sergio Teodoro Vite
*/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "camera.h"
#include "model.h"
#include "cubemap.h"
#include "light.h"
#include "material.h"
#include "AvatarController.h"
#include "Muebles.h"

// Dimensiones de ventana
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Cámara con física
Camera camera(glm::vec3(0.0f, 1.4f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
GLFWwindow* window = nullptr;

// Recursos gráficos
Shader* mainShader = nullptr;
Shader* cubemapShader = nullptr;
Shader* crosshairShader = nullptr;
Shader* tarjetaShader = nullptr;        // Tarjetas informativas de plantas
Model* casaModel = nullptr;
Model* characterModel = nullptr;
Muebles muebles;                       // Muebles, plantas y objetos de Romero (carpeta muebles/)
CubeMap* mainCubeMap = nullptr;

AvatarController avatarController;

unsigned int switchVAO = 0, switchVBO = 0;
unsigned int crosshairVAO = 0, crosshairVBO = 0;

std::vector<Light> gLights;
Material defaultMaterial;

bool spotLightOn = false;
bool dayNightCyclePaused = false;
float dayTimeProgression = 1.2f;

struct LightSwitch {
    std::string roomName;
    glm::vec3 position;
    glm::vec3 normal;
    size_t lightIndex;
    bool isOn;
    glm::vec4 basePower;
    AABB interactionBox;

    LightSwitch(std::string name, glm::vec3 pos, glm::vec3 norm, size_t lIndex, glm::vec4 power)
        : roomName(name), position(pos), normal(glm::normalize(norm)), lightIndex(lIndex),
        isOn(true), basePower(power)
    {
        glm::vec3 halfSize(0.22f, 0.28f, 0.22f);
        interactionBox = AABB(pos - halfSize, pos + halfSize);
    }
};
std::vector<LightSwitch> gSwitches;

enum InteractiveType { NONE_TYPE = 0, TYPE_DOOR, TYPE_WINDOW, TYPE_SWITCH, TYPE_FURNITURE, TYPE_PLANT };

struct RayHit {
    InteractiveType type = NONE_TYPE;
    size_t index = 0;
    float distance = 1000.0f;
    std::string prompt = "";
};
RayHit currentHit;

bool keyEPressed = false;
bool keyRPressed = false;
bool keyPPressed = false;
bool keyFPressed = false;
bool keyGPressed = false;
bool keyTPressed = false;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void InteractWithHovered();

void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
    std::ostringstream ss;
    ss << "allLights[" << lightIndex << "]." << propertyName;
    shader->setInt(ss.str().c_str(), value);
}
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
    std::ostringstream ss;
    ss << "allLights[" << lightIndex << "]." << propertyName;
    shader->setFloat(ss.str().c_str(), value);
}
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
    std::ostringstream ss;
    ss << "allLights[" << lightIndex << "]." << propertyName;
    shader->setVec4(ss.str().c_str(), value);
}
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
    std::ostringstream ss;
    ss << "allLights[" << lightIndex << "]." << propertyName;
    shader->setVec3(ss.str().c_str(), value);
}

bool RayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const AABB& box, float maxDist, float& outDist)
{
    float tMin = 0.0f;
    float tMax = maxDist;
    for (int i = 0; i < 3; ++i) {
        float invD = 1.0f / (std::abs(rayDir[i]) > 1e-6f ? rayDir[i] : (rayDir[i] >= 0.0f ? 1e-6f : -1e-6f));
        float t0 = (box.minPoint[i] - rayOrigin[i]) * invD;
        float t1 = (box.maxPoint[i] - rayOrigin[i]) * invD;
        if (invD < 0.0f) std::swap(t0, t1);
        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);
        if (tMax < tMin) return false;
    }
    outDist = tMin;
    return (tMin < maxDist);
}

RayHit PerformRaycast(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float maxDist = 3.8f)
{
    RayHit closestHit;
    closestHit.distance = maxDist;

    for (size_t i = 0; i < gSwitches.size(); ++i) {
        float d = 0.0f;
        if (RayIntersectsAABB(rayOrigin, rayDir, gSwitches[i].interactionBox, closestHit.distance, d)) {
            closestHit.type = TYPE_SWITCH;
            closestHit.index = i;
            closestHit.distance = d;
            closestHit.prompt = "[Click / E] Interruptor " + gSwitches[i].roomName + ": " + (gSwitches[i].isOn ? "APAGAR" : "ENCENDER");
        }
    }

    if (casaModel) {
        for (size_t i = 0; i < casaModel->doors.size(); ++i) {
            float d = 0.0f;
            AABB doorBox = casaModel->doors[i].GetInteractionBox();
            if (RayIntersectsAABB(rayOrigin, rayDir, doorBox, closestHit.distance, d)) {
                closestHit.type = TYPE_DOOR;
                closestHit.index = i;
                closestHit.distance = d;
                closestHit.prompt = "[Click / E] " + casaModel->doors[i].name + ": " + (casaModel->doors[i].isOpen ? "CERRAR" : "ABRIR");
            }
        }

        for (size_t i = 0; i < casaModel->windows.size(); ++i) {
            float d = 0.0f;
            AABB winBox = casaModel->windows[i].GetInteractionBox();
            if (RayIntersectsAABB(rayOrigin, rayDir, winBox, closestHit.distance, d)) {
                closestHit.type = TYPE_WINDOW;
                closestHit.index = i;
                closestHit.distance = d;
                closestHit.prompt = "[Click / R] " + casaModel->windows[i].name + ": " + (casaModel->windows[i].isOpen ? "CERRAR" : "ABRIR");
            }
        }
    }
    // Muebles con animación (clóset, cajones, laptop, estufa, refri...)
    for (size_t i = 0; i < muebles.grupos.size(); ++i) {
        float d = 0.0f;
        AABB caja(muebles.grupos[i].caja.minPoint - glm::vec3(0.05f), muebles.grupos[i].caja.maxPoint + glm::vec3(0.05f));
        if (RayIntersectsAABB(rayOrigin, rayDir, caja, closestHit.distance, d)) {
            closestHit.type = TYPE_FURNITURE;
            closestHit.index = i;
            closestHit.distance = d;
            closestHit.prompt = "[Click / E] " + muebles.Nombre(i) + ": " + (muebles.grupos[i].abierto ? "CERRAR" : "ABRIR");
        }
    }
    // Plantas con tarjeta informativa
    for (size_t i = 0; i < muebles.plantas.size(); ++i) {
        float d = 0.0f;
        AABB caja(muebles.plantas[i].caja.minPoint - glm::vec3(0.10f), muebles.plantas[i].caja.maxPoint + glm::vec3(0.10f));
        if (RayIntersectsAABB(rayOrigin, rayDir, caja, closestHit.distance, d)) {
            closestHit.type = TYPE_PLANT;
            closestHit.index = i;
            closestHit.distance = d;
            std::string n = muebles.plantas[i].nombre.substr(7);   // Quita "Planta_"
            closestHit.prompt = "[Click / E] " + n + ": " + (muebles.tarjetaActiva == (int)i ? "CERRAR TARJETA" : "VER TARJETA");
        }
    }
    return closestHit;
}

void InteractWithHovered()
{
    if (currentHit.type == TYPE_PLANT) {
        muebles.ToggleTarjeta(currentHit.index);
        return;
    }
    if (muebles.tarjetaActiva >= 0) {           // Click en otra cosa: se cierra la tarjeta abierta
        muebles.tarjetaActiva = -1;
        if (currentHit.type == NONE_TYPE) return;
    }
    if (currentHit.type == TYPE_SWITCH) {
        size_t idx = currentHit.index;
        if (idx < gSwitches.size()) {
            gSwitches[idx].isOn = !gSwitches[idx].isOn;
            size_t lIdx = gSwitches[idx].lightIndex;
            if (lIdx < gLights.size()) {
                gLights[lIdx].Power = gSwitches[idx].isOn ? gSwitches[idx].basePower : glm::vec4(0.0f);
            }
            std::cout << "[Interruptor] " << gSwitches[idx].roomName << ": "
                << (gSwitches[idx].isOn ? "Luz ENCENDIDA" : "Luz APAGADA") << std::endl;
        }
    }
    else if (currentHit.type == TYPE_DOOR) {
        if (casaModel) casaModel->ToggleDoor(currentHit.index);
    }
    else if (currentHit.type == TYPE_FURNITURE) {
        muebles.Toggle(currentHit.index);
    }
    else if (currentHit.type == TYPE_WINDOW) {
        if (casaModel) casaModel->ToggleWindow(currentHit.index);
    }
}

glm::vec3 GetWallNormalForPosition(glm::vec3 pos) {
    if (pos.x < -4.0f) return glm::vec3(1, 0, 0); // Pared Oeste
    if (pos.x > 4.0f) return glm::vec3(-1, 0, 0); // Pared Este
    if (pos.z > 3.0f) return glm::vec3(0, 0, -1); // Pared Norte
    if (pos.z < -3.0f) return glm::vec3(0, 0, 1); // Pared Sur
    return glm::vec3(0, 0, 1);
}

bool Start()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hogar Botanico 3D", NULL, NULL);
    if (window == NULL) {
        std::cout << "ERROR::GLFW: Fallo al crear la ventana" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mainShader = new Shader("shaders/11_PhongShaderMultLights.vs", "shaders/11_PhongShaderMultLights.fs");
    cubemapShader = new Shader("shaders/10_vertex_cubemap.vs", "shaders/10_fragment_cubemap.fs");
    crosshairShader = new Shader("shaders/crosshair.vs", "shaders/crosshair.fs");
    tarjetaShader = new Shader("shaders/tarjeta.vs", "shaders/tarjeta.fs");

    float crosshairVertices[] = { -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f };
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    float switchBoxVertices[] = {
        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,  0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,  0.5f,0.5f,-0.5f, 0,0,-1, 1,1,  0.5f,0.5f,-0.5f, 0,0,-1, 1,1, -0.5f,0.5f,-0.5f, 0,0,-1, 0,1, -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,
        -0.5f,-0.5f, 0.5f, 0,0, 1, 0,0,  0.5f,-0.5f, 0.5f, 0,0, 1, 1,0,  0.5f,0.5f, 0.5f, 0,0, 1, 1,1,  0.5f,0.5f, 0.5f, 0,0, 1, 1,1, -0.5f,0.5f, 0.5f, 0,0, 1, 0,1, -0.5f,-0.5f, 0.5f, 0,0, 1, 0,0,
        -0.5f, 0.5f, 0.5f,-1,0, 0, 1,0, -0.5f, 0.5f,-0.5f,-1,0, 0, 1,1, -0.5f,-0.5f,-0.5f,-1,0, 0, 0,1, -0.5f,-0.5f,-0.5f,-1,0, 0, 0,1, -0.5f,-0.5f, 0.5f,-1,0, 0, 0,0, -0.5f, 0.5f, 0.5f,-1,0, 0, 1,0,
         0.5f, 0.5f, 0.5f, 1,0, 0, 1,0,  0.5f, 0.5f,-0.5f, 1,0, 0, 1,1,  0.5f,-0.5f,-0.5f, 1,0, 0, 0,1,  0.5f,-0.5f,-0.5f, 1,0, 0, 0,1,  0.5f,-0.5f, 0.5f, 1,0, 0, 0,0,  0.5f, 0.5f, 0.5f, 1,0, 0, 1,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,1,  0.5f,-0.5f,-0.5f, 0,-1,0, 1,1,  0.5f,-0.5f, 0.5f, 0,-1,0, 1,0,  0.5f,-0.5f, 0.5f, 0,-1,0, 1,0, -0.5f,-0.5f, 0.5f, 0,-1,0, 0,0, -0.5f,-0.5f,-0.5f, 0,-1,0, 0,1,
        -0.5f, 0.5f,-0.5f, 0, 1,0, 0,1,  0.5f, 0.5f,-0.5f, 0, 1,0, 1,1,  0.5f, 0.5f, 0.5f, 0, 1,0, 1,0,  0.5f, 0.5f, 0.5f, 0, 1,0, 1,0, -0.5f, 0.5f, 0.5f, 0, 1,0, 0,0, -0.5f, 0.5f,-0.5f, 0, 1,0, 0,1
    };
    glGenVertexArrays(1, &switchVAO);
    glGenBuffers(1, &switchVBO);
    glBindVertexArray(switchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, switchVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(switchBoxVertices), switchBoxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    std::cout << "[Iniciando carga de escena arquitectonica y colisionadores...]" << std::endl;

    // CARGAR ARCHIVO 3 COMO LA CASA PRINCIPAL (Contiene las paredes, el techo, y los contactos)
    std::string houseModelPath = "Casa_coliciones3.fbx";
    FILE* fCheck = fopen(houseModelPath.c_str(), "rb");
    if (fCheck) { fclose(fCheck); }
    else { houseModelPath = "../Casa_coliciones3.fbx"; }

    casaModel = new Model(houseModelPath, true);

    // Muebles, plantas y objetos (carpeta muebles/ junto al ejecutable o un nivel arriba)
    if (!muebles.Cargar("muebles")) muebles.Cargar("../muebles");

    std::string charPath = "personaje_estudiante.fbx";
    fCheck = fopen(charPath.c_str(), "rb");
    if (fCheck) { fclose(fCheck); }
    else { charPath = "../personaje_estudiante.fbx"; }
    characterModel = new Model(charPath, false);
    avatarController.Init(charPath);

    std::vector<std::string> faces{
        "textures/cubemap/01/posx.png", "textures/cubemap/01/negx.png",
        "textures/cubemap/01/posy.png", "textures/cubemap/01/negy.png",
        "textures/cubemap/01/posz.png", "textures/cubemap/01/negz.png"
    };
    mainCubeMap = new CubeMap();
    mainCubeMap->loadCubemap(faces);

    // -------------------------------------------------------------
    // LUCES
    // -------------------------------------------------------------
    gLights.clear();

    // 0) Sol (Exterior)
    Light sunLight;
    sunLight.Position = glm::vec3(0.0f, 25.0f, 0.0f);
    sunLight.Direction = glm::vec3(0.0f, -1.0f, 0.0f);
    sunLight.Color = glm::vec4(1.0f, 0.98f, 0.92f, 1.0f);
    sunLight.Power = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
    sunLight.distance = 15.0f;
    gLights.push_back(sunLight);

    // 1) Luz Sala Principal
    Light roomLight;
    roomLight.Position = glm::vec3(-1.75f, 2.45f, 1.50f);
    roomLight.Color = glm::vec4(1.0f, 0.85f, 0.65f, 1.0f);
    roomLight.Power = glm::vec4(3.5f, 3.5f, 3.5f, 1.0f);
    roomLight.distance = 7.0f;
    gLights.push_back(roomLight);

    // 2) Luz Cocina
    Light kitchenLight;
    kitchenLight.Position = glm::vec3(-3.85f, 2.45f, -1.50f);
    kitchenLight.Color = glm::vec4(1.0f, 0.85f, 0.65f, 1.0f);
    kitchenLight.Power = glm::vec4(3.5f, 3.5f, 3.5f, 1.0f);
    kitchenLight.distance = 7.0f;
    gLights.push_back(kitchenLight);

    // 3) Luz Baño
    Light bathLight;
    bathLight.Position = glm::vec3(2.85f, 2.45f, -2.35f);
    bathLight.Color = glm::vec4(1.0f, 0.85f, 0.65f, 1.0f);
    bathLight.Power = glm::vec4(3.5f, 3.5f, 3.5f, 1.0f);
    bathLight.distance = 5.0f;
    gLights.push_back(bathLight);

    // 4) Luz Recámara Este
    Light bedLight;
    bedLight.Position = glm::vec3(2.80f, 2.45f, 1.30f);
    bedLight.Color = glm::vec4(1.0f, 0.85f, 0.65f, 1.0f);
    bedLight.Power = glm::vec4(3.5f, 3.5f, 3.5f, 1.0f);
    bedLight.distance = 7.0f;
    gLights.push_back(bedLight);

    // 5) Luz Terraza Jardín
    Light gardenLight;
    gardenLight.Position = glm::vec3(-3.95f, 2.45f, -3.00f);
    gardenLight.Color = glm::vec4(1.0f, 0.90f, 0.80f, 1.0f);
    gardenLight.Power = glm::vec4(3.5f, 3.5f, 3.5f, 1.0f);
    gardenLight.distance = 7.0f;
    gLights.push_back(gardenLight);

    gSwitches.clear();

    // -------------------------------------------------------------
    // GENERAR INTERRUPTORES DESDE BLENDER (Material "contacto")
    // -------------------------------------------------------------
    if (casaModel && !casaModel->contactPoints.empty()) {
        std::cout << "[Configurando " << casaModel->contactPoints.size() << " interruptores desde Blender]" << std::endl;

        for (size_t i = 0; i < casaModel->contactPoints.size(); ++i) {
            const auto& cp = casaModel->contactPoints[i];

            std::string roomName = "Contacto " + std::to_string(i + 1);
            size_t lightIdx = 1;

            if (cp.position.x < -1.0f && cp.position.z > 0.0f) {
                roomName = "Sala";
                lightIdx = 1;
            }
            else if (cp.position.x < -1.0f && cp.position.z <= 0.0f) {
                roomName = "Cocina";
                lightIdx = 2;
            }
            else if (cp.position.x > 1.0f && cp.position.z < 0.0f) {
                roomName = "Bano";
                lightIdx = 3;
            }
            else if (cp.position.x > 2.0f && cp.position.z >= 0.0f) {
                roomName = "Recamara";
                lightIdx = 4;
            }
            else {
                roomName = "Terraza";
                lightIdx = 5;
            }

            glm::vec4 pwr = (lightIdx < gLights.size()) ? gLights[lightIdx].Power : glm::vec4(3.5f);

            gSwitches.push_back(LightSwitch(roomName, cp.position, cp.normal, lightIdx, pwr));

            std::cout << "  [+] Vinculado: \"" << roomName << "\" a la Luz [" << lightIdx << "]" << std::endl;
        }
    }
    else {
        std::cout << "[!] No se detectaron cubos con material 'contacto'. Usando interruptores por defecto." << std::endl;
        gSwitches.push_back(LightSwitch("Sala", glm::vec3(-0.45f, 1.15f, 3.65f), glm::vec3(0, 0, -1), 1, gLights[1].Power));
        gSwitches.push_back(LightSwitch("Cocina", glm::vec3(-3.85f, 1.15f, 1.60f), glm::vec3(1, 0, 0), 2, gLights[2].Power));
        gSwitches.push_back(LightSwitch("Bano", glm::vec3(2.95f, 1.15f, 0.60f), glm::vec3(-1, 0, 0), 3, gLights[3].Power));
    }

    defaultMaterial.ambient = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
    defaultMaterial.diffuse = glm::vec4(0.85f, 0.85f, 0.85f, 1.0f);
    defaultMaterial.specular = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);

    std::cout << "🌿 HOGAR BOTANICO 3D - LISTO 🌿\n";
    return true;
}

bool Update()
{
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    if (deltaTime > 0.1f) deltaTime = 0.1f;

    processInput(window);

    if (casaModel) {
        casaModel->UpdateAnimations(deltaTime);
        std::vector<AABB> colliders = casaModel->GetActiveColliders();
        colliders.insert(colliders.end(), muebles.colisiones.begin(), muebles.colisiones.end());   // Muebles que bloquean el paso
        camera.UpdatePhysics(deltaTime, colliders);
        muebles.Update(deltaTime);
    }

    if (characterModel) {
        avatarController.Update(deltaTime, camera.isMoving, characterModel);
    }

    currentHit = PerformRaycast(camera.GetEyePosition(), camera.Front, 3.8f);

    if (!dayNightCyclePaused) dayTimeProgression += deltaTime * 0.05f;
    float sunY = std::sin(dayTimeProgression) * 35.0f;
    gLights[0].Position = glm::vec3(std::cos(dayTimeProgression) * 35.0f, std::max(sunY, -8.0f), 0.0f);
    gLights[0].Direction = glm::normalize(-gLights[0].Position);

    glm::vec4 skyClearColor = (sunY > 8.0f) ? glm::vec4(0.20f, 0.35f, 0.55f, 1.0f) : glm::vec4(0.04f, 0.05f, 0.10f, 1.0f);

    glClearColor(skyClearColor.r, skyClearColor.g, skyClearColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.08f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    if (mainCubeMap && cubemapShader) mainCubeMap->drawCubeMap(*cubemapShader, projection, view);

    if (mainShader)
    {
        mainShader->use();
        mainShader->setMat4("projection", projection);
        mainShader->setMat4("view", view);
        mainShader->setVec3("eye", camera.GetEyePosition());

        mainShader->setInt("numLights", (int)gLights.size());
        for (size_t i = 0; i < gLights.size(); ++i) {
            SetLightUniformVec3(mainShader, "Position", i, gLights[i].Position);
            SetLightUniformVec3(mainShader, "Direction", i, gLights[i].Direction);
            SetLightUniformVec4(mainShader, "Color", i, gLights[i].Color);
            SetLightUniformVec4(mainShader, "Power", i, gLights[i].Power);
            SetLightUniformFloat(mainShader, "distance", i, gLights[i].distance);
        }

        // Cubemap para reflejos (metales, espejo) y Fresnel (vidrio, agua). Antes nunca se le pasaba al shader,
        // por eso hasSkybox quedaba en false y no habia reflejo. Va en la unidad 7 para no chocar con las texturas del mesh.
        if (mainCubeMap) {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, mainCubeMap->textureID);
            glActiveTexture(GL_TEXTURE0);
            mainShader->setInt("skybox", 7);
            mainShader->setBool("hasSkybox", true);
        }

        mainShader->setBool("spotLightOn", spotLightOn);
        mainShader->setVec3("spotLightPos", camera.GetEyePosition());
        mainShader->setVec3("spotLightDir", camera.Front);
        mainShader->setVec4("spotLightColor", glm::vec4(1.0f, 0.98f, 0.88f, 1.0f));

        if (casaModel) casaModel->Draw(*mainShader);
        muebles.Draw(*mainShader);                 // Muebles, plantas y objetos con sus animaciones

        for (size_t s = 0; s < gSwitches.size(); ++s) {
            const auto& sw = gSwitches[s];
            glm::mat4 rot(1.0f);
            if (sw.normal.x > 0.5f)       rot = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
            else if (sw.normal.x < -0.5f) rot = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
            else if (sw.normal.z > 0.5f)  rot = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));
            else if (sw.normal.z < -0.5f) rot = glm::mat4(1.0f);

            glm::vec3 platePos = sw.position + sw.normal * 0.015f;
            glm::mat4 plateMat = glm::translate(glm::mat4(1.0f), platePos) * rot * glm::scale(glm::mat4(1.0f), glm::vec3(0.10f, 0.15f, 0.015f));
            mainShader->setMat4("model", plateMat);
            mainShader->setVec4("MaterialDiffuseColor", glm::vec4(0.92f));
            glBindVertexArray(switchVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            float tiltAngle = sw.isOn ? -12.0f : 12.0f;
            glm::mat4 rockerMat = glm::translate(glm::mat4(1.0f), platePos + sw.normal * 0.012f) * rot * glm::rotate(glm::mat4(1.0f), glm::radians(tiltAngle), glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.045f, 0.075f, 0.020f));
            mainShader->setMat4("model", rockerMat);
            mainShader->setVec4("MaterialDiffuseColor", sw.isOn ? glm::vec4(1.0f, 0.95f, 0.60f, 1.0f) : glm::vec4(0.25f));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        if (camera.isThirdPerson && characterModel) {
            float feetY = camera.BasePosition.y - camera.eyeHeight;
            avatarController.Draw(*mainShader, characterModel, glm::vec3(camera.BasePosition.x, feetY, camera.BasePosition.z), camera.Yaw, true);
        }
    }

    if (crosshairShader && crosshairVAO != 0 && currentHit.type != NONE_TYPE)
    {
        glDisable(GL_DEPTH_TEST);
        crosshairShader->use();
        crosshairShader->setVec2("uScale", glm::vec2(13.5f / SCR_WIDTH, 13.5f / SCR_HEIGHT));
        crosshairShader->setVec4("uColor", glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));
        glBindVertexArray(crosshairVAO);
        glDrawArrays(GL_LINES, 0, 4);
        glEnable(GL_DEPTH_TEST);
        glfwSetWindowTitle(window, currentHit.prompt.c_str());
    }
    else {
        glfwSetWindowTitle(window, "Hogar Botanico 3D");
    }

    if (tarjetaShader) muebles.DibujarTarjeta(*tarjetaShader, (float)SCR_WIDTH, (float)SCR_HEIGHT);   // Tarjeta de la planta (a la derecha)

    glfwSwapBuffers(window);
    glfwPollEvents();
    return true;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    camera.isMoving = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.Jump();

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!keyPPressed) { camera.TogglePerspective(); keyPPressed = true; }
    }
    else keyPPressed = false;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!keyFPressed) { spotLightOn = !spotLightOn; keyFPressed = true; }
    }
    else keyFPressed = false;

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!keyTPressed) { dayNightCyclePaused = !dayNightCyclePaused; keyTPressed = true; }
    }
    else keyTPressed = false;

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (!keyGPressed) { avatarController.TriggerWatering(); keyGPressed = true; }
    }
    else keyGPressed = false;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        if (!keyEPressed) { InteractWithHovered(); keyEPressed = true; }
    }
    else keyEPressed = false;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!keyRPressed) { if (casaModel) casaModel->ToggleAllWindows(); keyRPressed = true; }
    }
    else keyRPressed = false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) InteractWithHovered();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    camera.ProcessMouseMovement((float)xpos - lastX, lastY - (float)ypos);
    lastX = (float)xpos; lastY = (float)ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}

int main(int argc, char* argv[]) {
    if (!Start()) return -1;
    while (!glfwWindowShouldClose(window)) if (!Update()) break;
    return 0;
}