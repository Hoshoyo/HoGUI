#include "gui_shader.h"

GLuint LoadShader(const char* vert_shader, const char* frag_shader, GLint vert_length, GLint frag_length)
{
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	GLint compile_status;

	const GLchar* p_v[1]{ vert_shader };
	glShaderSource(vs_id, 1, p_v, &vert_length);

	const GLchar* p_f[1]{ frag_shader };
	glShaderSource(fs_id, 1, p_f, &frag_length);

	glCompileShader(vs_id);
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status)
	{
		char error_buffer[512] = {};
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		std::cout << "Vert shader error: " << error_buffer << std::endl;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status)
	{
		char error_buffer[512] = {};
		glGetShaderInfoLog(fs_id, sizeof(error_buffer), NULL, error_buffer);
		std::cout << "Frag shader error: " << error_buffer << std::endl;
	}

	GLuint shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0)
	{
		GLchar error_buffer[512] = {};
		glGetProgramInfoLog(shader_id, sizeof(error_buffer), NULL, error_buffer);
		std::cout << "Link shader error: " << error_buffer << std::endl;
	}

	glValidateProgram(shader_id);
	glUseProgram(shader_id);
	return shader_id;
}