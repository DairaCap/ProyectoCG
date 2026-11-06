# Arquitectura de Software del Proyecto Final: Casa Interactiva 3D

> **Asignatura:** Computación Gráfica e Interacción Humano-Computadora  
> **Semestre:** 2027-1 | Facultad de Ingeniería, UNAM  
> **Profesor:** Dr. Sergio Teodoro Vite  
> **Alineación:** Especificaciones del Proyecto Final + Anteproyecto + Prácticas de Laboratorio (00 a 14)

---

## 1. Mapeo y Trazabilidad con las Prácticas del Curso

Para garantizar compatibilidad técnica absoluta y evitar la invención de dependencias o estructuras ajenas al curso, el proyecto reutiliza estrictamente el ecosistema de código, bibliotecas (`deps/`), shaders (`bin/shaders/`) y assets (`bin/models/`, `bin/textures/`) desarrollados en las 14 prácticas:

| Práctica / Módulo del Curso | Componente o Técnica en Práctica | Aplicación en la Arquitectura de la Casa Interactiva | Shaders / Clases Reutilizadas del Repositorio |
|---|---|---|---|
| **00 & 01 - Setup / Pipeline** | GLAD, GLFW, Contexto OpenGL 3.3 Core Profile, VBO/VAO/EBO | Ventana, contexto, configuración de depth buffer, viewport y ciclo principal | `glad/glad.h`, `GLFW/glfw3.h`, `Shader.h` / `shader_m.h` |
| **02 - Viewport & Proyecciones** | `Camera.h` (cámara libre / FPS en primera persona) | **Cámara 1:** Primera Persona (FPS) con navegación dentro de la casa | `camera.h` (`ProcessKeyboard`, `ProcessMouseMovement`, `Zoom`) |
| **03 - Primitivas & 04 - Jerárquico** | Primitivas (`Cube.h`, `Sphere.h`, etc.) y jerarquías matriciales con GLM | Colisiones geométricas simples (AABB) y articulación jerárquica de puertas/ventanas | Transformaciones locales respecto a pivote de bisagra (`glm::rotate`, `glm::translate`) |
| **05 - Model Loading** | `model.h`, `mesh.h`, `modelstructs.h`, Assimp v5.0 | Carga de la casa (`House02.fbx` / `House03.fbx`), puertas, mobiliario y accesorios | `Model::Draw(Shader&)`, `Mesh`, estructuras de vértices |
| **06 - Texturizado** | `stb_image.h`, mapas diffuse, specular y normal | Mapeo UV de maderas, muros, techos, suelos y detalles decorativos | `stb_image.cpp`, texturas 2D en slots `GL_TEXTURE0`, etc. |
| **07 & 08 - Iluminación** | Phong con múltiples fuentes de luz (`Light`, `Material`) | Iluminación de habitaciones: luces direccionales (sol/luna), puntuales (focos de techo/lámparas) | `light.h`, `material.h`, `11_PhongShaderMultLights.vs/fs`, `10_fragment_simpleLights.fs` |
| **09 & 10 - Animación** | `AnimatedModel` (skeletal skinning) y cámara en 3ª persona | **Cámara 2:** Tercera Persona orbitando a avatar (`KAYA.fbx`) recorriendo la casa; animaciones de puertas | `animatedmodel.h`, `10_vertex_skinning-IT.vs/fs`, `camera3rd` vinculada al personaje |
| **11 - Partículas** | `particles.h`, texturas con alpha blending (`GL_BLEND`) | Efectos de ambiente: polvo en habitaciones, humo de chimenea o lluvia/nieve exterior | `particles.h`, `13_particles.vs/fs` |
| **12 - Espejos / FBO** | Framebuffer Object (render-to-texture), plano de reflexión | Espejo del baño o vestidor reflejando la habitación | `14_mirror.vs/fs`, textura de reflexión FBO |
| **13 - Picking** | Selección por color codificado en 24-bit (`glReadPixels`) | **Motor de Interacción:** Identificación de interruptores, puertas, ventanas y objetos | `15_picking.vs/fs`, `IDforPicking` en cada `Model` |
| **14 - Fresnel** | Reflexión/Refracción dieléctrica con Cubemap | Materiales traslúcidos (cristales de ventanas, peceras) y metales (grifos, manijas) | `11_Fresnel.vs/fs`, `mRefractionRatio`, `_Bias`, `_Scale`, `_Power` |
| **Ambiente & Audio** | `CubeMap` (Skybox 6 caras) y motor de audio irrKlang | Cielo diurno/nocturno (`cubemap.h`) y audio espacial 3D/2D para puertas, interruptores y pasos | `cubemap.h`, `10_vertex_cubemap.vs/fs`, `irrKlang.h` (`ISoundEngine`) |

