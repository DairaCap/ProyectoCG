#ifndef BOTANICAL_SYSTEM_H
#define BOTANICAL_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include "AABB.h"
#include "shader_m.h"
#include "material.h"

// Tipos de morfología botánica
enum PlantMorphology {
    PLANT_BUSH,       // Arbusto tupido (Monstera, Menta, Peperomia, Cuna de Moisés)
    PLANT_TREE,       // Árbol leñoso con tronco (Ficus, Palo de Brasil, Palmera)
    PLANT_SUCCULENT,  // Hojas carnosas puntiagudas (Aloe, Sansevieria, Echeveria)
    PLANT_FERN,       // Frondas arqueadas / Bambú (Helecho, Bambú)
    PLANT_FLOWER,     // Flores y aromáticas (Rosal, Lavanda)
    PLANT_CACTUS      // Cactus globular con espinas
};

struct BotanicalCareCard {
    std::string commonName;
    std::string scientificName;
    std::string roomLocation;
    std::string lightNeed;
    std::string waterNeed;
    std::string healthBenefit;
    std::string mentalBenefit;
};

struct PlantInstance {
    int id;
    BotanicalCareCard info;
    glm::vec3 position;
    float potScale;
    PlantMorphology morphology;
    glm::vec4 leafColor;
    glm::vec4 accentColor; // Flores o espinas
    bool isWatered;
    AABB boundingBox;

    PlantInstance(int _id, BotanicalCareCard _info, glm::vec3 _pos, float _scale,
                  PlantMorphology _morph, glm::vec4 _leafCol, glm::vec4 _accentCol = glm::vec4(0.0f))
        : id(_id), info(_info), position(_pos), potScale(_scale),
          morphology(_morph), leafColor(_leafCol), accentColor(_accentCol), isWatered(false)
    {
        glm::vec3 halfSize(0.35f * _scale, 0.65f * _scale, 0.35f * _scale);
        boundingBox = AABB(_pos - glm::vec3(halfSize.x, 0.0f, halfSize.z),
                           _pos + glm::vec3(halfSize.x, halfSize.y * 2.0f, halfSize.z));
    }
};

class BotanicalSystem {
public:
    std::vector<PlantInstance> plants;
    
    // Regadera interactiva
    glm::vec3 canPosition;
    bool playerHasCan;
    AABB canAABB;

    // Buffers de primitivas 3D compartidas
    unsigned int cylinderVAO = 0, cylinderVBO = 0, cylinderIndexCount = 0;
    unsigned int sphereVAO = 0, sphereVBO = 0, sphereIndexCount = 0;
    unsigned int coneVAO = 0, coneVBO = 0, coneIndexCount = 0;

    BotanicalSystem() : playerHasCan(false) {
        canPosition = glm::vec3(-3.2f, 0.45f, -6.2f); // En la banca del jardín exterior
        canAABB = AABB(canPosition - glm::vec3(0.25f, 0.05f, 0.25f), canPosition + glm::vec3(0.25f, 0.45f, 0.25f));
    }

    void Init() {
        initGeometry();
        initPlantsCatalog();
        std::cout << "[BotanicalSystem] Inicializado catalogo de " << plants.size() 
                  << " plantas interactivas segun plano del Anteproyecto." << std::endl;
    }

