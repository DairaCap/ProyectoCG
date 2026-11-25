#pragma once
// ============================================================================
// Trivia.h - Minijuego de trivia botanica (popup a la derecha de la pantalla).
//
// Flujo:
//   CERRADA  -> el jugador se acerca al checkpoint -> INTRO (reglas)
//   INTRO    -> ENTER o E -> PREGUNTA
//   PREGUNTA -> imagen de la pregunta; el jugador contesta con A, B o C -> RESPUESTA
//   RESPUESTA-> "Correcto" / "Incorrecto" + imagen con la respuesta; ENTER -> siguiente pregunta
//   FINAL    -> despues de 5 preguntas: marcador y tiempo; ENTER cierra, R juega de nuevo
//
// Se sortean 5 de las 10 preguntas. El cronometro solo corre mientras se ve la
// pregunta (leer la respuesta no cuenta). Las imagenes estan en minijuegos/trivia/:
//   qNN_pregunta.png (con las opciones a, b, c)   y   qNN_respuesta.png (con la opcion correcta)
// Las imagenes se cargan solo cuando se necesitan y se liberan despues.
// ============================================================================
#include <string>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include <cstdio>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "UI.h"

class Trivia {
public:
    enum Estado { CERRADA, INTRO, PREGUNTA, RESPUESTA, FINAL };

    static constexpr int TOTAL = 10;               // Preguntas disponibles
    static constexpr int POR_JUEGO = 5;            // Preguntas que se sortean en cada partida

    // Letra correcta de cada pregunta (sacada de las imagenes de respuesta, en el orden q01..q10)
    static constexpr char CORRECTA[TOTAL] = { 'C', 'B', 'B', 'C', 'A', 'A', 'B', 'B', 'B', 'A' };

    // Tema de cada pregunta (para la lista del marcador final)
    static inline const char* const TEMA[TOTAL] = {
        "Riego del cactus", "Exceso de riego", "Fotosíntesis y luz", "El gas de la fotosíntesis",
        "Drenaje de la maceta", "Cuándo regar el jardín", "Plantas que almacenan agua",
        "Cuándo cambiar de maceta", "Luz para plantas de interior", "Señales de poca luz"
    };

    Estado estado = CERRADA;

    // carpetaRecursos = carpeta "minijuegos" ya resuelta por main.cpp
    void Init(const std::string& carpetaRecursos) { baseDir = carpetaRecursos + "/trivia"; }

    bool Visible() const { return estado != CERRADA; }                       // Hay popup en pantalla
    bool Activa() const { return estado >= PREGUNTA; }                       // Se esta jugando: se bloquea el movimiento

    // Se llama cada cuadro con la posicion del jugador y la del checkpoint de la trivia
    void Update(float dt, const glm::vec3& jugador, const glm::vec3& checkpoint, float radio) {
        float d = glm::length(glm::vec2(jugador.x - checkpoint.x, jugador.z - checkpoint.z));
        cerca = d <= radio;
        switch (estado) {
        case CERRADA:
            if (cerca && armado) estado = INTRO;                            // Se acerca: se abre la pantalla de inicio
            if (d > radio * 1.3f) armado = true;                            // Ya se alejo: puede volver a abrirse
            break;
        case INTRO:
            if (d > radio * 1.4f) { estado = CERRADA; armado = true; }      // Se alejo sin jugar
            break;
        case PREGUNTA:
            tiempo += dt;                                                   // El cronometro corre mientras se contesta
            break;
        default: break;
        }
    }

    // Lee las teclas (con deteccion de "recien presionada"). Regresa true si consumio la tecla E,
    // para que main.cpp no la use ademas para interactuar con lo que apunta la mira.
    // Con activo = false solo actualiza el estado de las teclas (por ejemplo en la pantalla de inicio).
    bool ProcesarTeclas(GLFWwindow* ventana, bool activo) {
        bool a = Borde(ventana, GLFW_KEY_A), b = Borde(ventana, GLFW_KEY_B), c = Borde(ventana, GLFW_KEY_C);
        bool enter = Borde(ventana, GLFW_KEY_ENTER), enterNum = Borde(ventana, GLFW_KEY_KP_ENTER);
        bool e = Borde(ventana, GLFW_KEY_E), r = Borde(ventana, GLFW_KEY_R);
        if (!activo) return false;
        bool confirmar = enter || enterNum || e;
        bool consumioE = false;
        switch (estado) {
        case CERRADA:
            if (e && cerca) { estado = INTRO; consumioE = true; }           // Reabrir la trivia con E cerca del pin
            break;
        case INTRO:
            if (confirmar) { Comenzar(); consumioE = e; }
            break;
        case PREGUNTA:                                                      // A, B y C valen como respuestas (no mueven al jugador)
            if (a) Responder('A'); else if (b) Responder('B'); else if (c) Responder('C');
            break;
        case RESPUESTA:
            if (confirmar) { Siguiente(); consumioE = e; }
            break;
        case FINAL:
            if (confirmar) { Cerrar(); consumioE = e; }
            else if (r) Comenzar();                                         // Jugar de nuevo con otras 5 preguntas
            break;
        }
        return consumioE;
    }

