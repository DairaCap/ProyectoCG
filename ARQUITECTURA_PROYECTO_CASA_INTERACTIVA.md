# Arquitectura del Proyecto: Casa Interactiva (Entorno de Videojuego)

## 1. Análisis de la Base de Código Existente

### 1.1 Estructura de Prácticas del Curso

El repositorio contiene 14+ prácticas progresivas de OpenGL:

| Práctica | Conceptos Clave | Archivos Relevantes |
|----------|-----------------|---------------------|
| **00-01** | Configuración OpenGL, triángulos básicos | `Shader.h`, `Vertex.h`, `Mesh.h` |
| **02** | Transformaciones Proyección/Viewport | `Camera.h`, `Piramide.h`, `Triangle3D.h` |
| **03** | Modelado Geométrico (primitivas) | `Cube.h`, `Sphere.h`, `Cylinder.h`, `Cone.h`, `Plane.h` |
| **04** | Modelado Jerárquico | `hand.png` (ejemplo articulado) |
| **05** | Carga de Modelos (Assimp) | `model.h`, `animatedmodel.h`, `modelstructs.h` |
| **06** | Texturizado | `stb_image.h`, texturas diffuse/specular/normal |
| **07-08** | Modelos de Iluminación (Phong, múltiples luces) | `light.h`, `material.h`, `cubemap.h` |
| **09-10** | Animación Esquelética | `animatedmodel.h`, bone weights, skinning |
| **11** | Partículas | `particles.h` |
| **12** | Espejos/Reflejos | Framebuffers, render-to-texture |
| **13** | **Picking (Selección por color)** | `15_picking.vs/fs`, color encoding por mesh ID |
| **14** | Fresnel | Efectos de refracción/reflexión |

### 1.2 Arquitectura Común (Patrón Base)

Todas las prácticas desde la **05** comparten esta arquitectura:

```
┌─────────────────────────────────────────────────────────────┐
│                      MAIN.CPP                               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────────────┐  │
│  │  Start() │ │ Update() │ │ Process  │ │  Callbacks     │  │
│  │  (Init)  │ │ (Render) │ │ Input()  │ │  (GLFW)        │  │
│  └──────────┘ └──────────┘ └──────────┘ └────────────────┘  │
└─────────────────────────────────────────────────────────────┘
          │           │           │              │
          ▼           ▼           ▼              ▼
┌──────────────────┐ ┌────────┐ ┌────────┐ ┌────────────┐
│   SHADER         │ │ CAMERA │ │ MODEL  │ │  LIGHT/    │
│   (shader_m.h)   │ │        │ │        │ │  MATERIAL  │
└──────────────────┘ └────────┘ └────────┘ └────────────┘
          │                                       │
          ▼                                       ▼
┌──────────────────┐                    ┌─────────────────┐
│   MESH           │                    │   CUBEMAP       │
│   (VAO/VBO/EBO)  │                    │   (Skybox)      │
└──────────────────┘                    └─────────────────┘
```

### 1.3 Recursos Disponibles (Assets)

**Modelos FBX en `08_IlluminationModelsII/Proyecto/bin/models/IllumModels/`:**
- `House02.fbx`, `House03.fbx` - Casas completas
- `Door.fbx` - Puerta independiente (animable)
- `KAYA.fbx` - Personaje animado (rigged)
- `Sphere.fbx`, `Table02.fbx`, `grid.fbx`, `plane.fbx`, `lightDummy.fbx`, `moon.fbx`
- `character.fbx`, `cube.fbx`, `floor.fbx`, `mycube.fbx`

**Texturas:** Madera, hierba, techo, normales, specular maps

**Shaders en `bin/shaders/`:**
- Phong multi-luces (`11_PhongShaderMultLights`)
- Skinning/Animación (`09_10_vertex_skinning`, `10_fragment_skinning-IT`)
- **Picking** (`15_picking.vs/fs`) - **Clave para interacción**
- Cubemap/Skybox (`10_vertex_cubemap`, `10_fragment_cubemap`)
- Procedural/Waves (`12_ProceduralAnimation`, `13_wavesAnimation`)
- Fresnel (`11_Fresnel`), Mirror (`14_mirror`)

---