---

## 2. Cumplimiento de Especificaciones y Rúbrica Oficial (2027-1)

El diseño arquitectónico atiende al 100% de los criterios de la rúbrica oficial evaluada sobre 10 puntos:

```
┌──────────────────────────────────────────────────────────────────────────────────────────────────┐
│                      MATRIZ DE CUMPLIMIENTO DE RÚBRICA DEL PROYECTO                             │
├──────────────────────────────────────┬─────────┬─────────────────────────────────────────────────┤
│ Requisito Oficial de la Rúbrica      │ Puntos  │ Solución en la Arquitectura                     │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ Escena con >= 80 modelos geométricos │ 2.0 pts │ Escena completa de la casa dividida en sectores │
│ (>= 80% creación/autoría)            │         │ con mobiliario, estructura y decoraciones       │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ Mínimo 2 modelos de control de cámara│ 1.0 pts │ Modo 1: Cámara Primera Persona (FPS interactiva)│
│                                      │         │ Modo 2: Cámara Tercera Persona (avatar KAYA)   │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ Interacción con >= 15 elementos      │ 1.0 pts │ 4 puertas + 4 interruptores + 3 ventanas +      │
│ (tomar, rotar, trasladar, etc.)      │         │ 4 objetos manipulables/agarrables (15 objetos)  │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ Materiales: opacos, plásticos,       │ 1.0 pts │ Phong (opacos y plásticos), Fresnel (vidrios    │
│ traslúcidos y metálicos              │         │ de ventana y accesorios metálicos)              │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ Texturizado y Cubemap de ambiente    │ 0.5 pts │ Skybox 6 caras con `cubemap.h` + mapas difusos, │
│ de alta calidad                      │         │ normales y especulares en superficies           │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ 10 Animaciones simples               │ 2.0 pts │ - 10 Simples: 4 puertas, 3 ventanas, 3 switches │
│ 5 Animaciones por keyframes          │ (Total  │ - 5 Keyframes: Caminado/Idle de KAYA, cajones,  │
│ 5 Animaciones procedurales           │ Anim.)  │   péndulo reloj, apertura portón, abanico techo │
│                                      │         │ - 5 Procedurales: Chimenea, olas/agua piscina,  │
│                                      │         │   parpadeo luz, cortinas al viento, oscilación  │
├──────────────────────────────────────┼─────────┼─────────────────────────────────────────────────┤
│ Empaquetado InstallForge (.exe),     │ 2.5 pts │ Instalador autónomo con DLLs requeridas y       │
│ manual.pdf y ejecución funcional     │         │ manual de usuario e instalación estructurado    │
└──────────────────────────────────────┴─────────┴─────────────────────────────────────────────────┘
```

---

## 3. Arquitectura del Sistema: Patrón del Curso

La arquitectura sigue el patrón estructurado en funciones globales y módulos limpios que los alumnos dominan desde las prácticas 08, 13 y 14, evitando frameworks externos innecesarios:

```
                           ┌──────────────────────────┐
                           │         main()           │
                           │   ┌──────────────────┐   │
                           │   │     Start()      │   │
                           │   └────────┬─────────┘   │
                           │            ▼             │
                           │   ┌──────────────────┐   │
                           │   │  While running:  │   │
                           │   │     Update()     │   │
                           │   └──────────────────┘   │
                           └────────────┬─────────────┘
                                        │
      ┌──────────────────┬──────────────┼──────────────┬──────────────────┐
      ▼                  ▼              ▼              ▼                  ▼
┌───────────┐      ┌───────────┐  ┌───────────┐  ┌───────────┐      ┌───────────┐
│ Input &   │      │ Scene &   │  │ Shaders   │  │ Physics & │      │ Audio     │
│ Cameras   │      │ Objects   │  │ Pipeline  │  │ Picking   │      │ Engine    │
├───────────┤      ├───────────┤  ├───────────┤  ├───────────┤      ├───────────┤
│- GLFW     │      │- House    │  │- Phong    │  │- Offscreen│      │- irrKlang │
│  callbacks│      │  (Static) │  │  MultLight│  │  Pick FBO │      │  ISound-  │
│- Cam 1st  │      │- Doors &  │  │- Fresnel  │  │- Color ID │      │  Engine   │
│  Person   │      │  Windows  │  │  Shader   │  │  Decodif. │      │- Spatial  │
│- Cam 3rd  │      │- Switches │  │- Skinned  │  │- AABB     │      │  3D Sound │
│  Person   │      │- Props    │  │  Shader   │  │  Collision│      │- SFX UI   │
│- Key/Mouse│      │- Avatar   │  │- Cubemap  │  │- Drag/Drop│      │           │
└───────────┘      └───────────┘  └───────────┘  └───────────┘      └───────────┘
```

### 3.1 Ciclo de Ejecución (Start & Update)

1. **`Start()`**:
   - Inicializa GLFW (versión 3.3 Core Profile) y crea ventana de $1024 \times 768$ (o pantalla completa).
   - Registra callbacks: `framebuffer_size_callback`, `mouse_callback`, `scroll_callback`, `mouseButtonCallback`.
   - Inicializa GLAD para enlazar las funciones del API OpenGL.
   - Habilita `GL_DEPTH_TEST` y modo de blending para transparencias.
   - Compila el repertorio de shaders:
     - `mLightsShader` (`shaders/11_PhongShaderMultLights.*`)
     - `fresnelShader` (`shaders/11_Fresnel.*`)
     - `dynamicShader` (`shaders/10_vertex_skinning-IT.*`)
     - `cubemapShader` (`shaders/10_vertex_cubemap.*`)
     - `pickingShader` (`shaders/15_picking.*`)
     - `particlesShader` (`shaders/13_particles.*`)
     - `proceduralShader` (`shaders/12_ProceduralAnimation.*`)
   - Instancia y carga los modelos 3D (`Model`, `AnimatedModel`) asignando a cada uno su identificador entero `IDforPicking`.
   - Carga el Skybox (`CubeMap`) con texturas de día/noche.
   - Inicializa fuentes de luz (`Light`) y materiales base (`Material`).
   - Inicializa el dispositivo de sonido `createIrrKlangDevice()`.

2. **`Update()`**:
   - Cálculo estricto de tiempo entre cuadros: `deltaTime = currentFrame - lastFrame`.
   - Procesamiento continuo de entradas en `processInput(window)`.
   - **Pase 1 (Picking si hubo click o hover interactivo):**
     - Renderiza con `pickingShader` los objetos interactuables en buffer de selección con codificación de color RGB según `IDforPicking`.
     - Lee el pixel bajo el cursor (`glReadPixels`) y recupera el objeto apuntado.
   - **Pase 2 (Renderizado Principal de la Escena):**
     - Limpieza de buffers de color y profundidad (`GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT`).
     - Selección de matriz de proyección y vista según la cámara activa (Primera Persona o Tercera Persona).
     - Render del Skybox (`CubeMap::drawCubeMap`).
     - Render de objetos opacos (estructuras, paredes, muebles) con `mLightsShader`.
     - Render de objetos animados por software/matriz (puertas abriendo, interruptores presionados).
     - Render del personaje animado por huesos con `dynamicShader`.
     - Render de superficies transparentes/reflectivas (ventanas, espejos, vidrios) con `fresnelShader` ordenados traslúcidos.
     - Render de sistemas de partículas (fuego/humo de chimenea) con `particlesShader`.
   - Intercambio de buffers (`glfwSwapBuffers`) y captura de eventos (`glfwPollEvents`).

---

## 4. Modelos de Cámara y Navegación