    // Cierra el popup (ESC) y no lo vuelve a abrir hasta que el jugador se aleje y regrese
    void Cerrar() {
        LiberarImagenes();
        estado = CERRADA;
        armado = false;
    }

    // Dibuja el popup (o el aviso de "presiona E") segun el estado
    void Dibujar(UI& ui) {
        switch (estado) {
        case CERRADA:
            if (cerca && !armado) DibujarAvisoReabrir(ui);
            break;
        case INTRO:     DibujarIntro(ui); break;
        case PREGUNTA:  DibujarPregunta(ui, false); break;
        case RESPUESTA: DibujarPregunta(ui, true); break;
        case FINAL:     DibujarFinal(ui); break;
        }
    }

private:
    // --- Constantes de diseno del popup (en pixeles de la ventana de 1280 x 720) ---
    static constexpr float PW = 640.0f;            // Ancho del panel (la mitad de la ventana: asi se leen las opciones de las imagenes)
    static constexpr float MARGEN = 24.0f;         // Distancia al borde derecho de la pantalla
    static constexpr float PAD = 14.0f;            // Margen interior

    std::string baseDir;
    std::mt19937 rng{ std::random_device{}() };    // Sorteo distinto en cada partida
    std::vector<int> orden;                        // Indices (0 a 9) de las 5 preguntas sorteadas
    std::vector<char> elegidas;                    // Letra que contesto el jugador en cada una
    std::vector<bool> acierto;                     // Si acerto en cada una
    int actual = 0;                                // Pregunta en curso (0 a 4)
    int aciertos = 0;
    float tiempo = 0.0f;                           // Segundos acumulados contestando
    GLuint texPregunta = 0, texRespuesta = 0;      // Imagenes de la pregunta en curso
    bool cerca = false;                            // Jugador a menos de radioInteraccion del pin
    bool armado = true;                            // Puede abrirse sola al acercarse
    bool prevTecla[GLFW_KEY_LAST + 1] = {};

    // true solo en el cuadro en que la tecla pasa de suelta a presionada
    bool Borde(GLFWwindow* v, int tecla) {
        bool ahora = glfwGetKey(v, tecla) == GLFW_PRESS;
        bool nueva = ahora && !prevTecla[tecla];
        prevTecla[tecla] = ahora;
        return nueva;
    }

    std::string Ruta(int pregunta, const char* tipo) const {
        char nombre[64];
        snprintf(nombre, sizeof(nombre), "/q%02d_%s.png", pregunta + 1, tipo);
        return baseDir + nombre;
    }

    void LiberarImagenes() {
        if (texPregunta) { glDeleteTextures(1, &texPregunta); texPregunta = 0; }
        if (texRespuesta) { glDeleteTextures(1, &texRespuesta); texRespuesta = 0; }
    }

    // Sortea 5 de las 10 preguntas y empieza la partida
    void Comenzar() {
        LiberarImagenes();
        std::vector<int> todas(TOTAL);
        std::iota(todas.begin(), todas.end(), 0);
        std::shuffle(todas.begin(), todas.end(), rng);
        orden.assign(todas.begin(), todas.begin() + POR_JUEGO);
        elegidas.assign(POR_JUEGO, '-');
        acierto.assign(POR_JUEGO, false);
        actual = 0; aciertos = 0; tiempo = 0.0f;
        texPregunta = UI::CargarTextura(Ruta(orden[0], "pregunta"));
        estado = PREGUNTA;
        std::cout << "[Trivia] Inicia la partida" << std::endl;
    }