    void initPlantsCatalog() {
        plants.clear();

        // 1. SALA (Zona Frontal)
        plants.push_back(PlantInstance(1, {
            "Monstera Deliciosa", "Monstera deliciosa", "Sala Principal (Junto al ventanal)",
            "Luz indirecta brillante (media-alta). Evitar sol directo para no quemar hojas.",
            "Riego moderado (1 vez por semana). Dejar secar los primeros 3cm de sustrato.",
            "Purifica el aire absorbiendo CO2 y formaldehído; aumenta la humedad ambiental.",
            "Reduce el cortisol en sangre, genera sensación de frescura y serenidad."
        }, glm::vec3(-1.1f, 0.0f, 2.75f), 1.0f, PLANT_BUSH, glm::vec4(0.12f, 0.52f, 0.18f, 1.0f)));

        plants.push_back(PlantInstance(2, {
            "Ficus Elastica", "Ficus elastica", "Sala Principal (Rincón este)",
            "Luz filtrada abundante. Tolera sol suave de mañana.",
            "Riego semanal moderado. Es sensible al exceso de encharcamiento.",
            "Excelente extractor de toxinas en ambientes cerrados.",
            "Aporta elegancia visual y bienestar psicológico gracias a sus hojas lustrosas."
        }, glm::vec3(-2.85f, 0.0f, 1.45f), 1.25f, PLANT_TREE, glm::vec4(0.08f, 0.38f, 0.14f, 1.0f)));

        // 2. COMEDOR
        plants.push_back(PlantInstance(3, {
            "Palo de Brasil", "Dracaena fragrans", "Comedor (Esquina norte)",
            "Luz media indirecta. Ideal para interiores cálidos y luminosos.",
            "Riego moderado cada 7 a 10 días. Mantener tierra ligeramente húmeda.",
            "Regula la humedad relativa del aire combatiendo la resequedad ambiental.",
            "Transmite tranquilidad y vitalidad durante las horas de reunión familiar."
        }, glm::vec3(-0.75f, 0.0f, -0.65f), 1.15f, PLANT_TREE, glm::vec4(0.15f, 0.60f, 0.20f, 1.0f)));

        plants.push_back(PlantInstance(4, {
            "Cinta o Malamadre", "Chlorophytum comosum", "Comedor (Sobre repisa)",
            "Luz difusa media a baja. Muy resistente y adaptable.",
            "Riego regular (2 veces por semana en primavera/verano, 1 en invierno).",
            "Elimina hasta el 95% del monóxido de carbono en espacios cerrados (Estudio NASA).",
            "Fomenta el optimismo y la alegría por sus estolones colgantes decorativos."
        }, glm::vec3(-2.15f, 0.72f, -0.55f), 0.75f, PLANT_FERN, glm::vec4(0.35f, 0.75f, 0.30f, 1.0f)));

        // 3. SALA DE TELEVISIÓN
        plants.push_back(PlantInstance(5, {
            "Palmera de Salon", "Chamaedorea elegans", "Sala de TV (Junto a pantalla)",
            "Luz baja a media, ideal para cuartos con luz tenue o artificial.",
            "Riego ligero semanal. Tolera ambientes con poca humedad.",
            "Excelente humidificador natural y filtro de ondas electromagnéticas secundarias.",
            "Reduce la fatiga ocular y promueve relajación mental tras ver pantallas."
        }, glm::vec3(-1.75f, 0.0f, -2.60f), 1.10f, PLANT_TREE, glm::vec4(0.18f, 0.58f, 0.22f, 1.0f)));

        // 4. COCINA
        plants.push_back(PlantInstance(6, {
            "Aloe Vera", "Aloe barbadensis", "Cocina (Isla o barra)",
            "Luz directa abundante. Necesita ventana soleada para engrosar pencas.",
            "Riego escaso (cada 15 días). El exceso pudre las raíces fácilmente.",
            "Gel cicatrizante y antiinflamatorio para quemaduras de cocina; purifica el aire.",
            "Aporta seguridad y contacto directo con la medicina natural en el hogar."
        }, glm::vec3(-0.45f, 0.85f, -1.85f), 0.70f, PLANT_SUCCULENT, glm::vec4(0.20f, 0.65f, 0.25f, 1.0f)));

        plants.push_back(PlantInstance(7, {
            "Menta Aromatica", "Mentha spicata", "Cocina (Mostrador trasero)",
            "Luz brillante moderada con buena ventilación.",
            "Riego frecuente manteniendo el sustrato siempre húmedo pero no anegado.",
            "Repelente natural de insectos y especia culinaria digestiva.",
            "Su aroma fresco de mentol vigoriza el ánimo, aclara la mente y alivia la tensión."
        }, glm::vec3(-1.35f, 0.85f, -3.15f), 0.65f, PLANT_BUSH, glm::vec4(0.25f, 0.75f, 0.25f, 1.0f)));

        // 5. HABITACIÓN 1 (Principal)
        plants.push_back(PlantInstance(8, {
            "Sansevieria (Lengua de Suegra)", "Dracaena trifasciata", "Habitacion 1 (Frente a ventana)",
            "Gran tolerancia: vive desde sombra hasta sol directo matutino.",
            "Riego muy espaciado (cada 2 a 3 semanas). Casi inmortal a la sequía.",
            "Libera oxígeno durante la noche, mejorando la calidad del sueño profundo.",
            "Símbolo de resistencia y purificación mental; disipa energías negativas."
        }, glm::vec3(1.75f, 0.0f, 2.65f), 0.95f, PLANT_SUCCULENT, glm::vec4(0.18f, 0.45f, 0.20f, 1.0f)));

        plants.push_back(PlantInstance(9, {
            "Suculenta Echeveria", "Echeveria elegans", "Habitacion 1 (Escritorio)",
            "Luz solar directa indispensable para mantener forma compacta de roseta.",
            "Riego mínimo directo al sustrato cuando la tierra esté completamente seca.",
            "Requiere nulo mantenimiento, ideal para escritorios de trabajo/estudio.",
            "Su armoniosa simetría fractal relaja la mente durante jornadas de estudio."
        }, glm::vec3(1.10f, 0.72f, 3.45f), 0.55f, PLANT_SUCCULENT, glm::vec4(0.35f, 0.70f, 0.55f, 1.0f)));

        // 6. HABITACIÓN 2
        plants.push_back(PlantInstance(10, {
            "Cuna de Moises (Lirio de Paz)", "Spathiphyllum", "Habitacion 2 (Junto a cama)",
            "Sombra o semisombra. La luz directa quema sus flores blancas (espatas).",
            "Riego regular de 2 veces por semana; prefiere humedad alta.",
            "Absorbe esporas de moho y compuestos orgánicos volátiles del ambiente.",
            "Transmite paz profunda, induce descanso reparador y reduce la ansiedad."
        }, glm::vec3(2.85f, 0.0f, -2.55f), 0.90f, PLANT_BUSH, glm::vec4(0.12f, 0.55f, 0.20f, 1.0f), glm::vec4(0.95f, 0.95f, 0.92f, 1.0f)));

        plants.push_back(PlantInstance(11, {
            "Peperomia Esmeralda", "Peperomia caperata", "Habitacion 2 (Comoda)",
            "Luz media indirecta. No tolera sol abrasador.",
            "Riego moderado dejando secar la capa superior.",
            "Compacta y segura para mascotas, renueva el aire interior.",
            "Textura visual calmante que ayuda a desacelerar el ritmo cardíaco."
        }, glm::vec3(4.15f, 0.72f, -2.15f), 0.60f, PLANT_BUSH, glm::vec4(0.10f, 0.48f, 0.18f, 1.0f)));

        // 7. BAÑO
        plants.push_back(PlantInstance(12, {
            "Helecho Macho", "Nephrolepis exaltata", "Bano (Esquina tina)",
            "Luz filtrada húmeda. Adora la humedad de la ducha y el vapor caliente.",
            "Riego constante y pulverización periódica de follaje.",
            "Absorbe eficazmente la humedad excesiva impidiendo hongos en azulejos.",
            "Sensación de baño spa natural reconectando los sentidos con la naturaleza."
        }, glm::vec3(4.85f, 0.0f, 0.85f), 1.05f, PLANT_FERN, glm::vec4(0.22f, 0.68f, 0.22f, 1.0f)));

        plants.push_back(PlantInstance(13, {
            "Bambu de la Suerte", "Dracaena sanderiana", "Bano (Lavabo con espejo)",
            "Luz media a baja, crece perfectamente en agua o sustrato hidropónico.",
            "Cambio de agua semanal o riego frecuente si está en maceta.",
            "Oxigena cuartos cerrados y neutraliza olores del baño.",
            "Según la biofilia y el Feng Shui, promueve armonía, vigor y buena salud."
        }, glm::vec3(3.55f, 0.85f, -0.25f), 0.70f, PLANT_TREE, glm::vec4(0.28f, 0.72f, 0.20f, 1.0f)));

        // 8. PASILLO INTERIOR
        plants.push_back(PlantInstance(14, {
            "Zamioculca (Planta ZZ)", "Zamioculcas zamiifolia", "Pasillo (Bajo aplique de pared)",
            "Excelente para pasillos oscuros con poca iluminación o solo luz artificial.",
            "Riego muy espaciado mensual. Almacena agua en sus rizomas subterráneos.",
            "Filtra xileno y tolueno en zonas de tránsito y pasillos cerrados.",
            "Sus hojas enceradas aportan brillo estético en rincones oscuros."
        }, glm::vec3(-4.05f, 0.0f, -1.25f), 0.95f, PLANT_BUSH, glm::vec4(0.08f, 0.42f, 0.12f, 1.0f)));

        // 9. JARDÍN EXTERIOR
        plants.push_back(PlantInstance(15, {
            "Cactus de Barril", "Ferocactus wislizeni", "Jardin Posterior (Pleno Sol)",
            "Pleno sol directo (6+ horas diarias obligatorias). Resiste altas temperaturas.",
            "Riego mensual escaso. Sustrato poroso con arena y grava volcánica.",
            "Capta radiación ultravioleta y conserva recursos hídricos al máximo.",
            "Simboliza resiliencia, paciencia y protección en el exterior del hogar."
        }, glm::vec3(-0.60f, 0.0f, -6.60f), 1.15f, PLANT_CACTUS, glm::vec4(0.25f, 0.60f, 0.25f, 1.0f), glm::vec4(0.95f, 0.80f, 0.20f, 1.0f)));

        plants.push_back(PlantInstance(16, {
            "Lavanda Francesa", "Lavandula dentata", "Jardin Posterior (Bancal)",
            "Sol radiante directo y viento exterior. Requiere aireación constante.",
            "Riego moderado a bajo cuando el suelo esté seco.",
            "Atracción de abejas polinizadoras benéficas; repele mosquitos.",
            "El linalool aromático reduce el estrés, la frecuencia cardíaca y el insomnio."
        }, glm::vec3(-2.40f, 0.0f, -6.80f), 1.05f, PLANT_FLOWER, glm::vec4(0.30f, 0.50f, 0.35f, 1.0f), glm::vec4(0.65f, 0.35f, 0.85f, 1.0f)));

        plants.push_back(PlantInstance(17, {
            "Rosal Arbustivo", "Rosa gallica", "Jardin Posterior (Cerca baja)",
            "Sol directo al menos 5 horas al día para una floración vigorosa.",
            "Riego profundo 2 a 3 veces por semana en la base de la tierra.",
            "Producción de néctar para polinizadores y fragancia ambiental.",
            "Eleva el estado de ánimo y la satisfacción personal mediante el cuidado activo."
        }, glm::vec3(2.20f, 0.0f, -6.30f), 1.20f, PLANT_FLOWER, glm::vec4(0.18f, 0.52f, 0.20f, 1.0f), glm::vec4(0.92f, 0.15f, 0.22f, 1.0f)));
    }

