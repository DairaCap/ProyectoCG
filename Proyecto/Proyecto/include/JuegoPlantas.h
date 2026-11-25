#pragma once
// ============================================================================
// JuegoPlantas.h - Minijuego "Acomoda las plantas".
//
// Flujo:
//   CERRADA  -> cerca del checkpoint del patio sale el aviso "PRESIONA E"; al presionar E -> REGLAS
//   REGLAS   -> ENTER o E -> NIVEL
//   NIVEL    -> tecla 1, 2 o 3 (Basico, Intermedio, Experto) -> JUGANDO
//   JUGANDO  -> 8 plantas al azar se teletransportan al patio. Se agarran MANTENIENDO el clic izquierdo
//               (la planta flota frente a la mira) y se sueltan al dejar el clic. ENTER califica:
//               aura verde = bien colocada, roja = mal. Las rojas se pueden volver a agarrar.
//   FINAL    -> cuando TODAS estan en verde: tiempo y numero de calificaciones.
//
// Niveles (pistas al agarrar una planta):
//   En todos los niveles se ven los circulos (auras) de los 8 lugares correctos.
//   1 Basico:      el aura de la planta que cargas se resalta + zona + distancia y direccion
//   2 Intermedio:  auras iguales + nombre de la zona
//   3 Experto:     auras iguales, sin nombre ni pistas (hay que recordar a quien le toca cada una)
//
// Las plantas se mueven con Muebles::plantaMatriz / plantaMovida (ver Muebles.h). El lugar correcto de cada
// planta es su lugar original (el centro de su caja en plantas.csv); cuenta como bien si queda a menos de
// TOLERANCIA metros en el plano (la altura no importa: una planta de repisa en el piso debajo cuenta).
// ============================================================================
#include <string>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "UI.h"
#include "Muebles.h"
#include "Aura.h"
#include "PuntoLibre.h"

class JuegoPlantas {
public:
    enum Estado { CERRADA, REGLAS, NIVEL, JUGANDO, FINAL };

    static constexpr int POR_JUEGO = 8;            // Plantas que se usan en cada partida (al azar de las 15 con tarjeta)
    static constexpr float TOLERANCIA = 0.5f;      // m: a esta distancia del lugar original cuenta como bien colocada

    // Nombre para mostrar y zona de la casa de cada planta. REVISAR las zonas contra el plano de la casa.
    struct Info { const char* clave; const char* nombre; const char* zona; };
    static inline const Info INFO[] = {
        { "Planta_Rosal",        "Rosal",              "Jardín" },
        { "Planta_Monstera",     "Monstera",           "Sala" },
        { "Planta_Ficus",        "Ficus elástica",     "Habitación 2" },
        { "Planta_PaloBrasil",   "Palo de Brasil",     "Comedor" },
        { "Planta_Cinta",        "Cinta",              "Comedor" },
        { "Planta_Aloe",         "Aloe vera",          "Jardinera bajo la ventana de la cocina" },
        { "Planta_Menta",        "Menta",              "Jardinera bajo la ventana de la cocina" },
        { "Planta_LenguaSuegra", "Lengua de suegra",   "Sala" },
        { "Planta_Echeveria",    "Echeveria",          "Habitación 1, sobre el escritorio" },
        { "Planta_CunaMoises",   "Cuna de Moisés",     "Cocina" },
        { "Planta_Peperomia",    "Peperomia",          "Comedor, en la repisa" },
        { "Planta_Helecho",      "Helecho",            "Baño" },
        { "Planta_Bambu",        "Bambú de la suerte", "Baño" },
        { "Planta_ZZ",           "Planta ZZ",          "Recibidor" },
        { "Planta_Palmera",      "Palmera",            "Jardín, junto a la sala" },
    };

    Estado estado = CERRADA;
    bool bloqueado = false;        // main.cpp lo pone en true mientras la trivia esta en pantalla

    void Init(Muebles* m) { muebles = m; }