## 2. Objetivos del Proyecto: Casa Interactiva

### 2.1 Visión General
Crear un **entorno de videojuego en primera persona** donde el jugador pueda:
- **Recorrer** la casa libremente (navegación FPS)
- **Interactuar con interruptores** (encender/apagar luces)
- **Abrir/cerrar puertas y ventanas** (animaciones)
- **Hacer click sobre objetos** (picking/selección)
- **Agarrar, mover y soltar objetos** (física básica / kinematics)

### 2.2 Alcance Mínimo Viable (MVP)
1. **Navegación FPS** - Cámara primera persona con colisiones básicas
2. **Sistema de Picking** - Selección de objetos por click (ya implementado en práctica 13)
3. **Interacción con Puertas** - Click → animación abrir/cerrar
4. **Interruptores de Luz** - Click → toggle luz puntual + feedback visual
5. **Ventanas** - Click → animación abrir/cerrar (rotación/traslación)
6. **Agarrar objetos** - Click sostenido → attach a cámara → soltar → drop

### 2.3 Características Avanzadas (Stretch Goals)
- Inventario simple (tecla `TAB` / `I`)
- Física básica (gravedad al soltar objetos)
- Sonidos (irrKlang ya integrado)
- Partículas (polvo al abrir puertas, chispas en interruptores)
- Reflejos en espejos/ventanas (práctica 12)
- Estado persistente (guardar/cargar posición de objetos)

---

## 3. Arquitectura Propuesta

### 3.1 Diagrama de Alto Nivel

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           INTERACTIVE HOUSE ENGINE                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────┐  │
│  │   INPUT      │    │   GAME       │    │   RENDER     │    │  AUDIO   │  │
│  │   MANAGER    │───▶│   LOGIC      │───▶│   SYSTEM     │    │  SYSTEM  │  │
│  └──────────────┘    └──────────────┘    └──────────────┘    └──────────┘  │
│         │                    │                    │              │          │
│         ▼                    ▼                    ▼              ▼          │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────┐  │
│  │ Keyboard/    │    │ Entity       │    │ Shader       │    │ irrKlang │  │
│  │ Mouse/       │    │ Component    │    │ Pipeline     │    │          │  │
│  │ Callbacks    │    │ System (ECS) │    │ (Multi-pass) │    │          │  │
│  └──────────────┘    └──────────────┘    └──────────────┘    └──────────┘  │
│                             │                    │                          │
│                    ┌────────┼────────┐           │                          │
│                    ▼        ▼        ▼           ▼                          │
│              ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐                    │
│              │Transform│ │Render  │ │Physics │ │Interact│                    │
│              │Component│ │Component│ │Component│ │Component│                   │
│              └────────┘ └────────┘ └────────┘ └────────┘                    │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3.2 Sistema de Entidades (ECS Ligero)

Dado que el código base usa herencia y no ECS puro, propondré un **híbrido pragmático**:

```cpp
// Base común para todos los objetos del juego
class GameObject {
public:
    Transform transform;           // Posición, rotación, escala
    std::string name;              // Identificador único
    std::string tag;               // "Door", "LightSwitch", "Window", "Grabbable", etc.
    bool active = true;
    unsigned int pickingID;        // Para color-picking (práctica 13)
    
    virtual void Update(float deltaTime) {}
    virtual void OnInteract() {}   // Click izquierdo
    virtual void OnHover() {}      // Mouse sobre objeto
    virtual void OnGrab() {}       // Click sostenido
    virtual void OnRelease() {}    // Soltar click
    virtual void Draw(Shader& shader) {}
};

// Componentes especializados (composition over inheritance)
struct InteractableComponent {
    InteractionType type;          // DOOR, LIGHT_SWITCH, WINDOW, GRABBABLE
    bool isActive = true;
    bool isToggled = false;        // Para luces/puertas (estado binario)
    float animationProgress = 0.0f; // 0.0 - 1.0
    float animationSpeed = 2.0f;   // Segundos para completar
    glm::vec3 startPos, endPos;    // Para traslaciones
    glm::vec3 startRot, endRot;    // Para rotaciones (puertas/ventanas)
    std::function<void(bool)> onStateChange; // Callback al cambiar estado
};

struct PhysicsComponent {
    bool useGravity = false;
    bool isKinematic = false;      // Si true, se mueve por código, no por física
    glm::vec3 velocity = glm::vec3(0);
    float mass = 1.0f;
    // Colisión simple AABB o esfera
    BoundingVolume bounds;
};

struct LightComponent {
    Light light;                   // Reusa la clase Light existente
    bool isOn = true;
    GameObject* switchObject = nullptr; // Referencia al interruptor que lo controla
};
```

