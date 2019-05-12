#include "shader.h"
#include "../os.h"
#include <ho_gl.h>
#include <string.h>
#include <stdio.h>

Shader shader_load(const char* vertex_shader_path, const char* fragment_shader_path)
{
	s32 vertex_shader_length, fragment_shader_length;
	char* vertex_shader_buffer = os_file_read(vertex_shader_path, &vertex_shader_length, malloc);
	char* fragment_shader_buffer = os_file_read(fragment_shader_path, &fragment_shader_length, malloc);
	u32 shader = shader_load_from_buffer(vertex_shader_buffer, fragment_shader_buffer,
		vertex_shader_length, fragment_shader_length);
	os_file_free(vertex_shader_buffer);
	os_file_free(fragment_shader_buffer);
	return shader;
}

Shader shader_load_from_buffer(const s8* vert_shader, const s8* frag_shader, int vert_length, int frag_length)
{
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	GLint compile_status;

	const GLchar* p_v[1] = { vert_shader };
	glShaderSource(vs_id, 1, p_v, &vert_length);

	const GLchar* p_f[1] = { frag_shader };
	glShaderSource(fs_id, 1, p_f, &frag_length);

	glCompileShader(vs_id);
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		printf("shader_load: Error compiling vertex shader: %s", error_buffer);
		return -1;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		char error_buffer[512] = { 0 };
		glGetShaderInfoLog(fs_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		printf("shader_load: Error compiling fragment shader: %s", error_buffer);
		return -1;
	}

	GLuint shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0) {
		GLchar error_buffer[512] = { 0 };
		glGetProgramInfoLog(shader_id, sizeof(error_buffer) - 1, NULL, error_buffer);
		printf("shader_load: Error linking program: %s", error_buffer);
		return -1;
	}

	glValidateProgram(shader_id);
	return shader_id;
}

void shader_delete(Shader shader)
{
	glDeleteProgram(shader);
}

Shader_HL shader_hl_load(const char* vertex_shader_path, const char* fragment_shader_path)
{
	Shader_HL shl;
	shl.shader = shader_load(vertex_shader_path, fragment_shader_path);
	shl.vertex_shader_path = malloc(strlen(vertex_shader_path) + 1);
	shl.fragment_shader_path = malloc(strlen(fragment_shader_path) + 1);
	memcpy(shl.vertex_shader_path, vertex_shader_path, strlen(vertex_shader_path) + 1);
	memcpy(shl.fragment_shader_path, fragment_shader_path, strlen(fragment_shader_path) + 1);
	return shl;
}

void shader_hl_update(Shader_HL* shl)
{
	shader_delete(shl->shader);
	shl->shader = shader_load(shl->vertex_shader_path, shl->fragment_shader_path);
}

void shader_hl_destroy(const Shader_HL* shl)
{
	free(shl->vertex_shader_path);
	free(shl->fragment_shader_path);
	shader_delete(shl->shader);
}