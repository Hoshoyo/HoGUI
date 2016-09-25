#pragma once

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

struct FontShader
{
	GLuint id;
	GLuint uni_projection;
	GLuint uni_color;

	glm::mat4 projection_matrix;
};

inline void SetOrthoProjection(glm::mat4* matrix, float width, float height)
{
	*matrix = glm::ortho(0.0f, width, 0.0f, height);
}

inline void SetPerspectiveProjection(glm::mat4* matrix, float width, float height, float fov, float znear, float zfar)
{
	*matrix = glm::perspective(fov, width / height, znear, zfar);
}