    bool Visible() const { return estado != CERRADA; }              // Hay popup o juego en pantalla
    bool Jugando() const { return estado == JUGANDO; }
    bool Activa() const { return estado == REGLAS || estado == NIVEL || estado == FINAL; }   // Popup que bloquea movimiento y camara (jugando se camina libre)
    bool Sosteniendo() const { return sostenida >= 0; }

    // Se llama cada cuadro. jugador = BasePosition de la camara, frente = hacia donde mira, pies = altura del piso
    // bajo el jugador, cajas = muebles y paredes, patio = posicion del checkpoint del juego.
    void Update(float dt, const glm::vec3& jugador, const glm::vec3& frente, float pies,
                const std::vector<AABB>& cajasMundo, const glm::vec3& patio, float radio) {
        cajas = &cajasMundo; centroPatio = patio; ultimoPies = pies; ultimoJugador = jugador;
        reloj += dt;
        float d = glm::length(glm::vec2(jugador.x - patio.x, jugador.z - patio.z));
        cerca = d <= radio;
        switch (estado) {
        case CERRADA:
            break;                                                  // No se abre sola: el jugador presiona E
        case REGLAS:
        case NIVEL:
            if (d > radio * 1.4f || bloqueado) estado = CERRADA;
            break;
        case JUGANDO:
            tiempo += dt;
            if (sostenida >= 0) {                                   // La planta que se carga flota frente a la mira y gira
                PlantaJ& p = set[sostenida];
                p.base = PosicionSostenida(jugador, frente);
                p.giro += glm::radians(80.0f) * dt;
            }
            AplicarMatrices();
            break;
        case FINAL: break;
        }
    }

    // Lee las teclas (con deteccion de "recien presionada"). Regresa true si consumio la E (para que main.cpp no la
    // use ademas para interactuar). Con activo = false solo actualiza el estado de las teclas.
    bool ProcesarTeclas(GLFWwindow* ventana, bool activo) {
        bool k1 = Borde(ventana, GLFW_KEY_1), k1n = Borde(ventana, GLFW_KEY_KP_1);
        bool k2 = Borde(ventana, GLFW_KEY_2), k2n = Borde(ventana, GLFW_KEY_KP_2);
        bool k3 = Borde(ventana, GLFW_KEY_3), k3n = Borde(ventana, GLFW_KEY_KP_3);
        bool enter = Borde(ventana, GLFW_KEY_ENTER), enterNum = Borde(ventana, GLFW_KEY_KP_ENTER);
        bool e = Borde(ventana, GLFW_KEY_E), r = Borde(ventana, GLFW_KEY_R);
        if (!activo) return false;
        bool confirmar = enter || enterNum;
        bool consumioE = false;
        switch (estado) {
        case CERRADA:
            if (e && cerca && !bloqueado) { estado = REGLAS; consumioE = true; }   // E cerca del pin abre las reglas
            break;
        case REGLAS:
            if (confirmar || e) { estado = NIVEL; consumioE = e; }
            break;
        case NIVEL:
            if (k1 || k1n) Comenzar(1);
            else if (k2 || k2n) Comenzar(2);
            else if (k3 || k3n) Comenzar(3);
            break;
        case JUGANDO:
            if (confirmar) Calificar();
            break;
        case FINAL:
            if (confirmar || e) { Cerrar(); consumioE = e; }
            else if (r) { set.clear(); estado = NIVEL; }                           // Jugar de nuevo: otra vez a elegir nivel
            break;
        }
        return consumioE;
    }

    // Planta del juego a la que apunta la mira (-1 = ninguna). Se llama cada cuadro mientras se juega.
    int Apuntar(const glm::vec3& ojo, const glm::vec3& frente) {
        apuntada = -1;
        if (estado != JUGANDO || sostenida >= 0) return apuntada;
        float mejor = 4.5f;                                                       // Alcance maximo para agarrar (m)
        for (size_t i = 0; i < set.size(); ++i) {
            AABB c = muebles->CajaActual(set[i].idx);
            c.minPoint -= glm::vec3(0.20f); c.maxPoint += glm::vec3(0.20f);       // Margen para apuntar mejor
            float t;
            if (RayAABB(ojo, glm::normalize(frente), c, mejor, t)) { mejor = t; apuntada = (int)i; }
        }
        return apuntada;
    }