Cumpliendo el requisito de al menos 2 modelos de cámara conmutables en tiempo de ejecución:

```cpp
// Variables de control de cámara (en el estilo de práctica 09 y 13)
Camera camera(glm::vec3(0.0f, 1.7f, 5.0f));     // Cámara FPS (altura de ojos: 1.7m)
Camera camera3rd(glm::vec3(0.0f, 0.0f, 0.0f));  // Cámara 3ra Persona orbitando avatar
bool activeCamera = 1;                           // 1 = 1ra persona, 0 = 3ra persona

// Parámetros de órbita para 3ra persona
float trdpersonOffset = 4.0f;
float trdpersonHeight = 1.8f;
glm::vec3 avatarPosition(0.0f, 0.0f, 0.0f);
glm::vec3 forwardView(0.0f, 0.0f, 1.0f);
```

- **Cámara 1 - Primera Persona (FPS):**
  - Ideal para inspeccionar la casa, acercarse a objetos, leer notas e interactuar directamente con interruptores y puertas.
  - Teclas `W`, `A`, `S`, `D` para desplazamiento horizontal sobre el plano $XZ$ de la casa.
  - Rotación libre mediante `mouse_callback` (`Yaw` y `Pitch`).
  - Colisión básica contra los muros de la casa mediante cajas delimitadoras AABB.

- **Cámara 2 - Tercera Persona (Avatar Follow):**
  - Muestra al personaje `KAYA.fbx` caminando por la casa.
  - Teclas `UP`, `DOWN` mueven el personaje hacia adelante/atrás en dirección a su vector de orientación `forwardView`.
  - Teclas `LEFT`, `RIGHT` rotan el ángulo de giro del personaje (`rotateCharacter`).
  - La cámara `camera3rd` se posiciona automáticamente detrás de KAYA:
    $$\text{pos}_{\text{cam3rd}} = \text{avatarPosition} + (0, 1.8, 0) - (\text{trdpersonOffset} \cdot \text{forwardView})$$
  - Conmutación directa entre cámaras con teclas de acceso rápido `[F1]` (Tercera persona) y `[F2]` (Primera persona).

---

## 5. Sistema de Picking y Catálogo de 15 Objetos Interactivos

### 5.1 Mecanismo de Selección por Color (de la Práctica 13)

El picking opera exactamente con el shader `15_picking.vs` y `15_picking.fs`:
1. Cada modelo interactivo posee un `unsigned int IDforPicking` asignado en su constructor:
   ```cpp
   // Asignación de ID único
   house->IDforPicking       = 100;
   doorLivingRoom->IDforPicking = 201;
   switchLivingRoom->IDforPicking = 301;
   ```
2. Al pulsar el botón izquierdo del ratón (`GLFW_MOUSE_BUTTON_LEFT`), en `mouseButtonCallback`:
   ```cpp
   unsigned char data[4];
   glReadPixels(SCR_WIDTH / 2, SCR_HEIGHT / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
   unsigned int pickedID = data[0] + (data[1] << 8) + (data[2] << 16);
   ```
3. Se despacha el evento de interacción correspondiente según el ID decodificado.

### 5.2 Catálogo Detallado de los 15 Elementos Interactivos

