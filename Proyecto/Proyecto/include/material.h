#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Tipos de Material para la Rúbrica de Evaluación
enum MaterialTypeEnum {
	MAT_OPACO = 0,
	MAT_PLASTICO = 1,
	MAT_METALICO = 2,
	MAT_TRASLUCIDO = 3
};

class Material 
{
public:
	// Atributos de Material
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float     transparency;
	float     shininess;
	int       type; // 0 = OPACO, 1 = PLASTICO, 2 = METALICO, 3 = TRASLUCIDO (Fresnel)

	Material() {
		ambient = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f); 
		diffuse = glm::vec4(0.65f, 0.65f, 0.65f, 1.0f);
		specular = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f); 
		transparency = 1.0f;
		shininess = 32.0f;
		type = MAT_OPACO;
	}
	~Material() {}
};

#endif
