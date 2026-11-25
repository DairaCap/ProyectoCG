#pragma once
// ============================================================================
// UI.h - Interfaz 2D sencilla: rectangulos, imagenes y texto con una fuente de
// mapa de bits 5x7 propia (sin librerias externas de texto).
//
// Las coordenadas se dan en PIXELES de la ventana, con el origen arriba a la izquierda.
// Uso por cuadro:  ui.Begin();  ui.Panel(...);  ui.Texto(...);  ui.End();
// ============================================================================
#include <string>
#include <vector>
#include <cstring>
#include <cmath>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "shader_m.h"

// ---------------------------------------------------------------------------
// Fuente 5x7: cada letra son 7 renglones de 5 columnas ('#' = pixel encendido).
// Los codigos '\1' y '\2' son los signos de apertura "¡" y "¿".
// ---------------------------------------------------------------------------
struct GlifoDef { char c; const char* f[7]; };

inline const GlifoDef kGlifos[] = {
    {' ',  {".....", ".....", ".....", ".....", ".....", ".....", "....."}},
    {'A',  {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
    {'B',  {"####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."}},
    {'C',  {".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."}},
    {'D',  {"####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."}},
    {'E',  {"#####", "#....", "#....", "####.", "#....", "#....", "#####"}},
    {'F',  {"#####", "#....", "#....", "####.", "#....", "#....", "#...."}},
    {'G',  {".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".####"}},
    {'H',  {"#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
    {'I',  {".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."}},
    {'J',  {"..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."}},
    {'K',  {"#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"}},
    {'L',  {"#....", "#....", "#....", "#....", "#....", "#....", "#####"}},
    {'M',  {"#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"}},
    {'N',  {"#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#"}},
    {'O',  {".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
    {'P',  {"####.", "#...#", "#...#", "####.", "#....", "#....", "#...."}},
    {'Q',  {".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"}},
    {'R',  {"####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"}},
    {'S',  {".####", "#....", "#....", ".###.", "....#", "....#", "####."}},
    {'T',  {"#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."}},
    {'U',  {"#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
    {'V',  {"#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."}},
    {'W',  {"#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"}},
    {'X',  {"#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"}},
    {'Y',  {"#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."}},
    {'Z',  {"#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"}},
    {'0',  {".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."}},
    {'1',  {"..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."}},
    {'2',  {".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"}},
    {'3',  {"####.", "....#", "....#", ".###.", "....#", "....#", "####."}},
    {'4',  {"...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."}},
    {'5',  {"#####", "#....", "####.", "....#", "....#", "#...#", ".###."}},
    {'6',  {".###.", "#....", "#....", "####.", "#...#", "#...#", ".###."}},
    {'7',  {"#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."}},
    {'8',  {".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."}},
    {'9',  {".###.", "#...#", "#...#", ".####", "....#", "....#", ".###."}},
    {'.',  {".....", ".....", ".....", ".....", ".....", ".##..", ".##.."}},
    {',',  {".....", ".....", ".....", ".....", ".##..", "..#..", ".#..."}},
    {':',  {".....", ".##..", ".##..", ".....", ".##..", ".##..", "....."}},
    {'!',  {"..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.."}},
    {'?',  {".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.."}},
    {'-',  {".....", ".....", ".....", "#####", ".....", ".....", "....."}},
    {'/',  {"....#", "....#", "...#.", "..#..", ".#...", "#....", "#...."}},
    {'(',  {"...#.", "..#..", ".#...", ".#...", ".#...", "..#..", "...#."}},
    {')',  {".#...", "..#..", "...#.", "...#.", "...#.", "..#..", ".#..."}},
    {'\1', {"..#..", ".....", "..#..", "..#..", "..#..", "..#..", "..#.."}},   // ¡
    {'\2', {"..#..", ".....", "..#..", ".#...", "#....", "#...#", ".###."}},   // ¿
};
inline const int kNumGlifos = (int)(sizeof(kGlifos) / sizeof(kGlifos[0]));

class UI {
public:
    float W = 1280.0f, H = 720.0f;       // Tamano de la ventana en pixeles (el mismo que usa main.cpp)

    // Colores de la interfaz (paleta calida de la casa: madera, crema y verde de plantas)
    static glm::vec4 Fondo()    { return glm::vec4(0.13f, 0.10f, 0.08f, 0.90f); }   // Madera muy oscura, casi opaca
    static glm::vec4 Borde()    { return glm::vec4(0.67f, 0.53f, 0.39f, 1.00f); }   // Madera clara (#AB8863)
    static glm::vec4 Crema()    { return glm::vec4(0.83f, 0.80f, 0.77f, 1.00f); }   // Crema (#D3CCC5)
    static glm::vec4 Verde()    { return glm::vec4(0.55f, 0.75f, 0.35f, 1.00f); }   // Verde de planta
    static glm::vec4 Rojo()     { return glm::vec4(0.85f, 0.30f, 0.26f, 1.00f); }   // Rojo apagado

    bool Init(float ancho, float alto) {
        W = ancho; H = alto;
        shader = new Shader("shaders/ui.vs", "shaders/ui.fs");

        // Cuadro unitario (2 triangulos) que se escala y mueve con uRect
        float quad[] = { 0,0,  1,0,  1,1,   0,0,  1,1,  0,1 };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);

        ConstruirFuente();
        return true;
    }

    // Prepara el estado para dibujar interfaz (sin profundidad, con transparencia)
    void Begin() {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader->use();
        shader->setInt("uTex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fontTex);       // Siempre hay una textura valida en la unidad 0 (si no, algunos drivers avisan)
        glBindVertexArray(vao);
    }
    void End() {
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
    }

    // Rectangulo de color liso (x, y = esquina superior izquierda, en pixeles)
    void Rect(float x, float y, float w, float h, const glm::vec4& color) {
        shader->setInt("uMode", 0);
        shader->setVec4("uColor", color);
        Cuadro(x, y, w, h, glm::vec4(0, 0, 1, 1));
    }

    // Panel con borde: fondo oscuro y marco de madera
    void Panel(float x, float y, float w, float h) {
        const float b = 4.0f;
        Rect(x - b, y - b, w + 2 * b, h + 2 * b, Borde());
        Rect(x, y, w, h, Fondo());
    }

    // Imagen RGBA (textura de OpenGL) con tinte opcional
    void Imagen(GLuint tex, float x, float y, float w, float h, const glm::vec4& tinte = glm::vec4(1.0f)) {
        if (tex == 0) return;
        shader->setInt("uMode", 1);
        shader->setVec4("uColor", tinte);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        Cuadro(x, y, w, h, glm::vec4(0, 0, 1, 1));
    }

    // Ancho en pixeles de un texto con el tamano dado (escala = pixeles de pantalla por pixel de la fuente)
    float AnchoTexto(const std::string& s, float escala) const {
        int n = (int)Decodificar(s).size();
        return n > 0 ? (n * 6 - 1) * escala : 0.0f;
    }

    // Escribe texto en mayusculas (acepta tildes, ñ y ¡ ¿). Regresa el ancho dibujado.
    float Texto(const std::string& s, float x, float y, float escala, const glm::vec4& color, bool sombra = true) {
        if (sombra) DibujarLetras(s, x + escala, y + escala, escala, glm::vec4(0, 0, 0, color.a * 0.6f));
        return DibujarLetras(s, x, y, escala, color);
    }
    // Texto centrado horizontalmente en el intervalo [x, x + ancho]
    void TextoCentrado(const std::string& s, float x, float ancho, float y, float escala, const glm::vec4& color) {
        Texto(s, x + (ancho - AnchoTexto(s, escala)) * 0.5f, y, escala, color);
    }

    // Texto que baja de renglon al llegar a maxAncho. Con dibujar = false solo mide.
    // Regresa la coordenada y donde termina el texto (para apilar parrafos).
    float TextoEnvuelto(const std::string& s, float x, float y, float maxAncho, float escala,
                        const glm::vec4& color, bool dibujar = true) {
        float alto = 7 * escala, sep = 4 * escala;
        std::string linea, palabra;
        auto vaciar = [&]() {
            if (dibujar && !linea.empty()) Texto(linea, x, y, escala, color);
            y += alto + sep;
            linea.clear();
        };
        std::string texto = s + " ";
        for (char c : texto) {
            if (c != ' ') { palabra += c; continue; }
            if (palabra.empty()) continue;
            std::string prueba = linea.empty() ? palabra : linea + " " + palabra;
            if (!linea.empty() && AnchoTexto(prueba, escala) > maxAncho) { vaciar(); linea = palabra; }
            else linea = prueba;
            palabra.clear();
        }
        if (!linea.empty()) vaciar();
        return y;
    }

    // Carga una imagen como textura de OpenGL (regresa 0 si no existe); aspecto = ancho / alto
    static GLuint CargarTextura(const std::string& ruta, float* aspecto = nullptr) {
        int w, h, c;
        unsigned char* px = stbi_load(ruta.c_str(), &w, &h, &c, 4);       // Siempre RGBA
        if (!px) { std::cout << "[UI] No se pudo cargar " << ruta << std::endl; return 0; }
        GLuint t; glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        stbi_image_free(px);
        if (aspecto) *aspecto = (float)w / (float)h;
        return t;
    }

private:
    Shader* shader = nullptr;
    GLuint vao = 0, vbo = 0, fontTex = 0;
    int indice[256];                      // Codigo de caracter -> posicion en kGlifos (-1 si no existe)

    // Una letra a dibujar: posicion en kGlifos y la marca que lleva encima (0 ninguna, 1 acento, 2 tilde ñ, 3 dieresis)
    struct Letra { int glifo; int marca; };

    // Arma la textura de la fuente: una tira de 5 x 7 pixeles por letra, un solo canal
    void ConstruirFuente() {
        for (int i = 0; i < 256; ++i) indice[i] = -1;
        int w = kNumGlifos * 5, h = 7;
        std::vector<unsigned char> datos(w * h, 0);
        for (int g = 0; g < kNumGlifos; ++g) {
            indice[(unsigned char)kGlifos[g].c] = g;
            for (int r = 0; r < 7; ++r)
                for (int c = 0; c < 5; ++c)
                    if (kGlifos[g].f[r][c] == '#') datos[r * w + g * 5 + c] = 255;
        }
        glGenTextures(1, &fontTex);
        glBindTexture(GL_TEXTURE_2D, fontTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                  // Filas de 1 byte (el ancho no es multiplo de 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, datos.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   // Letras de pixeles nitidos
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Convierte el texto en letras. Entiende UTF-8 (2 bytes) y Latin-1/Windows-1252 (1 byte), asi funciona
    // igual sin importar como guarde el compilador las tildes de las cadenas.
    std::vector<Letra> Decodificar(const std::string& s) const {
        std::vector<Letra> out;
        for (size_t i = 0; i < s.size(); ++i) {
            unsigned char b = (unsigned char)s[i];
            int cp = b;
            if ((b == 0xC2 || b == 0xC3) && i + 1 < s.size() && ((unsigned char)s[i + 1] & 0xC0) == 0x80) {
                cp = ((b & 0x1F) << 6) | ((unsigned char)s[i + 1] & 0x3F);   // UTF-8 de 2 bytes
                ++i;
            }
            char base = 0; int marca = 0;
            switch (cp) {
                case 0xC1: case 0xE1: base = 'A'; marca = 1; break;   // Á á
                case 0xC9: case 0xE9: base = 'E'; marca = 1; break;   // É é
                case 0xCD: case 0xED: base = 'I'; marca = 1; break;   // Í í
                case 0xD3: case 0xF3: base = 'O'; marca = 1; break;   // Ó ó
                case 0xDA: case 0xFA: base = 'U'; marca = 1; break;   // Ú ú
                case 0xDC: case 0xFC: base = 'U'; marca = 3; break;   // Ü ü
                case 0xD1: case 0xF1: base = 'N'; marca = 2; break;   // Ñ ñ
                case 0xA1: base = '\1'; break;                        // ¡
                case 0xBF: base = '\2'; break;                        // ¿
                default:
                    if (cp < 128) base = (char)((cp >= 'a' && cp <= 'z') ? cp - 32 : cp);   // Todo en mayusculas
                    break;
            }
            int g = (base != 0) ? indice[(unsigned char)base] : -1;
            if (g < 0) g = indice[(unsigned char)' '];                // Caracter desconocido = espacio
            out.push_back({ g, marca });
        }
        return out;
    }

    float DibujarLetras(const std::string& s, float x, float y, float e, const glm::vec4& color) {
        std::vector<Letra> letras = Decodificar(s);
        shader->setInt("uMode", 2);
        shader->setVec4("uColor", color);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fontTex);
        float total = (float)(kNumGlifos * 5);
        // Se alinea a pixeles enteros: con medio pixel de desfase la textura se muestrea justo en el borde de una
        // letra y se cuelan rayitas de la letra vecina.
        x = std::round(x); y = std::round(y);
        float cx = x;
        for (const Letra& l : letras) {
            glm::vec4 uv((l.glifo * 5) / total, 0.0f, (l.glifo * 5 + 5) / total, 1.0f);
            Cuadro(cx, y, 5 * e, 7 * e, uv);
            if (l.marca != 0) Marca(l.marca, cx, y, e, color);
            cx += 6 * e;                                              // 5 pixeles de letra + 1 de espacio
        }
        return cx - x - (letras.empty() ? 0.0f : e);
    }

    // Tilde, acento o dieresis: pixeles sueltos encima de la letra
    void Marca(int tipo, float x, float y, float e, const glm::vec4& color) {
        static const int acento[][2] = { {3, -2}, {2, -1} };
        static const int tilde[][2]  = { {0, -1}, {1, -2}, {2, -2}, {3, -1} };
        static const int dier[][2]   = { {1, -2}, {3, -2} };
        const int (*pts)[2] = (tipo == 1) ? acento : (tipo == 2) ? tilde : dier;
        int n = (tipo == 2) ? 4 : 2;
        shader->setInt("uMode", 0);
        shader->setVec4("uColor", color);
        for (int i = 0; i < n; ++i) Cuadro(x + pts[i][0] * e, y + pts[i][1] * e, e, e, glm::vec4(0, 0, 1, 1));
        shader->setInt("uMode", 2);                                   // Se regresa al modo letra
        shader->setVec4("uColor", color);
    }

    // Dibuja el cuadro unitario en un rectangulo de pixeles con las coordenadas de textura uv
    void Cuadro(float x, float y, float w, float h, const glm::vec4& uv) {
        float x0 = x / W * 2.0f - 1.0f, x1 = (x + w) / W * 2.0f - 1.0f;
        float yArriba = 1.0f - y / H * 2.0f, yAbajo = 1.0f - (y + h) / H * 2.0f;
        shader->setVec4("uRect", glm::vec4(x0, yAbajo, x1, yArriba));
        shader->setVec4("uUV", uv);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};