    void initGeometry() {
        // 1. Cilindro paramétrico (para macetas, troncos y tallos)
        std::vector<float> cylVerts;
        std::vector<unsigned int> cylIndices;
        const int segments = 16;
        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / (float)segments * 2.0f * 3.14159265f;
            float cosA = cos(angle);
            float sinA = sin(angle);
            // Vértice inferior
            cylVerts.push_back(cosA * 0.5f); cylVerts.push_back(0.0f); cylVerts.push_back(sinA * 0.5f);
            cylVerts.push_back(cosA); cylVerts.push_back(0.0f); cylVerts.push_back(sinA);
            cylVerts.push_back((float)i / (float)segments); cylVerts.push_back(0.0f);
            // Vértice superior
            cylVerts.push_back(cosA * 0.5f); cylVerts.push_back(1.0f); cylVerts.push_back(sinA * 0.5f);
            cylVerts.push_back(cosA); cylVerts.push_back(0.0f); cylVerts.push_back(sinA);
            cylVerts.push_back((float)i / (float)segments); cylVerts.push_back(1.0f);
        }
        for (int i = 0; i < segments; ++i) {
            unsigned int b0 = i * 2;
            unsigned int b1 = (i + 1) * 2;
            cylIndices.push_back(b0); cylIndices.push_back(b0 + 1); cylIndices.push_back(b1);
            cylIndices.push_back(b1); cylIndices.push_back(b0 + 1); cylIndices.push_back(b1 + 1);
        }
        cylinderIndexCount = (unsigned int)cylIndices.size();

