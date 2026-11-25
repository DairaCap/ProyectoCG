#pragma once
// ============================================================================
// Sonido.h - Audio del recorrido con irrKlang.
//   - Ambiente de la casa (adentro) y de bosque (afuera): los dos suenan en bucle y se mezclan
//     poco a poco segun la distancia a las paredes de la casa, asi al cruzar la puerta uno sube y el otro baja.
//   - Pasos: suenan solo mientras el jugador camina sobre el suelo.
// Los archivos estan en la carpeta sonidos/. Para cambiar de archivo o de volumen, ver las constantes de abajo.
// ============================================================================
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <irrKlang.h>

class Sonido {
public:
    // Archivos dentro de la carpeta sonidos/
    static constexpr const char* ARCHIVO_CASA    = "Stillness.mp3";             // Ambiente de adentro
    static constexpr const char* ARCHIVO_BOSQUE  = "naturaleza_bosque.mp3";     // Ambiente de afuera
    static constexpr const char* ARCHIVO_PASOS   = "Footsteps.mp3";

    // Volumenes (0 a 1)
    static constexpr float VOL_CASA   = 0.50f;
    static constexpr float VOL_BOSQUE = 0.70f;
    static constexpr float VOL_PASOS  = 0.60f;

    // Rectangulo de la casa en el plano (x, z) del juego, medido del modelo. Se usa para saber que tan adentro esta el jugador.
    static constexpr float CASA_MIN_X = -9.25f, CASA_MAX_X = 7.29f;
    static constexpr float CASA_MIN_Z = -4.74f, CASA_MAX_Z = 5.78f;
    static constexpr float ZONA_TRANSICION = 1.5f;   // m a cada lado de la pared donde los dos ambientes se mezclan

    ~Sonido() {
        if (casa) casa->drop();
        if (bosque) bosque->drop();
        if (pasos) pasos->drop();
        if (motor) motor->drop();
    }

    // Busca la carpeta sonidos (junto al ejecutable o un nivel arriba) y arranca los ambientes. Regresa false si no hay audio.
    bool Init() {
        std::string carpeta = "sonidos";
        { std::ifstream probar(std::string("sonidos/") + ARCHIVO_BOSQUE, std::ios::binary); if (!probar) carpeta = "../sonidos"; }
        motor = irrklang::createIrrKlangDevice();
        if (!motor) { std::cout << "[Sonido] No se pudo iniciar irrKlang (sin dispositivo de audio)" << std::endl; return false; }
        // play2D(archivo, bucle, empezarEnPausa, rastrear): rastrear = true para poder cambiar el volumen despues
        casa   = motor->play2D((carpeta + "/" + ARCHIVO_CASA).c_str(), true, false, true);
        bosque = motor->play2D((carpeta + "/" + ARCHIVO_BOSQUE).c_str(), true, false, true);
        pasos  = motor->play2D((carpeta + "/" + ARCHIVO_PASOS).c_str(), true, true, true);     // Empieza en pausa
        if (casa) casa->setVolume(0.0f);
        if (bosque) bosque->setVolume(0.0f);
        if (pasos) pasos->setVolume(0.0f);
        std::cout << "[Sonido] Carpeta " << carpeta << " | casa:" << (casa != nullptr) << " bosque:" << (bosque != nullptr)
                  << " pasos:" << (pasos != nullptr) << std::endl;
        return true;
    }

    // Que tan "dentro" de la casa esta el jugador: 0 = afuera, 1 = adentro. Cambia suave en la franja de las paredes.
    static float Interior(const glm::vec3& p) {
        float profundidad = std::min(std::min(p.x - CASA_MIN_X, CASA_MAX_X - p.x), std::min(p.z - CASA_MIN_Z, CASA_MAX_Z - p.z));
        float t = (profundidad + ZONA_TRANSICION) / (2.0f * ZONA_TRANSICION);       // -1.5 m -> 0 ; +1.5 m -> 1
        t = std::min(std::max(t, 0.0f), 1.0f);
        return t * t * (3.0f - 2.0f * t);                                            // smoothstep: sin cortes bruscos
    }

    // Se llama cada cuadro. caminando = el jugador avanza sobre el suelo (no en pantalla de inicio ni con popups).
    void Update(float dt, const glm::vec3& jugador, bool caminando) {
        if (!motor) return;
        float dentro = Interior(jugador);
        // Aun dentro se oye un poco del bosque (por puertas y ventanas); afuera no se oye la casa
        float volCasa = VOL_CASA * dentro;
        float volBosque = VOL_BOSQUE * (0.15f + 0.85f * (1.0f - dentro));
        if (casa) casa->setVolume(volCasa);
        if (bosque) bosque->setVolume(volBosque);

        // Pasos: el volumen sube y baja rapido para que no corten de golpe
        float objetivo = caminando ? 1.0f : 0.0f;
        pasosNivel += (objetivo - pasosNivel) * std::min(dt * 10.0f, 1.0f);
        if (pasos) {
            if (caminando && pasos->getIsPaused()) pasos->setIsPaused(false);
            if (!caminando && pasosNivel < 0.02f && !pasos->getIsPaused()) pasos->setIsPaused(true);
            pasos->setVolume(VOL_PASOS * pasosNivel);
        }
    }

private:
    irrklang::ISoundEngine* motor = nullptr;
    irrklang::ISound* casa = nullptr;
    irrklang::ISound* bosque = nullptr;
    irrklang::ISound* pasos = nullptr;
    float pasosNivel = 0.0f;       // 0 a 1, para el fundido de los pasos
};