    // Clic izquierdo: al presionarlo agarra la planta a la que apunta la mira; al soltarlo la deja.
    // Regresa true si el clic fue del juego (para que main.cpp no lo use ademas para abrir puertas).
    bool Clic(bool presionado, const glm::vec3& ojo, const glm::vec3& frente) {
        if (estado != JUGANDO) return false;
        if (presionado && sostenida < 0) {
            int k = Apuntar(ojo, frente);
            if (k < 0) return false;
            sostenida = k;
            set[k].calif = 0;                                                     // Al moverla se borra su calificacion
            std::cout << "[Plantas] Agarra: " << NombreDe(set[k].idx) << std::endl;
            return true;
        }
        if (!presionado && sostenida >= 0) {
            PlantaJ& p = set[sostenida];
            glm::vec3 meta = muebles->BaseOriginal(p.idx);
            if (glm::length(glm::vec2(p.base.x - meta.x, p.base.z - meta.z)) <= TOLERANCIA) p.base = meta;   // Encaja en su aura (a su altura original)
            else p.base.y = ultimoPies;                                           // Si no, cae al piso
            std::cout << "[Plantas] Suelta: " << NombreDe(p.idx) << " en (" << p.base.x << ", " << p.base.z << ")" << std::endl;
            sostenida = -1;
            AplicarMatrices();
            return true;
        }
        return false;
    }

    // ESC: sale del juego y todas las plantas regresan a su lugar. No se vuelve a abrir hasta alejarse y regresar.
    void Cerrar() {
        Restaurar();
        set.clear();
        sostenida = -1;
        estado = CERRADA;
        armado = false;
    }

    // Auras a dibujar este cuadro: lugares correctos (todos los niveles), marcas de "agarrable" bajo cada planta y calificaciones
    void ObtenerAuras(std::vector<AuraInfo>& out) const {
        out.clear();
        if (estado != JUGANDO) return;
        for (int i = 0; i < (int)set.size(); ++i) {
            const PlantaJ& p = set[i];
            glm::vec3 meta = muebles->BaseOriginal(p.idx);
            bool resaltada = (nivel == 1 && i == sostenida);                       // Basico: el destino de la planta que cargas
            if (p.calif != 1) {                                                    // Si ya esta en verde, la planta tapa su aura
                glm::vec4 col = resaltada ? glm::vec4(1.0f, 0.95f, 0.70f, 1.0f) : glm::vec4(0.65f, 0.80f, 1.0f, 0.75f);
                out.push_back({ meta, resaltada ? TOLERANCIA + 0.1f : TOLERANCIA, col });
            }
            if (i == sostenida) continue;
            if (p.calif == 1)      out.push_back({ p.base, 0.55f, glm::vec4(0.35f, 0.85f, 0.30f, 1.0f) });
            else if (p.calif == 2) out.push_back({ p.base, 0.55f, glm::vec4(0.90f, 0.25f, 0.22f, 1.0f) });
            else                   out.push_back({ p.base, i == apuntada ? 0.60f : 0.45f,      // Marca: esta planta se puede agarrar
                                                   i == apuntada ? glm::vec4(1.0f, 1.0f, 0.85f, 1.0f) : glm::vec4(1.0f, 0.78f, 0.25f, 1.0f) });
        }
    }

    // Dibuja los popups segun el estado. frente = hacia donde mira el jugador (para la flecha del nivel Basico).
    void Dibujar(UI& ui, const glm::vec3& jugador, const glm::vec3& frente) {
        switch (estado) {
        case CERRADA:
            if (cerca && !bloqueado) DibujarAvisoReabrir(ui);
            break;
        case REGLAS:  DibujarReglas(ui); break;
        case NIVEL:   DibujarNivel(ui); break;
        case JUGANDO: DibujarJuego(ui, jugador, frente); break;
        case FINAL:   DibujarFinal(ui); break;
        }
    }

