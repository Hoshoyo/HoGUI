#include "platform/platform.h"

#include <GL/glew.h>

#define ERROR_BUFFER_SIZE 512

void Concatenate(char* left, const char* right, int left_size)
{
	int i = 0;
	int j = 0;
	for (; left[i] != 0; ++i);
	for (; right[j] != 0 && i + j < left_size - 1; ++j)
		left[i + j] = right[j];
	left[i + j] = 0;
}

GLuint CreateShader(const char* vert_shader, const char* frag_shader, GLint vert_length, GLint frag_length)
{
	GLuint shader_id;

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
		char error_buffer[ERROR_BUFFER_SIZE] = {};
		glGetShaderInfoLog(vs_id, sizeof(error_buffer), NULL, error_buffer);
		PLATFORM_FATAL_ERROR(error_buffer, "Vertex shader compile error");
		return -1;
	}

	glCompileShader(fs_id);
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status)
	{
		char error_buffer[ERROR_BUFFER_SIZE] = {};
		glGetShaderInfoLog(fs_id, sizeof(error_buffer), NULL, error_buffer);
		PLATFORM_FATAL_ERROR(error_buffer, "Fragment shader compile error");
		return -1;
	}

	shader_id = glCreateProgram();
	glAttachShader(shader_id, vs_id);
	glAttachShader(shader_id, fs_id);
	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_LINK_STATUS, &compile_status);
	if (compile_status == 0)
	{
		GLchar error_buffer[ERROR_BUFFER_SIZE] = {};
		glGetProgramInfoLog(shader_id, sizeof(error_buffer), NULL, error_buffer);
		PLATFORM_FATAL_ERROR(error_buffer, "Shader linking error");
		return -1;
	}

	glValidateProgram(shader_id);
	glUseProgram(shader_id);
	return shader_id;
}

// TODO: make it platform independent
GLuint LoadShader(const char* vs_filename, const char* fs_filename)
{
	HANDLE vs_file = CreateFile(vs_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (vs_file == INVALID_HANDLE_VALUE)
	{
		char buffer[ERROR_BUFFER_SIZE] = "Could not open file ";
		Concatenate(buffer, vs_filename, ERROR_BUFFER_SIZE);
		PLATFORM_FATAL_ERROR(buffer, "");
		return -1;
	}
	DWORD vs_size = GetFileSize(vs_file, 0);
	char* vertex_shader = (char*)VirtualAlloc(0, vs_size, MEM_COMMIT, PAGE_READWRITE);
	OVERLAPPED ol = {};
	ReadFileEx(vs_file, vertex_shader, vs_size, &ol, 0);
	CloseHandle(vs_file);

	HANDLE fs_file = CreateFile(fs_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (fs_file == INVALID_HANDLE_VALUE)
	{
		char buffer[ERROR_BUFFER_SIZE] = "Could not open file ";
		Concatenate(buffer, fs_filename, ERROR_BUFFER_SIZE);
		MessageBox(0, buffer, 0, 0);
		return -1;
	}
	DWORD fs_size = GetFileSize(fs_file, 0);
	char* fragment_shader = (char*)VirtualAlloc(0, fs_size, MEM_COMMIT, PAGE_READWRITE);
	ReadFileEx(fs_file, fragment_shader, fs_size, &ol, 0);
	CloseHandle(fs_file);


	GLuint shader = CreateShader(vertex_shader, fragment_shader, vs_size, fs_size);

	VirtualFree(vertex_shader, 0, MEM_RELEASE);
	VirtualFree(fragment_shader, 0, MEM_RELEASE);

	return shader;
}

