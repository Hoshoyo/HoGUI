#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

struct Transform
{
	glm::vec3 position;
	glm::vec3 orientation;
	glm::vec3 scale;
	glm::mat4 model_matrix;
};

struct Model
{
	int num_indices;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	Transform transform;
};