| # | Objeto | ID Picking | Habitación | Tipo de Interacción | Efecto Visual / Animación | Efecto de Sonido (irrKlang) |
|---|---|---|---|---|---|---|
| **1** | Puerta Principal | `201` | Fachada / Acceso | Click izquierdo | Rotación sobre bisagra $Y$ ($0^\circ \to 90^\circ$) | `sound/door_open.wav` |
| **2** | Puerta Dormitorio | `202` | Planta Alta | Click izquierdo | Rotación sobre bisagra $Y$ ($0^\circ \to 85^\circ$) | `sound/door_wood.wav` |
| **3** | Puerta Cocina / Jardín | `203` | Cocina | Click izquierdo | Deslizamiento lateral corredizo en $X$ ($0 \to 1.2\text{m}$) | `sound/door_slide.wav` |
| **4** | Puerta de Baño | `204` | Baño | Click izquierdo | Rotación sobre bisagra $Y$ ($0^\circ \to -90^\circ$) | `sound/door_close.wav` |
| **5** | Switch Sala | `301` | Sala Principal | Click izquierdo | Oprime botón + Conmuta Luz puntual de Techo Sala | `sound/switch_click.wav` |
| **6** | Switch Cocina | `302` | Cocina | Click izquierdo | Oprime botón + Conmuta Focos de Barra Cocina | `sound/switch_click.wav` |
| **7** | Switch Dormitorio | `303` | Dormitorio | Click izquierdo | Oprime botón + Conmuta Lámparas de Mesa de noche | `sound/switch_click.wav` |
| **8** | Switch Baño | `304` | Baño | Click izquierdo | Oprime botón + Conmuta Luz de Espejo de Baño | `sound/switch_click.wav` |
| **9** | Ventana Fachada | `401` | Sala Frontal | Click izquierdo | Apertura tipo abatible, rotación en $Y$ ($35^\circ$) | `sound/window_open.wav` |
| **10**| Ventana Cocina | `402` | Cocina | Click izquierdo | Apertura tipo guillotina, traslación vertical $+Y$ | `sound/window_slide.wav` |
| **11**| Ventana Balcón | `403` | Dormitorio Balcón| Click izquierdo | Abatible exterior, rotación en $Y$ ($45^\circ$) | `sound/wind_breeze.wav` |
| **12**| Lámpara Portátil / Linterna| `501` | Sala | Click sostenido (Drag) | Objeto se adhiere a cámara; soltar activa gravedad | `sound/grab.wav` / `drop.wav`|
| **13**| Silla de Comedor | `502` | Comedor | Click sostenido (Drag) | Permite arrastrarla y desplazarla en plano del piso | `sound/chair_drag.wav` |
| **14**| Libro / Diario | `503` | Estudio / Sala | Click izquierdo | Lo levanta al frente de cámara y abre cubierta | `sound/page_turn.wav` |
| **15**| Taza / Vaso de Vidrio | `504` | Cocina | Click sostenido (Drag) | Se toma en mano; usa material translúcido Fresnel | `sound/glass_clink.wav` |

---

## 6. Iluminación y Catálogo de Materiales

La escena combina dos técnicas de iluminación y sombreado explícitamente requeridas por el curso:

### 6.1 Técnica 1: Modelo de Phong con Múltiples Luces (`11_PhongShaderMultLights`)

Representación de materiales **opacos y plásticos** (muros, pisos de parqué, madera de muebles, techos):
- **Ecuación implementada en fragment shader:**
  $$I_{\text{total}} = I_{\text{ambient}} + \sum_{k=1}^{N} \left( I_{\text{diffuse}}^{(k)} + I_{\text{specular}}^{(k)} \right) \cdot \text{atenuación}^{(k)}$$
- **Configuración de fuentes de luz:**
  1. *Luz Direccional (Ambiental Exterior):* Simula luz solar/lunar que entra por las ventanas.
  2. *Luces Puntuales (Interiores):* Una por cada habitación, controladas por su respectivo switch (`gLights[i].Power = isOn ? glm::vec4(60.0f) : glm::vec4(0.0f)`).
  3. *Luz Focal (Spotlight linterna):* Cono de luz acoplado a la cámara en primera persona para recorrer cuartos oscuros.

### 6.2 Técnica 2: Modelo de Fresnel (`11_Fresnel`)

Representación de materiales **translúcidos y metálicos** (ventanas, espejos, grifería, cristalería):
- Utiliza la aproximación de Schlick en GPU para mezclar reflexión y refracción sobre el CubeMap ambiental:
  $$\text{factorFresnel} = \text{\_Bias} + \text{\_Scale} \cdot (1.0 + \mathbf{I} \cdot \mathbf{N})^{\text{\_Power}}$$
- **Perfiles de materiales calibrados:**
  - *Vidrios de ventana y vasos (Translúcido):* `mRefractionRatio = 1.0f / 1.52f` (Vidrio común), `_Bias = 0.05f`, `_Scale = 0.95f`, `_Power = 2.0f`.
  - *Grifos de baño/cocina y pomos (Metálico):* `mRefractionRatio = 1.0f / 2.40f`, `_Bias = 0.85f`, `_Scale = 0.15f`, `_Power = 1.0f`.
  - *Espejo del baño:* Reflexión planar directa mediante cubemap/FBO con coeficiente de reflexión $> 0.95$.