    void Responder(char letra) {
        int q = orden[actual];
        bool ok = (letra == CORRECTA[q]);
        elegidas[actual] = letra;
        acierto[actual] = ok;
        if (ok) ++aciertos;
        LiberarImagenes();
        texRespuesta = UI::CargarTextura(Ruta(q, "respuesta"));
        estado = RESPUESTA;
        std::cout << "[Trivia] Pregunta " << (actual + 1) << " (q" << (q + 1) << "): contesto " << letra
                  << (ok ? " - CORRECTO" : " - INCORRECTO") << std::endl;
    }

    void Siguiente() {
        LiberarImagenes();
        ++actual;
        if (actual >= POR_JUEGO) {
            estado = FINAL;
            std::cout << "[Trivia] Fin: " << aciertos << "/" << POR_JUEGO << " en " << Formato(tiempo) << std::endl;
        } else {
            texPregunta = UI::CargarTextura(Ruta(orden[actual], "pregunta"));
            estado = PREGUNTA;
        }
    }

    static std::string Formato(float segundos) {
        int t = (int)segundos;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", t / 60, t % 60);
        return buf;
    }

    // ------------------------------------------------------------------ dibujo

    // Pantalla de inicio de la trivia: reglas y como empezar
    void DibujarIntro(UI& ui) {
        float x = ui.W - PW - MARGEN, ancho = PW - 2 * PAD - 8.0f;
        // El mismo recorrido sirve para medir (dibujar = false) y para dibujar
        auto contenido = [&](float y, bool dibujar) {
            if (dibujar) ui.TextoCentrado("TRIVIA BOTÁNICA", x, PW, y, 4.0f, UI::Verde());
            y += 28.0f + 22.0f;
            y = ui.TextoEnvuelto("RESPONDE 5 PREGUNTAS AL AZAR SOBRE EL CUIDADO DE LAS PLANTAS.", x + PAD + 4, y, ancho, 3.0f, UI::Crema(), dibujar) + 8.0f;
            y = ui.TextoEnvuelto("CONTESTA CON A, B O C.", x + PAD + 4, y, ancho, 3.0f, UI::Crema(), dibujar) + 8.0f;
            y = ui.TextoEnvuelto("TU TIEMPO SOLO CORRE MIENTRAS RESPONDES.", x + PAD + 4, y, ancho, 3.0f, UI::Crema(), dibujar) + 14.0f;
            y = ui.TextoEnvuelto("PRESIONA ENTER O E PARA COMENZAR", x + PAD + 4, y, ancho, 3.0f, UI::Verde(), dibujar);
            return y;
        };
        float alto = contenido(0.0f, false) + 2 * PAD;
        float y = (ui.H - alto) * 0.5f;
        ui.Panel(x, y, PW, alto);
        contenido(y + PAD, true);
    }

    // Pregunta (conRespuesta = false) o su resultado con la imagen de la respuesta (true)
    void DibujarPregunta(UI& ui, bool conRespuesta) {
        float x = ui.W - PW - MARGEN;
        float anchoImg = PW - 2 * PAD, altoImg = anchoImg * 9.0f / 16.0f;          // Las imagenes son 16:9
        float altoPanel = PAD + 21.0f + 14.0f + altoImg + 12.0f + 21.0f + 6.0f + 14.0f + PAD;
        if (conRespuesta) altoPanel += 35.0f + 14.0f;                              // Renglon extra con el letrero
        float y = (ui.H - altoPanel) * 0.5f;
        ui.Panel(x, y, PW, altoPanel);
        float cy = y + PAD;

        // Encabezado: numero de pregunta a la izquierda y cronometro a la derecha
        char buf[48];
        snprintf(buf, sizeof(buf), "PREGUNTA %d DE %d", actual + 1, POR_JUEGO);
        ui.Texto(buf, x + PAD, cy, 3.0f, UI::Crema());
        std::string t = Formato(tiempo);
        ui.Texto(t, x + PW - PAD - ui.AnchoTexto(t, 3.0f), cy, 3.0f, UI::Verde());
        cy += 21.0f + 14.0f;

        if (conRespuesta) {                                                        // Letrero Correcto / Incorrecto
            bool ok = acierto[actual];
            ui.TextoCentrado(ok ? "¡CORRECTO!" : "INCORRECTO", x, PW, cy, 5.0f, ok ? UI::Verde() : UI::Rojo());
            cy += 35.0f + 14.0f;
        }

        GLuint tex = conRespuesta ? texRespuesta : texPregunta;
        if (tex) ui.Imagen(tex, x + PAD, cy, anchoImg, altoImg);
        else {                                                                     // No se encontro la imagen
            ui.Rect(x + PAD, cy, anchoImg, altoImg, glm::vec4(0.25f, 0.20f, 0.15f, 1.0f));
            ui.TextoCentrado("NO SE ENCONTRÓ LA IMAGEN", x, PW, cy + altoImg * 0.5f, 3.0f, UI::Rojo());
        }
        cy += altoImg + 12.0f;

        if (!conRespuesta) {
            ui.TextoCentrado("PRESIONA A, B O C", x, PW, cy, 3.0f, UI::Verde());
            ui.TextoCentrado("ESC: SALIR DE LA TRIVIA", x, PW, cy + 21.0f + 6.0f, 2.0f, UI::Crema());
        } else {
            snprintf(buf, sizeof(buf), "TU RESPUESTA: %c", elegidas[actual]);
            ui.TextoCentrado(buf, x, PW, cy, 3.0f, UI::Crema());
            ui.TextoCentrado(actual + 1 >= POR_JUEGO ? "ENTER: VER MI MARCADOR" : "ENTER: SIGUIENTE PREGUNTA",
                             x, PW, cy + 21.0f + 6.0f, 2.0f, UI::Verde());
        }
    }