### 3.3 Gestor de Interacciones (Interaction Manager)

```cpp
class InteractionManager {
public:
    // Picking por color (ya implementado en práctica 13)
    GameObject* PickObject(int screenX, int screenY);
    
    // Raycast alternativo (más preciso, para futuro)
    GameObject* RaycastPick(const Camera& camera, double mouseX, double mouseY);
    
    // Estado de interacción actual
    GameObject* hoveredObject = nullptr;
    GameObject* grabbedObject = nullptr;
    bool isGrabbing = false;
    
    void Update(float deltaTime, GLFWwindow* window, Camera& camera);
    void OnMouseButton(int button, int action, int mods);
    void OnMouseMove(double xpos, double ypos);
    
private:
    void HandleGrabRelease(Camera& camera);
    void HandleClick(GameObject* object);
    void UpdateGrabbedObject(Camera& camera);
};
```

### 3.4 Pipeline de Renderizado (Multi-Pass)

```
PASS 1: PICKING BUFFER (offscreen, 1x1 o full-res)
├── Shader: picking.vs/fs
├── Cada GameObject dibuja con su pickingID único como color
└── Output: Texture ID buffer (para leer pixel bajo cursor)

PASS 2: SHADOW MAP (opcional, para luces direccionales)
├── Depth-only pass desde light POV
└── Output: Depth texture

PASS 3: GEOMETRY PASS (Main render)
├── Skybox/Cubemap (primero, depth test LEQUAL)
├── Opaque objects (Phong multi-light + texturas + normal mapping)
├── Transparent objects (ordenados back-to-front)
│   ├── Ventanas (alpha blending)
│   ├── Partículas
│   └── Agua/espejos
└── Debug overlays (wireframe, bounds, picking IDs)

PASS 4: POST-PROCESS (opcional)
├── Bloom (para luces encendidas)
├── Tone mapping
└── FXAA
```

---

## 4. Reglas de Negocio / Lógica de Juego

### 4.1 Definición de Objetos Interactivos

| Tipo | Tag | Comportamiento | Animación | Feedback |
|------|-----|----------------|-----------|----------|
| **Puerta** | `Door` | Click → toggle open/close | Rotación Y (0° ↔ 90°) o translación X | Sonido bisagra, polvo partículas |
| **Interruptor** | `LightSwitch` | Click → toggle luz asociada | Pequeña animación press (traslación Z) | Click sound, luz on/off, cambio material emissive |
| **Ventana** | `Window` | Click → toggle open/close | Rotación X (0° ↔ 45°) o translación Y | Sonido cristal, partícula brisa |
| **Objeto Agarrable** | `Grabbable` | Hold click → attach to camera | Suave follow (lerp) | Outline/shader highlight, sonido agarre |
| **Luz** | `Light` | Controlada por switch | Fade in/out (0.5s) | Cambio intensidad, color temperatura |

### 4.2 Estados de Interacción

```
┌─────────────┐     Mouse Enter      ┌─────────────┐
│   IDLE      │ ──────────────────▶ │   HOVER     │
│ (Nada)      │                      │ (Highlight) │
└─────────────┘                      └──────┬──────┘
      ▲                                      │
      │           Mouse Exit                 │ Left Click
      └──────────────────────────────────────┘
                                              │
                                              ▼
                                    ┌─────────────────┐
                                    │   INTERACTING   │
                                    │ (Door/Window    │
                                    │  toggle, Light  │
                                    │  toggle)        │
                                    └────────┬────────┘
                                             │
                              ┌──────────────┼──────────────┐
                              │              │              │
                        Animation         State           Callback
                        Complete          Changed         (Sound, Particles)
                              │              │              │
                              ▼              ▼              ▼
                                    ┌─────────────┐
                                    │    IDLE     │ (nuevo estado)
                                    └─────────────┘

AGARRE (Hold Click > 0.3s):
┌─────────────┐     Hold Click       ┌─────────────┐     Release      ┌─────────────┐
│   HOVER     │ ──────────────────▶ │  GRABBING   │ ───────────────▶ │   IDLE      │
│             │   (0.3s threshold)  │ (Attached   │   (Drop +        │ (Physics    │
│             │                      │  to camera)│    gravity)      │  enabled)   │
└─────────────┘                      └─────────────┘                  └─────────────┘
```