---

## 7. Desglose Exhaustivo de las 20 Animaciones Requeridas

La especificación exige exactamente 20 animaciones clasificadas en tres familias:

### 7.1 Diez (10) Animaciones Simples
Movimientos basados en interpolación directa de matrices de transformación (traslación, rotación, escala con función smoothstep de GLM):

1. **Apertura/Cierre Puerta Principal:** Rotación paramétrica en $Y$ de $0^\circ$ a $90^\circ$.
2. **Apertura/Cierre Puerta Dormitorio:** Rotación paramétrica en $Y$ de $0^\circ$ a $85^\circ$.
3. **Deslizamiento Puerta Cocina:** Traslación en $X$ de $0$ a $1.2\text{m}$.
4. **Apertura Puerta Baño:** Rotación en $Y$ de $0^\circ$ a $-90^\circ$.
5. **Basculación Switch Sala:** Desplazamiento local en $Z$ y rotación $15^\circ$ al oprimir.
6. **Basculación Switch Cocina:** Desplazamiento y cambio de color emisivo al oprimir.
7. **Basculación Switch Dormitorio:** Toggle visual de palanca on/off.
8. **Basculación Switch Baño:** Toggle visual de tecla on/off.
9. **Apertura Ventana Fachada:** Rotación angular abatible en $Y$ hacia el exterior.
10. **Apertura Ventana Cocina:** Elevación guillotina en $+Y$ de $0$ a $0.6\text{m}$.

### 7.2 Cinco (5) Animaciones Basadas en Cuadros Clave (Keyframes)
Animaciones complejas con secuenciamiento de tiempo, huesos esqueléticos o cadenas de keyframes interpolados:

11. **Ciclo de Caminado de KAYA (`KAYA.fbx`):** Skeletal skinning multihueso interpolado por Assimp/GLM (`dynamicShader`, `gBones[100]`).
12. **Ciclo Idle / Respiración de KAYA:** Animación esquelética de descanso cuando el personaje está detenido.
13. **Péndulo del Reloj de Pared:** Movimiento armónico con amortiguamiento y marcación periódica de cuadrantes.
14. **Apertura Secuencial de Cajones de Cómoda:** Keyframe 1: apertura primer cajón, Keyframe 2: segundo cajón, Keyframe 3: retorno suave.
15. **Apertura Cinemática de Portón de Cochera:** Elevación vertical inicial y curvado horizontal bajo el techo en dos fases cinemáticas.

### 7.3 Cinco (5) Animaciones Procedurales
Animaciones generadas matemáticamente en tiempo de ejecución (funciones trigonométricas, ruido o manipulación en vertex/fragment shader):

16. **Llama y Humo en Chimenea:** Variación procedural en shader (`12_ProceduralAnimation.vs/fs`) deformando vértices y colores con senos y cosenos en función de `glfwGetTime()`.
17. **Ondulaciones de Agua en Fuente / Tina de Baño:** Implementada con `13_wavesAnimation.vs/fs` usando perturbación sinusoidal de normales en la malla plana.
18. **Parpadeo Estocástico de Lámpara Antigua:** Variación pseudo-aleatoria de la potencia luminosa (`glm::sin(time * 15.0f) * fract(sin(time))`) simulando falso contacto.
19. **Movimiento Suave de Cortinas por la Brisa:** Deformación armónica de los vértices inferiores de la tela ($z = A \cdot \sin(\omega t + ky)$).
20. **Ventilador de Techo en Rotación Continua:** Velocidad angular procedural con aceleración al encender y desaceleración por fricción al apagar ($\theta(t) = \theta_0 + \omega t$).

---

## 8. Estructura de Proyecto en Visual Studio (MSVC)

A diferencia de proyectos abstractos, esta estructura encaja perfectamente dentro de la solución `Proyecto/Proyecto/Proyecto.vcxproj` existente y enlaza con los directorios de librerías oficiales de la UNAM:

