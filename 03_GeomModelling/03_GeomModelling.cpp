/*
* Laboratorio de Computación Gráfica e Interacción Humano-Computadora
* 03 - Modelado Geométrico
*/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"

// Clases para construcción de geometrías
#include "Mesh.h"
#include "Triangle.h"
#include "Triangle3D.h"
#include "Plane.h"
#include "Axis.h"
#include "Cube.h"
#include "Circle.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "MathFunction.h"
#include "Cone.h"

// Funciones
bool Start();
bool Update();

// Definición de callbacks
void mouseCallback    (GLFWwindow* window, double xpos, double ypos);       // Mouse
void scrollCallback   (GLFWwindow* window, double xoffset, double yoffset); // Mouse movement
void keyboardCallback (GLFWwindow* window);                                 // Keyboard
void resizeCallback   (GLFWwindow* window, int width, int height);          // Window resize

using namespace std;

// Variables globales
GLFWwindow* window;
GLfloat  bgR, bgG, bgB, bgA;
Shader    * myShader, *myOtherShader;
Mesh      *mesh, *meshAxis, *meshPlane, *meshCircle, 
          *meshCylinder, *meshCube, *meshSphere,
          *meshCone;
int       screenWidth = 600, screenHeight = 600;

// Definición de cámara (XYZ position)
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f),  // Vector de Posición
              glm::vec3(0.0f, 0.0f, -1.0f), // Vector de Dirección (View)
              glm::vec3(0.0f, 1.0f, 0.0f)); // Vector de orientación (Up)

int main(void)
{
    if (!Start())
        return -1;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        if (!Update())
            break;
    }

    glfwTerminate();
    return 0;
}

bool Start() {

    /* Initialize the library GLFW */
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "Geometric Modeling", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Define callbacks */
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Depth test activation
    glEnable(GL_DEPTH_TEST);

    // Background color initialization
    bgR = bgG = bgB = bgA = 1.0f;
    
    // Shaders compilation and linking
    myShader = new Shader("shaders/02-simplePVM.vs", "shaders/02-simplePVM.fs");
    if (myShader == nullptr) {
        cout << "Error creating shaders." << endl;
        return false;
    }

    myOtherShader = new Shader("shaders/02-simplePVM.vs", "shaders/02-simplePVM.fs");
    if (myOtherShader == nullptr) {
        cout << "Error creating shaders." << endl;
        return false;
    }

    // Definición de ejes coordenados indicadas como líneas
    // rojo, verde y azul
    Axis axis(10.0f);
    meshAxis = new Mesh(axis.vertices, axis.indices);
    if (meshAxis == nullptr) {
        cout << "Error creating mesh axis object." << endl;
        return false;
    }

    // Ejercicio 1: Plano paralelo al plano XY
    /*
    Plane plane(1.0f);
    meshPlane = new Mesh(plane.vertices, plane.indices);
    if (meshPlane == nullptr) {
        cout << "Error creating mesh plane object." << endl;
        return false;
    }
    */
    
    // Ejercicio 2: Cubo
    /*
    Cube cube(1.0f);
    meshCube = new Mesh(cube.vertices, cube.indices);
    if (meshCube == nullptr) {
        cout << "Error creating mesh cube object." << endl;
        return false;
    }
    */

    // Ejercicio 3: Círculo
    /*
    Circle circle(1.0f);
    meshCircle = new Mesh(circle.vertices, circle.indices);
    if (meshCircle == nullptr) {
        cout << "Error creating mesh circle object." << endl;
        return false;
    }
    */

    // Ejercicio 4: Cilindro
    
    /*
    Cylinder cylinder(1.0f);
    meshCylinder = new Mesh(cylinder.vertices, cylinder.indices);
    if (meshCylinder == nullptr) {
        cout << "Error creating mesh cylinder object." << endl;
        return false;
    }
    */

    // Ejercicio 5: Esfera
    // Implementar el algoritmo de construcción de caras
    // En la clase Sphere.h
    /*
    Sphere sphere(1.0);
    meshSphere = new Mesh(sphere.vertices, sphere.indices);
    if (meshSphere == nullptr) {
        cout << "Error creating mesh object." << endl;
        return false;
    }
    */

    // Ejercicio 6: Cono
    /*
    Cone cone(1.0);
    meshCone = new Mesh(cone.vertices, cone.indices);
    if (meshCone == nullptr) {
        cout << "Error creating mesh object." << endl;
        return false;
    }
    */
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    return true;
}

bool Update() {
    /* Process keyboard */
    keyboardCallback(window);
    
    /* Change the background Color */
    glClearColor(bgR, bgG, bgB, bgA);
    
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    if (meshAxis != nullptr) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshAxis->Draw(*myOtherShader);

    }

    glUseProgram(0);
    
    // Ejercicio 1: Plano paralelo al plano XY
    /*
    if (meshPlane != nullptr) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshPlane->Draw(*myOtherShader);

    }
    
    glUseProgram(0);
    */

    // Ejercicio 2: Cubo
    /*
    if ( meshCube != nullptr ) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshCube->Draw(*myOtherShader);

    }

    glUseProgram(0);
    */

    // Ejercicio 3: Círculo
    /*
    if (meshCircle != nullptr) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshCircle->Draw(*myOtherShader);

    }

    glUseProgram(0);
    */
    
    // Ejercicio 4: Cilindro
    /*
    if (meshCylinder != nullptr) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshCylinder->Draw(*myOtherShader);

    }
    glUseProgram(0);
    */

    // Ejercicio 5: Esfera
    // Implementar el algoritmo de construcción de caras
    // En la clase Sphere.h
    /*
    if (meshSphere != nullptr) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshSphere->Draw(*myOtherShader);

    }
    glUseProgram(0);
    */

    // Ejercicio 6: Cono
    /*
    if (meshCone != nullptr) {
        glUseProgram(myOtherShader->ID);

        // Aplicamos transformaciones de proyección y cámara (si las hubiera)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
        myOtherShader->setMat4("projection", projection);

        glm::mat4 view = glm::lookAt(camera.Position, camera.View, camera.Up);
        myOtherShader->setMat4("view", view);

        // Aplicamos transformaciones del modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        myOtherShader->setMat4("model", model);

        meshCone->Draw(*myOtherShader);

    }
    glUseProgram(0);
    */

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();

    return true;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    // Do something when mouse actioned
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Do something when mouse moved
}

void keyboardCallback(GLFWwindow* window) {
    // Do something when key pressed

    // Exit action
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Standard keys for 3D movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {

    }

    // User defined
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {

    }

    // Special keys
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {

    }

    // Arrows
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {

    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {

    }

    // Functions
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        // Vista frontal
        camera.Position = glm::vec3(0.0f, 0.0f, 5.0f);
        camera.View = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
        // Vista oblícua
        camera.Position = glm::vec3(5.0f, 5.0f, 5.0f);
        camera.View = glm::vec3(-1.0f, -1.0f, -1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {

    }
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}