### 4.3 Reglas de Colisión y Navegación

```cpp
struct NavigationRules {
    // Jugador
    float playerHeight = 1.7f;
    float playerRadius = 0.3f;
    float stepHeight = 0.25f;      // Altura máxima de escalón
    float slopeLimit = 45.0f;      // Grados máximos de pendiente
    
    // Colisión con paredes/puertas
    bool useAABB = true;           // AABB simple vs mesh collision
    float doorOpenAngle = 90.0f;   // Grados para considerar "abierta"
    float interactionDistance = 2.5f; // Distancia máxima para interactuar
    
    // Objetos agarrable
    float grabDistance = 3.0f;
    float holdDistance = 1.5f;     // Distancia frente a cámara al sostener
    float throwForce = 10.0f;      // Si se implementa lanzamiento
};
```

### 4.4 Sistema de Luces (Integración con Práctica 07-08)

```cpp
// Cada luz en la escena tiene un GameObject asociado
struct LightGameObject : GameObject {
    LightComponent lightComp;
    InteractableComponent interactComp; // Tipo: LIGHT_SWITCH
    
    // El interruptor es un GameObject separado que referencia a esta luz
    GameObject* linkedSwitch = nullptr;
    
    void OnInteract() override {
        lightComp.isOn = !lightComp.isOn;
        // Animación fade
        interactComp.animationProgress = 0.0f;
        interactComp.startState = !lightComp.isOn;
        interactComp.endState = lightComp.isOn;
        
        // Callback: sonido, partículas, actualizar shader
        if (interactComp.onStateChange) 
            interactComp.onStateChange(lightComp.isOn);
    }
    
    void Update(float dt) override {
        if (interactComp.animationProgress < 1.0f) {
            interactComp.animationProgress += dt / interactComp.animationSpeed;
            float t = glm::smoothstep(0.0f, 1.0f, interactComp.animationProgress);
            lightComp.light.Power = glm::mix(
                glm::vec4(0), 
                lightComp.basePower, 
                lightComp.isOn ? t : (1.0f - t)
            );
        }
    }
};
```

---

## 5. Estructura de Archivos Propuesta

```
InteractiveHouse/
├── CMakeLists.txt / premake5.lua
├── src/
│   ├── main.cpp                    # Entry point, game loop
│   ├── core/
│   │   ├── Application.h/.cpp      # Wrapper GLFW + loop principal
│   │   ├── Camera.h/.cpp           # Extiende Camera.h existente (FPS + 3rd person)
│   │   ├── Shader.h/.cpp           # Extiende shader_m.h (utilidades extra)
│   │   ├── Model.h/.cpp            # Extiende Model.h (pickingID, animaciones)
│   │   └── ResourceManager.h/.cpp  # Cache de shaders, modelos, texturas
│   ├── ecs/
│   │   ├── GameObject.h/.cpp       # Entidad base
│   │   ├── Transform.h/.cpp        # Pos/Rot/Scale + matriz modelo
│   │   ├── Component.h             # Base para componentes
│   │   └── ComponentTypes.h        # Definiciones de componentes
│   ├── systems/
│   │   ├── InteractionSystem.h/.cpp    # Picking, hover, grab, click
│   │   ├── AnimationSystem.h/.cpp      # Interpolación estados (puertas, luces)
│   │   ├── PhysicsSystem.h/.cpp        # Gravedad, colisiones simples
│   │   ├── RenderSystem.h/.cpp         # Multi-pass render
│   │   └── AudioSystem.h/.cpp          # Wrapper irrKlang
│   ├── gameplay/
│   │   ├── PlayerController.h/.cpp     # Movimiento FPS, input
│   │   ├── Door.h/.cpp                 # Lógica puerta (hereda GameObject)
│   │   ├── LightSwitch.h/.cpp          # Lógica interruptor
│   │   ├── Window.h/.cpp               # Lógica ventana
│   │   ├── GrabbableObject.h/.cpp      # Lógica agarre
│   │   └── LightController.h/.cpp      # Gestor luces de la escena
│   └── scene/
│       ├── Scene.h/.cpp                # Grafo de escena, carga nivel
│       └── HouseScene.cpp              # Setup específico de la casa
├── shaders/
│   ├── picking.vs/.fs                  # Copy de bin/shaders/15_picking.*
│   ├── phong_lights.vs/.fs             # Copy de 11_PhongShaderMultLights.*
│   ├── skinned.vs/.fs                  # Copy de 10_vertex_skinning-IT.*
│   ├── cubemap.vs/.fs                  # Copy de 10_vertex_cubemap.*
│   ├── outline.vs/.fs                  # NUEVO: Post-process outline (hover)
│   └── ui.vs/.fs                       # NUEVO: UI/HUD simple
├── assets/
│   ├── models/                         # FBX de la casa, puertas, objetos
│   ├── textures/                       # Diffuse, normal, specular, emissive
│   ├── sounds/                         # .ogg/.wav para interacciones
│   └── cubemaps/                       # Skybox
└── bin/                                # Build output (DLLs, shaders, assets)
```

