#ifndef SPHERE_H
#define SPHERE_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Vertex.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;
using namespace glm;

class Sphere {
public:
	Sphere(float size) {
		buildVertices(size);
		buildIndices();
	}

	~Sphere() {
		vertices.clear();
		indices.clear();
	}

	vector<Vertex>       vertices;
	vector<unsigned int> indices;

private:

	void buildVertices(float size) {
		// Construcción de los puntos de la esfera
		for (int phi = 0; phi < 180; phi += 5) {
			for (int theta = 0; theta < 360; theta += 5) {
				Vertex  v;
				double trdeg = glm::radians((double)theta);
				double prdeg = glm::radians((double)phi);
				v.Position.x = size * (float)sin(prdeg) * (float)cos(trdeg);
				v.Position.y = size * (float)sin(prdeg) * (float)sin(trdeg);
				v.Position.z = size * (float)cos(prdeg);
				if (theta == 0) { v.Color.r = 0.0f; v.Color.g = 1.0f; v.Color.b = 1.0f; }
				if (theta == 120) { v.Color.r = 1.0f; v.Color.g = 0.0f; v.Color.b = 1.0f; }
				if (theta == 240) { v.Color.r = 0.0f; v.Color.g = 0.0f; v.Color.b = 1.0f; }
				v.Color.a = 1.0f;
				vertices.push_back(v);
			}
		}
	}

	void buildIndices() {
		// Ejercicio 5: Esfera
		// Aquí implementar el Algoritmo para construir las caras
		// de la geometría esfera


		// 
	}
};

#endif