    // Nombre bonito de la planta idx (por ejemplo "Aloe vera")
    std::string NombreDe(int idx) const {
        const Info* i = BuscarInfo(idx);
        return i ? i->nombre : muebles->plantas[idx].nombre;
    }

private:
    struct PlantaJ {
        int idx;               // Indice en muebles->plantas
        glm::vec3 base;        // Donde esta ahora el centro de su base
        float giro;            // Giro sobre su eje vertical (rad)
        int calif;             // 0 sin calificar, 1 bien colocada, 2 mal colocada
    };

    static constexpr float PW = 640.0f, MARGEN = 24.0f, PAD = 14.0f;

    Muebles* muebles = nullptr;
    const std::vector<AABB>* cajas = nullptr;
    std::vector<PlantaJ> set;                     // Las plantas de esta partida
    int apuntada = -1;                            // Planta del juego a la que apunta la mira
    int sostenida = -1;                           // Posicion en 'set' de la planta que se carga (-1 = ninguna)
    int nivel = 1;                                // 1 Basico, 2 Intermedio, 3 Experto
    int calificaciones = 0;                       // Cuantas veces se califico
    int ultimasBien = 0;
    float tiempo = 0.0f, tiempoFinal = 0.0f;
    float reloj = 0.0f;                           // Para el vaiven de la planta que flota
    float ultimoPies = 0.0f;
    glm::vec3 centroPatio{ 0.0f }, ultimoJugador{ 0.0f };
    bool cerca = false, armado = true;
    bool prevTecla[GLFW_KEY_LAST + 1] = {};
    std::mt19937 rng{ std::random_device{}() };

    bool Borde(GLFWwindow* v, int tecla) {
        bool ahora = glfwGetKey(v, tecla) == GLFW_PRESS;
        bool nueva = ahora && !prevTecla[tecla];
        prevTecla[tecla] = ahora;
        return nueva;
    }

    const Info* BuscarInfo(int idx) const {
        for (const Info& i : INFO) if (muebles->plantas[idx].nombre == i.clave) return &i;
        return nullptr;
    }

    static std::string Formato(float segundos) {
        int t = (int)segundos;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", t / 60, t % 60);
        return buf;
    }
    static const char* NombreNivel(int n) { return n == 1 ? "BÁSICO" : n == 2 ? "INTERMEDIO" : "EXPERTO"; }

    // Rayo contra caja alineada a los ejes (metodo de las "losas"). Regresa true si la toca antes de maxT.
    static bool RayAABB(const glm::vec3& o, const glm::vec3& d, const AABB& b, float maxT, float& tOut) {
        float tmin = 0.0f, tmax = maxT;
        for (int a = 0; a < 3; ++a) {
            float inv = (std::abs(d[a]) < 1e-8f) ? ((d[a] >= 0.0f) ? 1e8f : -1e8f) : 1.0f / d[a];
            float t0 = (b.minPoint[a] - o[a]) * inv, t1 = (b.maxPoint[a] - o[a]) * inv;
            if (t0 > t1) std::swap(t0, t1);
            tmin = std::max(tmin, t0); tmax = std::min(tmax, t1);
            if (tmin > tmax) return false;
        }
        tOut = tmin;
        return true;
    }

    // Donde flota la planta que se carga: 1.4 m frente al jugador (menos si hay una pared o mueble), a media altura y con vaiven
    glm::vec3 PosicionSostenida(const glm::vec3& jugador, const glm::vec3& frente) const {
        glm::vec2 f(frente.x, frente.z);
        f = (glm::length(f) > 1e-4f) ? glm::normalize(f) : glm::vec2(0.0f, -1.0f);
        float dist = 1.4f;
        if (cajas) {
            for (float s = 0.5f; s <= 1.4f; s += 0.1f) {                    // Camina el rayo hasta topar con algo
                glm::vec3 p(jugador.x + f.x * s, 0.0f, jugador.z + f.y * s);
                if (!PuntoLibre(p, 0.3f, *cajas)) { dist = std::max(0.4f, s - 0.1f); break; }
            }
        }
        float y = ultimoPies + 0.45f + 0.06f * std::sin(reloj * 2.5f);
        return glm::vec3(jugador.x + f.x * dist, y, jugador.z + f.y * dist);
    }