```
C:\Users\daira\Documents\CompuG\
├── bin/                               # Ejecutable final, DLLs y Assets
│   ├── assimp-vc143-mt.dll            # DLLs oficiales
│   ├── irrKlang.dll
│   ├── ikpMP3.dll
│   ├── models/                        # Modelos de la casa, KAYA, puertas, muebles
│   ├── shaders/                       # Shaders GLSL de las prácticas (01 a 15)
│   ├── textures/                      # Texturas y cubemaps
│   └── sound/                         # Efectos WAV/MP3 para irrKlang
├── deps/                              # Bibliotecas de cabecera y estáticas
│   ├── glad/ (MSVC2022/include & lib)
│   ├── glfw/ (include & lib-vc2022)
│   ├── glm/ (include)
│   ├── assimp/ (MSVC2022/include & lib)
│   └── irrklang/ (include & lib/x64)
└── Proyecto/                          # Directorio de trabajo del alumno
    ├── Anteproyecto.pdf               # Documento base de partida
    ├── Especificaciones...pdf         # Rúbrica oficial del profesor
    ├── Cimientos.blend                # Modelo base en Blender
    └── Proyecto/                      # Código C++ del Proyecto Final
        ├── Proyecto.slnx / .vcxproj   # Solución oficial Visual Studio 2022
        ├── main.cpp                   # Función main(), Start() y Update()
        ├── stb_image.cpp              # Implementación STB
        ├── HouseScene.h / .cpp        # Definición de los 80+ objetos y cuartos
        ├── InteractionSystem.h / .cpp # Lógica de Picking y respuesta a clicks
        ├── AnimationController.h      # Manejo de los 3 tipos de animaciones
        └── Player.h / .cpp            # Control de cámara FPS y colisiones
```

### 8.1 Configuración de Enlace en `Proyecto.vcxproj` (x64 Release / Debug)

- **C/C++ -> General -> Additional Include Directories:**
  ```
  ..\..\deps\glad\MSVC2022\include;
  ..\..\deps\glfw\include;
  ..\..\deps\glm\include;
  ..\..\deps\assimp\MSVC2022\include;
  ..\..\deps\irrklang\include;
  ..\..\08_IlluminationModelsII\Proyecto\08_IlluminationModelsII\include;
  %(AdditionalIncludeDirectories)
  ```
- **Linker -> General -> Additional Library Directories:**
  ```
  ..\..\deps\glad\MSVC2022\lib;
  ..\..\deps\glfw\lib-vc2022;
  ..\..\deps\assimp\MSVC2022\lib;
  ..\..\deps\irrklang\lib\x64;
  %(AdditionalLibraryDirectories)
  ```
- **Linker -> Input -> Additional Dependencies:**
  ```
  opengl32.lib;
  glfw3.lib;
  glad.lib;
  assimp-vc143-mt.lib;
  irrKlang.lib;
  %(AdditionalDependencies)
  ```

---

## 9. Plan de Empaquetado y Entregables Finales

En estricta conformidad con las normas de entrega del curso:
1. **No se entrega código fuente al final:** El código se evalúa durante las revisiones de avances; la entrega final se compone de:
   - **`ProyectoFinal_Instalador.exe`**: Creado con **InstallForge**, empaquetando el ejecutable Release x64, las carpetas `bin/models`, `bin/shaders`, `bin/textures`, `bin/sound` y las DLLs requeridas (`assimp-vc143-mt.dll`, `irrKlang.dll`, `ikpMP3.dll`).
   - **`manual.pdf`**: Documento con requerimientos de sistema, guía de instalación paso a paso, controles de teclado/mouse para ambas cámaras y descripción de las 15 interacciones.
   - **Reporte escrito en Español (10%) e Inglés (10%)**: Con portada formal, hoja de evidencias, resumen, introducción, metodología, resultados, experimentos y conclusiones individuales por integrante.
   - **Video Demostrativo (30%)**: Recorrido de 3 a 5 minutos mostrando los 80 modelos, ambas cámaras, las 15 interacciones, las 20 animaciones y los dos modelos de iluminación (Phong y Fresnel).