        glGenVertexArrays(1, &cylinderVAO);
        glGenBuffers(1, &cylinderVBO);
        unsigned int cylEBO;
        glGenBuffers(1, &cylEBO);
        glBindVertexArray(cylinderVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
        glBufferData(GL_ARRAY_BUFFER, cylVerts.size() * sizeof(float), cylVerts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylIndices.size() * sizeof(unsigned int), cylIndices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);

        // 2. Esfera UV (para follaje, cactus, copas de arbustos)
        std::vector<float> sphVerts;
        std::vector<unsigned int> sphIndices;
        const int rings = 12, sectors = 16;
        for (int r = 0; r <= rings; ++r) {
            float phi = (float)r / (float)rings * 3.14159265f;
            for (int s = 0; s <= sectors; ++s) {
                float theta = (float)s / (float)sectors * 2.0f * 3.14159265f;
                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);
                sphVerts.push_back(x * 0.5f); sphVerts.push_back(y * 0.5f); sphVerts.push_back(z * 0.5f);
                sphVerts.push_back(x); sphVerts.push_back(y); sphVerts.push_back(z);
                sphVerts.push_back((float)s / (float)sectors); sphVerts.push_back((float)r / (float)rings);
            }
        }
        for (int r = 0; r < rings; ++r) {
            for (int s = 0; s < sectors; ++s) {
                int cur = r * (sectors + 1) + s;
                int next = cur + sectors + 1;
                sphIndices.push_back(cur); sphIndices.push_back(next); sphIndices.push_back(cur + 1);
                sphIndices.push_back(cur + 1); sphIndices.push_back(next); sphIndices.push_back(next + 1);
            }
        }
        sphereIndexCount = (unsigned int)sphIndices.size();

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        unsigned int sphEBO;
        glGenBuffers(1, &sphEBO);
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, sphVerts.size() * sizeof(float), sphVerts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphIndices.size() * sizeof(unsigned int), sphIndices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);
    }

    // Renderiza las 17 plantas botánicas con materiales correctos y animación procedural de viento suave
    void Draw(Shader& shader, float globalTime) {
        for (size_t i = 0; i < plants.size(); ++i) {
            const auto& p = plants[i];
            float s = p.potScale;

            // 1. MACETA (Material Plástico / Cerámica pulida: TIPO_PLASTICO = 1)
            shader.setInt("materialType", 1);
            shader.setFloat("shininess", 64.0f);
            shader.setVec4("MaterialAmbientColor", glm::vec4(0.35f, 0.20f, 0.12f, 1.0f));
            shader.setVec4("MaterialDiffuseColor", glm::vec4(0.78f, 0.38f, 0.22f, 1.0f)); // Terracota
            shader.setVec4("MaterialSpecularColor", glm::vec4(0.85f, 0.85f, 0.85f, 1.0f));
            shader.setFloat("transparency", 1.0f);

            // Cuerpo cónico de maceta
            glm::mat4 potMat = glm::translate(glm::mat4(1.0f), p.position)
                             * glm::scale(glm::mat4(1.0f), glm::vec3(0.38f * s, 0.36f * s, 0.38f * s));
            shader.setMat4("model", potMat);
            glBindVertexArray(cylinderVAO);
            glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);

            // Borde superior de la maceta
            glm::mat4 rimMat = glm::translate(glm::mat4(1.0f), p.position + glm::vec3(0.0f, 0.34f * s, 0.0f))
                             * glm::scale(glm::mat4(1.0f), glm::vec3(0.43f * s, 0.06f * s, 0.43f * s));
            shader.setMat4("model", rimMat);
            glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);

            // 2. TIERRA (Material Opaco mate: TIPO_OPACO = 0)
            shader.setInt("materialType", 0);
            shader.setFloat("shininess", 8.0f);
            if (p.isWatered) {
                // Tierra húmeda oscura tras regar
                shader.setVec4("MaterialDiffuseColor", glm::vec4(0.12f, 0.08f, 0.05f, 1.0f));
            } else {
                // Tierra seca normal
                shader.setVec4("MaterialDiffuseColor", glm::vec4(0.28f, 0.20f, 0.14f, 1.0f));
            }
            glm::mat4 soilMat = glm::translate(glm::mat4(1.0f), p.position + glm::vec3(0.0f, 0.33f * s, 0.0f))
                              * glm::scale(glm::mat4(1.0f), glm::vec3(0.36f * s, 0.02f * s, 0.36f * s));
            shader.setMat4("model", soilMat);
            glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);

            // 3. VEGETACIÓN CON ANIMACIÓN PROCEDURAL DE VIENTO SUAVE (Rúbrica: Animación procedural)
            // Mecánica de viento: balanceo suave según tiempo y fase de la planta
            float swayAngle = std::sin(globalTime * 1.6f + (float)i * 0.8f) * 0.045f;
            glm::vec3 plantRoot = p.position + glm::vec3(0.0f, 0.35f * s, 0.0f);

            shader.setInt("materialType", 0);
            shader.setFloat("shininess", 24.0f);
            shader.setVec4("MaterialAmbientColor", p.leafColor * 0.4f);
            shader.setVec4("MaterialDiffuseColor", p.leafColor);
            shader.setVec4("MaterialSpecularColor", glm::vec4(0.20f, 0.35f, 0.20f, 1.0f));

            if (p.morphology == PLANT_TREE) {
                // Tronco leñoso
                shader.setVec4("MaterialDiffuseColor", glm::vec4(0.38f, 0.26f, 0.18f, 1.0f));
                glm::mat4 trunkMat = glm::translate(glm::mat4(1.0f), plantRoot)
                                   * glm::rotate(glm::mat4(1.0f), swayAngle * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f))
                                   * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f * s, 0.65f * s, 0.08f * s));
                shader.setMat4("model", trunkMat);
                glBindVertexArray(cylinderVAO);
                glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);

                // Copa frondosa
                shader.setVec4("MaterialDiffuseColor", p.leafColor);
                glm::mat4 foliageMat = glm::translate(glm::mat4(1.0f), plantRoot + glm::vec3(0.0f, 0.65f * s, 0.0f))
                                     * glm::rotate(glm::mat4(1.0f), swayAngle, glm::vec3(1.0f, 0.0f, 0.0f))
                                     * glm::scale(glm::mat4(1.0f), glm::vec3(0.55f * s, 0.65f * s, 0.55f * s));
                shader.setMat4("model", foliageMat);
                glBindVertexArray(sphereVAO);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }
            else if (p.morphology == PLANT_CACTUS) {
                // Tallo de cactus globular acanalado
                glm::mat4 cactusMat = glm::translate(glm::mat4(1.0f), plantRoot + glm::vec3(0.0f, 0.26f * s, 0.0f))
                                    * glm::scale(glm::mat4(1.0f), glm::vec3(0.38f * s, 0.52f * s, 0.38f * s));
                shader.setMat4("model", cactusMat);
                glBindVertexArray(sphereVAO);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

                // Flor apical del cactus
                shader.setVec4("MaterialDiffuseColor", p.accentColor);
                glm::mat4 flowerMat = glm::translate(glm::mat4(1.0f), plantRoot + glm::vec3(0.0f, 0.54f * s, 0.0f))
                                    * glm::scale(glm::mat4(1.0f), glm::vec3(0.12f * s, 0.10f * s, 0.12f * s));
                shader.setMat4("model", flowerMat);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }
            else if (p.morphology == PLANT_FLOWER) {
                // Follaje base
                glm::mat4 bushMat = glm::translate(glm::mat4(1.0f), plantRoot + glm::vec3(0.0f, 0.22f * s, 0.0f))
                                  * glm::rotate(glm::mat4(1.0f), swayAngle, glm::vec3(1.0f, 0.0f, 0.0f))
                                  * glm::scale(glm::mat4(1.0f), glm::vec3(0.48f * s, 0.40f * s, 0.48f * s));
                shader.setMat4("model", bushMat);
                glBindVertexArray(sphereVAO);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

                // Flores acentuadas (Lavanda o Rosal)
                shader.setVec4("MaterialDiffuseColor", p.accentColor);
                for (int f = 0; f < 5; ++f) {
                    float fAng = (float)f * 1.256f;
                    glm::vec3 fOffset(cos(fAng) * 0.16f * s, (0.35f + (float)f * 0.03f) * s, sin(fAng) * 0.16f * s);
                    glm::mat4 flMat = glm::translate(glm::mat4(1.0f), plantRoot + fOffset)
                                    * glm::scale(glm::mat4(1.0f), glm::vec3(0.09f * s, 0.15f * s, 0.09f * s));
                    shader.setMat4("model", flMat);
                    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
                }
            }
            else {
                // Arbusto frondoso / Suculenta / Helecho
                for (int l = 0; l < 3; ++l) {
                    float subSway = swayAngle * (1.0f + (float)l * 0.4f);
                    glm::mat4 leafMat = glm::translate(glm::mat4(1.0f), plantRoot + glm::vec3(0.0f, (0.18f + (float)l * 0.14f) * s, 0.0f))
                                      * glm::rotate(glm::mat4(1.0f), subSway, glm::vec3(1.0f, 0.0f, 0.0f))
                                      * glm::rotate(glm::mat4(1.0f), (float)l * 0.7f, glm::vec3(0.0f, 1.0f, 0.0f))
                                      * glm::scale(glm::mat4(1.0f), glm::vec3((0.44f - (float)l * 0.06f) * s, 0.28f * s, (0.44f - (float)l * 0.06f) * s));
                    shader.setMat4("model", leafMat);
                    glBindVertexArray(sphereVAO);
                    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
                }
            }
        }

        // 4. DIBUJAR REGADERA EN EL MUNDO SI NO HA SIDO RECOGIDA
        if (!playerHasCan) {
            // Material Plástico Verde (TIPO_PLASTICO = 1)
            shader.setInt("materialType", 1);
            shader.setFloat("shininess", 96.0f);
            shader.setVec4("MaterialAmbientColor", glm::vec4(0.12f, 0.35f, 0.15f, 1.0f));
            shader.setVec4("MaterialDiffuseColor", glm::vec4(0.20f, 0.72f, 0.28f, 1.0f));
            shader.setVec4("MaterialSpecularColor", glm::vec4(0.90f, 0.90f, 0.90f, 1.0f));

            // Cuerpo cilíndrico de la regadera
            glm::mat4 canMat = glm::translate(glm::mat4(1.0f), canPosition)
                             * glm::scale(glm::mat4(1.0f), glm::vec3(0.20f, 0.25f, 0.20f));
            shader.setMat4("model", canMat);
            glBindVertexArray(cylinderVAO);
            glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);

            // Pico aspersor inclinado
            glm::mat4 spoutMat = glm::translate(glm::mat4(1.0f), canPosition + glm::vec3(0.12f, 0.16f, 0.0f))
                               * glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(0, 0, 1))
                               * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.22f, 0.05f));
            shader.setMat4("model", spoutMat);
            glDrawElements(GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
    }

    // Muestra la ficha botánica y procesa el riego si procede
    void InspectPlant(size_t index) {
        if (index >= plants.size()) return;
        PlantInstance& p = plants[index];

        std::cout << "\n=======================================================" << std::endl;
        std::cout << "🌿 [FICHA BOTANICA INTERACTIVA - HOGAR BOTANICO 3D] 🌿" << std::endl;
        std::cout << "Nombre Comun:    " << p.info.commonName << std::endl;
        std::cout << "Nombre Cientif.: " << p.info.scientificName << std::endl;
        std::cout << "Ubicacion Casa:  " << p.info.roomLocation << std::endl;
        std::cout << "Requerim. Luz:   " << p.info.lightNeed << std::endl;
        std::cout << "Riego Recomend.: " << p.info.waterNeed << std::endl;
        std::cout << "Salud Fisica:    " << p.info.healthBenefit << std::endl;
        std::cout << "Salud Mental:    " << p.info.mentalBenefit << std::endl;
        std::cout << "Estado Actual:   " << (p.isWatered ? "💧 Regada (Sustrato humedo y hojas brillantes)" : "🌱 Lista para regar") << std::endl;

        if (playerHasCan) {
            p.isWatered = true;
            std::cout << "--> [ACCION]: Has regado con exito la " << p.info.commonName << "! Suelo fertilizado." << std::endl;
        } else {
            std::cout << "--> [PISTA]: Recoge la Regadera Verde en la banca del jardin exterior [Tecla G] para regar!" << std::endl;
        }
        std::cout << "=======================================================\n" << std::endl;
    }
};

#endif