    // Pasa la posicion y el giro de cada planta del juego a Muebles (que las dibuja movidas)
    void AplicarMatrices() {
        for (const PlantaJ& p : set) {
            glm::vec3 origen = muebles->BaseOriginal(p.idx);
            muebles->plantaMatriz[p.idx] = glm::translate(glm::mat4(1.0f), p.base)
                                         * glm::rotate(glm::mat4(1.0f), p.giro, glm::vec3(0, 1, 0))
                                         * glm::translate(glm::mat4(1.0f), -origen);   // Gira sobre su propio eje
            muebles->plantaMovida[p.idx] = 1;
        }
    }

    // Todas las plantas del juego regresan a su lugar original
    void Restaurar() {
        for (const PlantaJ& p : set) muebles->plantaMovida[p.idx] = 0;
    }

    // Sortea 8 plantas, las teletransporta al patio alrededor del checkpoint y empieza a correr el tiempo
    void Comenzar(int nivelElegido) {
        nivel = nivelElegido;
        tiempo = 0.0f; calificaciones = 0; ultimasBien = 0; sostenida = -1;
        set.clear();
        std::vector<int> todas(muebles->plantas.size());
        std::iota(todas.begin(), todas.end(), 0);
        std::shuffle(todas.begin(), todas.end(), rng);
        int n = std::min<int>(POR_JUEGO, (int)todas.size());
        std::uniform_real_distribution<float> azar(0.0f, 6.28318f);
        for (int k = 0; k < n; ++k) {
            PlantaJ p;
            p.idx = todas[k]; p.giro = azar(rng); p.calif = 0;
            p.base = LugarEnPatio(k, n);
            set.push_back(p);
        }
        estado = JUGANDO;
        AplicarMatrices();
        std::cout << "[Plantas] Inicia el nivel " << nivel << " con " << n << " plantas:";
        for (const PlantaJ& p : set) std::cout << " " << NombreDe(p.idx);
        std::cout << std::endl;
    }

    // Lugar libre del patio para la planta numero k de n: en un anillo alrededor del checkpoint, sin chocar con muebles ni entre ellas
    glm::vec3 LugarEnPatio(int k, int n) const {
        const float paso = 6.28318f / n;
        for (float r = 2.4f; r <= 5.4f; r += 0.6f) {
            for (int extra : { 0, 1, -1, 2, -2, 3, -3 }) {
                float ang = paso * k + extra * 0.2f;
                glm::vec3 c = centroPatio + glm::vec3(std::cos(ang), 0.0f, std::sin(ang)) * r;
                if (cajas && !PuntoLibre(c, 0.5f, *cajas)) continue;
                bool libreDeOtras = true;
                for (const PlantaJ& q : set)
                    if (glm::length(glm::vec2(q.base.x - c.x, q.base.z - c.z)) < 0.9f) { libreDeOtras = false; break; }
                if (libreDeOtras) return c;
            }
        }
        float ang = paso * k;
        return centroPatio + glm::vec3(std::cos(ang), 0.0f, std::sin(ang)) * 2.4f;
    }

    // ENTER: revisa cada planta (a menos de TOLERANCIA de su lugar = verde, si no = roja). Si todas estan bien, termina.
    void Calificar() {
        if (set.empty()) return;
        int bien = 0;
        for (size_t i = 0; i < set.size(); ++i) {
            PlantaJ& p = set[i];
            if ((int)i == sostenida) { p.calif = 2; continue; }               // La que aun se carga cuenta como mal
            glm::vec3 meta = muebles->BaseOriginal(p.idx);
            float d = glm::length(glm::vec2(p.base.x - meta.x, p.base.z - meta.z));
            p.calif = (d <= TOLERANCIA) ? 1 : 2;
            if (p.calif == 1) ++bien;
        }
        ++calificaciones;
        ultimasBien = bien;
        std::cout << "[Plantas] Calificacion " << calificaciones << ": " << bien << "/" << set.size() << " bien" << std::endl;
        if (bien == (int)set.size()) {
            tiempoFinal = tiempo;
            Restaurar();                                                      // Quedan exactamente en su lugar original
            estado = FINAL;
            std::cout << "[Plantas] Fin en " << Formato(tiempoFinal) << std::endl;
        }
    }