---

## 6. Plan de Implementación por Fases

### Fase 1: Fundación (Semana 1-2)
- [ ] Configurar proyecto CMake + copiar headers base (`camera.h`, `shader_m.h`, `model.h`, `mesh.h`, `light.h`, `material.h`, `cubemap.h`, `modelstructs.h`, `animatedmodel.h`)
- [ ] `Application` class: init GLFW/GLAD, loop principal, deltaTime
- [ ] `Camera` FPS: movimiento WASD + mouse look + colisión AABB básica
- [ ] `ResourceManager`: carga perezosa de shaders, modelos, texturas
- [ ] Cargar `House03.fbx` + skybox + iluminación básica (Phong multi-light)

### Fase 2: Sistema de Picking e Interacción Base (Semana 2-3)
- [ ] Implementar **Picking Pass** (offscreen FBO o leer pixel centro pantalla)
- [ ] `InteractionManager`: hover, click, estados
- [ ] `GameObject` base + `InteractableComponent`
- [ ] **Test**: Click en cubo → cambia color (validar pickingID)

### Fase 3: Objetos Interactivos Core (Semana 3-4)
- [ ] `Door`: Animación rotación Y (0°↔90°), sonido, colisión dinámica
- [ ] `LightSwitch`: Toggle luz asociada, fade in/out, material emissive
- [ ] `Window`: Animación rotación X (0°↔45°), colisión al abrir
- [ ] Integrar en `HouseScene`: colocar 3-4 puertas, 2-3 interruptores, 2 ventanas

### Fase 4: Sistema de Agarre (Semana 4-5)
- [ ] `GrabbableObject`: Hold click → attach a cámara (lerp suave)
- [ ] PhysicsComponent: gravedad al soltar, colisión suelo
- [ ] Outline shader (post-process) para feedback visual hover/grab
- [ ] 5-6 objetos agarrable en la casa (libros, vasos, sillas pequeñas)

### Fase 5: Pulido y Extras (Semana 5-6)
- [ ] Sonidos (irrKlang): pasos, puertas, interruptores, agarre, drops
- [ ] Partículas: polvo al abrir puertas, chispas en interruptores
- [ ] HUD simple: crosshair, tooltip interacción ("[E] Abrir", "[Hold] Agarrar")
- [ ] Guardar/cargar estado (JSON: posiciones objetos, estados luces/puertas)
- [ ] Optimización: Frustum culling, instancing para objetos repetidos
- [ ] Build release + README + controles

---

## 7. Decisiones Técnicas Clave

### 7.1 Picking: Color ID vs Raycast
**Decisión: Color ID (práctica 13)**
- **Pros:** Ya implementado, GPU-accelerado, pixel-perfect, funciona con geometría compleja
- **Contras:** Requiere pass extra, 1 frame de latencia, limited a 16M objetos (24-bit)
- **Implementación:** FBO offscreen 1x1 pixel en centro de pantalla (performance) o full-res