    // Marcador final: aciertos, tiempo y resultado de cada pregunta
    void DibujarFinal(UI& ui) {
        float x = ui.W - PW - MARGEN;
        float altoPanel = PAD + 28.0f + 16.0f + 28.0f + 10.0f + 28.0f + 16.0f + POR_JUEGO * 24.0f + 12.0f + 21.0f + 14.0f + 14.0f + PAD;
        float y = (ui.H - altoPanel) * 0.5f;
        ui.Panel(x, y, PW, altoPanel);
        float cy = y + PAD;

        ui.TextoCentrado("¡TRIVIA TERMINADA!", x, PW, cy, 4.0f, UI::Verde());
        cy += 28.0f + 16.0f;
        char buf[64];
        snprintf(buf, sizeof(buf), "MARCADOR: %d DE %d", aciertos, POR_JUEGO);
        ui.TextoCentrado(buf, x, PW, cy, 4.0f, UI::Crema());
        cy += 28.0f + 10.0f;
        ui.TextoCentrado("TIEMPO: " + Formato(tiempo), x, PW, cy, 4.0f, UI::Crema());
        cy += 28.0f + 16.0f;

        for (int i = 0; i < POR_JUEGO; ++i) {                                      // Una linea por pregunta
            bool ok = acierto[i];
            ui.Rect(x + 40.0f, cy + 1.0f, 16.0f, 16.0f, ok ? UI::Verde() : UI::Rojo());
            snprintf(buf, sizeof(buf), "%d. ", i + 1);
            ui.Texto(std::string(buf) + TEMA[orden[i]], x + 68.0f, cy + 2.0f, 2.0f, UI::Crema());
            cy += 24.0f;
        }
        cy += 12.0f;

        const char* mensaje = (aciertos == POR_JUEGO) ? "¡PERFECTO! ERES UN EXPERTO"
                            : (aciertos >= 4) ? "¡MUY BIEN! CASI PERFECTO"
                            : (aciertos >= 3) ? "¡BIEN! SIGUE APRENDIENDO"
                            : "¡ÁNIMO! INTÉNTALO DE NUEVO";
        ui.TextoCentrado(mensaje, x, PW, cy, 3.0f, UI::Verde());
        cy += 21.0f + 14.0f;
        ui.TextoCentrado("ENTER: CERRAR     R: JUGAR DE NUEVO", x, PW, cy, 2.0f, UI::Crema());
    }

    // Aviso pequeño cuando el jugador sigue cerca del pin pero ya cerro la trivia
    void DibujarAvisoReabrir(UI& ui) {
        float x = ui.W - PW - MARGEN, alto = 21.0f + 2 * PAD, y = 150.0f;
        ui.Panel(x, y, PW, alto);
        ui.TextoCentrado("PRESIONA E PARA JUGAR LA TRIVIA", x, PW, y + PAD, 3.0f, UI::Verde());
    }
};