    // ------------------------------------------------------------------ dibujo

    void DibujarAvisoReabrir(UI& ui) {
        float x = ui.W - PW - MARGEN, alto = 21.0f + 2 * PAD, y = 24.0f;
        ui.Panel(x, y, PW, alto);
        ui.TextoCentrado("PRESIONA E PARA JUGAR: ACOMODA LAS PLANTAS", x, PW, y + PAD, 3.0f, UI::Verde());
    }

    void DibujarReglas(UI& ui) {
        float x = ui.W - PW - MARGEN, ancho = PW - 2 * PAD - 8.0f, xt = x + PAD + 4;
        auto contenido = [&](float y, bool dibujar) {
            if (dibujar) ui.TextoCentrado("ACOMODA LAS PLANTAS", x, PW, y, 4.0f, UI::Verde());
            y += 28.0f + 20.0f;
            y = ui.TextoEnvuelto("LAS PLANTAS SE FUERON AL PATIO. LLÉVALAS DE VUELTA A SU LUGAR EN LA CASA.", xt, y, ancho, 3.0f, UI::Crema(), dibujar) + 6.0f;
            y = ui.TextoEnvuelto("MANTÉN EL CLIC IZQUIERDO SOBRE UNA PLANTA CON CÍRCULO AMARILLO PARA AGARRARLA. SUÉLTALA EN SU CÍRCULO.", xt, y, ancho, 3.0f, UI::Crema(), dibujar) + 6.0f;
            y = ui.TextoEnvuelto("ENTER CALIFICA: VERDE ES BIEN Y ROJO ES MAL. TERMINAS CUANDO TODAS ESTÉN EN VERDE.", xt, y, ancho, 3.0f, UI::Crema(), dibujar) + 12.0f;
            y = ui.TextoEnvuelto("PRESIONA ENTER O E PARA CONTINUAR", xt, y, ancho, 3.0f, UI::Verde(), dibujar);
            return y;
        };
        float alto = contenido(0.0f, false) + 2 * PAD - 8.0f;
        float y = (ui.H - alto) * 0.5f;
        ui.Panel(x, y, PW, alto);
        contenido(y + PAD, true);
    }

    void DibujarNivel(UI& ui) {
        float x = ui.W - PW - MARGEN, ancho = PW - 2 * PAD - 8.0f, xt = x + PAD + 4;
        auto contenido = [&](float y, bool dibujar) {
            if (dibujar) ui.TextoCentrado("ELIGE EL NIVEL", x, PW, y, 4.0f, UI::Verde());
            y += 28.0f + 20.0f;
            y = ui.TextoEnvuelto("1  BÁSICO: SE RESALTA EL CÍRCULO DE LA PLANTA, MÁS ZONA Y DISTANCIA", xt, y, ancho, 3.0f, UI::Crema(), dibujar) + 8.0f;
            y = ui.TextoEnvuelto("2  INTERMEDIO: SOLO EL NOMBRE DE LA ZONA", xt, y, ancho, 3.0f, UI::Crema(), dibujar) + 8.0f;
            y = ui.TextoEnvuelto("3  EXPERTO: SIN PISTAS", xt, y, ancho, 3.0f, UI::Crema(), dibujar) + 12.0f;
            y = ui.TextoEnvuelto("PRESIONA 1, 2 O 3 PARA COMENZAR. ESC PARA SALIR", xt, y, ancho, 3.0f, UI::Verde(), dibujar);
            return y;
        };
        float alto = contenido(0.0f, false) + 2 * PAD - 8.0f;
        float y = (ui.H - alto) * 0.5f;
        ui.Panel(x, y, PW, alto);
        contenido(y + PAD, true);
    }