### 7.2 Animaciones: Keyframe vs Procedural
**Decisión: Procedural (lerp/smoothstep)**
- Puertas/ventanas/interruptores: interpolación simple start→end
- No requiere sistema de animación complejo (ahorra tiempo)
- Extensible a keyframes si se necesita después

### 7.3 Física: Completa vs Kinemática
**Decisión: Kinemática + Gravedad Simple**
- Objetos agarrable: kinemáticos mientras se sostienen
- Al soltar: gravedad simple (velocity.y -= 9.8 * dt) + colisión AABB suelo/paredes
- Sin motor de física completo (Box2D/Bullet) - overkill para este scope

### 7.4 Colisiones: AABB vs Mesh
**Decisión: AABB + NavMesh Simple**
- Jugador: cápsula/AABB vs AABB de paredes/puertas
- Puertas abiertas: actualizar AABB o desactivar colisión
- Objetos pequeños: esfera vs AABB

### 7.5 Arquitectura: Herencia vs ECS
**Decisión: Híbrido (GameObject + Components)**
- `GameObject` base con `Transform` + virtual `Update/Draw`
- Components opcionales: `InteractableComponent`, `PhysicsComponent`, `LightComponent`
- Fácil migración a ECS puro si el proyecto crece

---

## 8. Riesgos y Mitigaciones

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|--------------|---------|------------|
| Picking no funciona en objetos transparentes | Alta | Media | Render picking pass ANTES de transparentes; ignorar alpha < 0.1 en shader picking |
| Animaciones de puertas se traban con colisión | Media | Alta | Desactivar colisión puerta mientras se anima; usar triggers |
| Performance con muchas luces dinámicas | Media | Media | Máx 8-16 luces por shader (UBO/SSBO); baked lightmaps para estáticas |
| Assimp no carga animaciones FBX correctamente | Baja | Alta | Validar modelos temprano; tener modelos backup (.obj/.gltf) |
| irrKlang problemas de licencia/distribución | Baja | Media | Usar OpenAL/SoLoud como alternativa; incluir DLLs en bin/ |

---

## 9. Criterios de Aceptación (Definition of Done)

### MVP (Entregable Mínimo)
- [ ] Compila y ejecuta sin errores en Windows (VS2022 / MinGW)
- [ ] Navegación FPS fluida (60 FPS @ 1080p, GTX 1060+)
- [ ] Click en puerta → se abre/cierra con animación suave
- [ ] Click en interruptor → luz asociada enciende/apaga (fade)
- [ ] Click en ventana → se abre/cierra
- [ ] Hold click en objeto → lo agarra, mover mouse → mueve objeto, soltar → cae con gravedad
- [ ] Skybox visible, iluminación coherente
- [ ] Sin memory leaks (valgrind / VS diagnostic tools)

### Calidad de Código
- [ ] Separación clara: core / systems / gameplay / scene
- [ ] Sin globals excesivos (máximo: Application, ResourceManager, Scene)
- [ ] RAII para recursos OpenGL (shaders, VAOs, textures, FBOs)
- [ ] Const-correctness, no raw owning pointers
- [ ] Documentación Doxygen en clases públicas

---

## 10. Próximos Pasos Inmediatos

1. **Crear estructura de carpetas** y `CMakeLists.txt`
2. **Copiar headers base** de `08_IlluminationModelsII/include/` a `src/core/`
3. **Extraer shaders** de `bin/shaders/` a `shaders/`
4. **Crear `Application` class** envolviendo el loop principal
5. **Test rápido**: Cargar `House03.fbx` + skybox + 1 luz + cámara FPS
6. **Validar picking** con el shader `15_picking.vs/fs` existente

---

¿Te gustaría que profundice en algún aspecto específico? Por ejemplo:
- Detalle técnico del **InteractionManager** y picking
- Diseño del **sistema de animaciones** para puertas/ventanas
- Estructura del **CMakeLists.txt** y dependencias
- **HouseScene** layout (dónde colocar cada objeto interactivo)
- Integración de **irrKlang** para audio posicional