    // Panel del juego (arriba a la derecha) y, si se carga una planta, el cuadro de pistas debajo
    void DibujarJuego(UI& ui, const glm::vec3& jugador, const glm::vec3& frente) {
        const float pw = 420.0f;
        float x = ui.W - pw - MARGEN, y = MARGEN, ancho = pw - 2 * PAD;
        auto hud = [&](float cy, bool dibujar) {
            if (dibujar) ui.TextoCentrado("ACOMODA LAS PLANTAS", x, pw, cy, 3.0f, UI::Verde());
            cy += 21.0f + 10.0f;
            if (dibujar) {
                ui.Texto(std::string("NIVEL: ") + NombreNivel(nivel), x + PAD, cy, 2.0f, UI::Crema());
                std::string t = "TIEMPO: " + Formato(tiempo);
                ui.Texto(t, x + pw - PAD - ui.AnchoTexto(t, 2.0f), cy, 2.0f, UI::Verde());
            }
            cy += 14.0f + 8.0f;
            if (dibujar) {
                char buf[64];
                if (calificaciones == 0) snprintf(buf, sizeof(buf), "AÚN SIN CALIFICAR");
                else snprintf(buf, sizeof(buf), "BIEN COLOCADAS: %d DE %d", ultimasBien, (int)set.size());
                ui.Texto(buf, x + PAD, cy, 2.0f, calificaciones == 0 ? UI::Crema() : UI::Verde());
            }
            cy += 14.0f + 10.0f;
            for (int i = 0; i < (int)set.size(); ++i) {                       // Una linea por planta con su estado
                if (dibujar) {
                    const PlantaJ& p = set[i];
                    glm::vec4 col = (p.calif == 1) ? UI::Verde() : (p.calif == 2) ? UI::Rojo() : glm::vec4(0.60f, 0.58f, 0.55f, 1.0f);
                    ui.Rect(x + PAD + 2, cy + 1.0f, 12.0f, 12.0f, col);
                    std::string nombre = NombreDe(p.idx) + (i == sostenida ? "  <" : "");
                    ui.Texto(nombre, x + PAD + 24, cy, 2.0f, i == sostenida ? UI::Verde() : UI::Crema());
                }
                cy += 18.0f;
            }
            cy += 6.0f;
            cy = ui.TextoEnvuelto("MANTÉN CLIC IZQ.: AGARRAR", x + PAD, cy, ancho, 2.0f, UI::Crema(), dibujar);
            cy = ui.TextoEnvuelto("ENTER: CALIFICAR    ESC: SALIR", x + PAD, cy, ancho, 2.0f, UI::Crema(), dibujar);
            return cy;
        };
        float alto = hud(0.0f, false) + 2 * PAD - 10.0f;
        ui.Panel(x, y, pw, alto);
        hud(y + PAD, true);

        if (sostenida >= 0) DibujarPistas(ui, jugador, frente, x, y + alto + 14.0f, pw);
        else if (apuntada >= 0) {                                                // Aviso abajo al centro: esta planta se puede agarrar
            std::string t = "MANTÉN EL CLIC PARA AGARRAR: " + NombreDe(set[apuntada].idx);
            float w = ui.AnchoTexto(t, 3.0f) + 2 * PAD;
            float bx = (ui.W - w) * 0.5f, by = ui.H - 90.0f;
            ui.Panel(bx, by, w, 21.0f + 2 * PAD);
            ui.Texto(t, bx + PAD, by + PAD, 3.0f, UI::Verde());
        }
    }

    // Cuadro con el nombre de la planta que se carga y las pistas que da el nivel
    void DibujarPistas(UI& ui, const glm::vec3& jugador, const glm::vec3& frente, float x, float y, float pw) {
        const PlantaJ& p = set[sostenida];
        const Info* info = BuscarInfo(p.idx);
        float ancho = pw - 2 * PAD;
        std::string zona = info ? info->zona : "";
        char dist[48] = "", dir[48] = "";
        if (nivel == 1) {                                                     // Basico: distancia y direccion al lugar de la planta
            glm::vec3 meta = muebles->BaseOriginal(p.idx);
            glm::vec2 hacia(meta.x - jugador.x, meta.z - jugador.z);
            snprintf(dist, sizeof(dist), "A %.1f METROS", glm::length(hacia));
            glm::vec2 f(frente.x, frente.z);
            f = (glm::length(f) > 1e-4f) ? glm::normalize(f) : glm::vec2(0.0f, -1.0f);
            glm::vec2 der(-f.y, f.x);                                         // Derecha de la camara en el plano
            float angulo = glm::degrees(std::atan2(glm::dot(hacia, der), glm::dot(hacia, f)));   // 0 = de frente, + = a la derecha
            if (std::abs(angulo) < 30.0f) snprintf(dir, sizeof(dir), "ADELANTE");
            else if (std::abs(angulo) > 150.0f) snprintf(dir, sizeof(dir), "ATRÁS");
            else if (angulo > 0.0f) snprintf(dir, sizeof(dir), "DERECHA >");
            else snprintf(dir, sizeof(dir), "< IZQUIERDA");
        }
        auto cuadro = [&](float cy, bool dibujar) {
            cy = ui.TextoEnvuelto("LLEVAS: " + NombreDe(p.idx), x + PAD, cy, ancho, 3.0f, UI::Verde(), dibujar) + 4.0f;
            if (nivel == 3) {
                cy = ui.TextoEnvuelto("SIN PISTAS: BUSCA EL CÍRCULO AZUL QUE LE TOCA", x + PAD, cy, ancho, 2.0f, UI::Crema(), dibujar);
            } else {
                cy = ui.TextoEnvuelto("ZONA: " + zona, x + PAD, cy, ancho, 2.0f, UI::Crema(), dibujar);
                if (nivel == 1) {
                    cy += 8.0f;
                    if (dibujar) ui.Texto(dist, x + PAD, cy, 3.0f, UI::Verde());
                    cy += 21.0f + 10.0f;
                    if (dibujar) ui.Texto(dir, x + PAD, cy, 3.0f, UI::Verde());
                    cy += 21.0f + 10.0f;
                    cy = ui.TextoEnvuelto("SUÉLTALA EN EL CÍRCULO CREMA", x + PAD, cy, ancho, 2.0f, UI::Crema(), dibujar);
                }
            }
            return cy;
        };
        float alto = cuadro(0.0f, false) + 2 * PAD - 6.0f;
        ui.Panel(x, y, pw, alto);
        cuadro(y + PAD, true);
    }

    void DibujarFinal(UI& ui) {
        float x = ui.W - PW - MARGEN;
        float alto = PAD + 28.0f + 18.0f + 28.0f + 10.0f + 28.0f + 10.0f + 28.0f + 18.0f + 21.0f + 14.0f + 14.0f + PAD;
        float y = (ui.H - alto) * 0.5f;
        ui.Panel(x, y, PW, alto);
        float cy = y + PAD;
        ui.TextoCentrado("¡TODAS EN SU LUGAR!", x, PW, cy, 4.0f, UI::Verde());
        cy += 28.0f + 18.0f;
        ui.TextoCentrado(std::string("NIVEL: ") + NombreNivel(nivel), x, PW, cy, 4.0f, UI::Crema());
        cy += 28.0f + 10.0f;
        ui.TextoCentrado("TIEMPO: " + Formato(tiempoFinal), x, PW, cy, 4.0f, UI::Crema());
        cy += 28.0f + 10.0f;
        char buf[48];
        snprintf(buf, sizeof(buf), "CALIFICACIONES: %d", calificaciones);
        ui.TextoCentrado(buf, x, PW, cy, 4.0f, UI::Crema());
        cy += 28.0f + 18.0f;
        ui.TextoCentrado(calificaciones == 1 ? "¡A LA PRIMERA! ERES EXPERTO" : "¡BIEN HECHO! SIGUE PRACTICANDO", x, PW, cy, 3.0f, UI::Verde());
        cy += 21.0f + 14.0f;
        ui.TextoCentrado("ENTER: CERRAR     R: JUGAR DE NUEVO", x, PW, cy, 2.0f, UI::Crema());
    }
};
