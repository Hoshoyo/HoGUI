#pragma once
#include "freetype-gl/freetype-gl.h"
#include "gui_shader.h"

#include <glm/glm.hpp>

struct TextInfo
{
	int num_rows;
	float width;
	float height;
	glm::vec2 start_position;
	glm::vec2 end_position;
};

struct FontRenderer
{
	FontRenderer(int quality, int font_size, glm::vec4& color)
		: font_size(font_size)
	{
		// **************************************************************
		// TODO: Make font choice more generic and get rid of freetype-gl
		// **************************************************************

		atlas = ftgl::texture_atlas_new(quality, quality, 1);
		font = ftgl::texture_font_new_from_file(atlas, (float)font_size, "res/LiberationMono-Regular.ttf");
		//font = ftgl::texture_font_new_from_file(atlas, (float)font_size, "res/times.ttf");
		font_color = color;

		InitFontRenderer();
	}

	~FontRenderer()
	{
		ftgl::texture_atlas_delete(atlas);
		ftgl::texture_font_delete(font);
	}

	int font_size;

	ftgl::texture_atlas_t* atlas;
	ftgl::texture_font_t* font;

	glm::vec2 positions[4];
	glm::vec2 texCoords[4];
	glm::vec4 font_color;

	// ********************************
	// TODO: Make it OpenGL independent
	// ********************************

	GLuint vao, vbo, ebo, tbo;
	GLuint shader_id;
	GLuint font_color_uniform;
	GLuint clip_topleft_uniform, clip_botright_uniform;
	unsigned int indices[6];

	void InitFontRenderer()
	{
		positions[0] = glm::vec2(-1.0f, -1.0f);
		positions[0] = glm::vec2(1.0f, -1.0f);
		positions[0] = glm::vec2(-1.0f, 1.0f);
		positions[0] = glm::vec2(1.0f, 1.0f);

		texCoords[0] = glm::vec2(0.0f, 0.0f);
		texCoords[1] = glm::vec2(1.0f, 0.0f);
		texCoords[2] = glm::vec2(0.0f, 1.0f);
		texCoords[3] = glm::vec2(1.0f, 1.0f);

		indices[0] = 0;
		indices[1] = 3;
		indices[2] = 1;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &tbo);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(unsigned int), indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions) * sizeof(float) * 2, positions, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDisableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords) * sizeof(float) * 2, texCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glDisableVertexAttribArray(1);

		glBindVertexArray(0);

		CreateFontShader();
	}

	void CreateFontShader()
	{
		const char vert_shader[] =
			"#version 330\n"

			"layout(location = 0) in vec3 vertex_position;\n"
			"layout(location = 1) in vec2 texCoords;\n"

			"out vec2 uvCoords;\n"
			"out vec4 position;\n"
			"out vec4 color;\n"

			"uniform vec4 font_color;\n"

			"void main()\n"
			"{\n"
			"	gl_Position = vec4(vertex_position, 1.0);\n"
			"	position = gl_Position;\n"
			"	uvCoords = texCoords;\n"
			"	color = font_color;\n"
			"}\n";

		const char frag_shader[] =
			"#version 330\n"

			"in vec2 uvCoords;\n"
			"in vec4 position;\n"
			"in vec4 color;\n"

			"out vec4 out_Color;\n"

			"uniform sampler2D textTexture;\n"
			"uniform vec2 clipTL;\n"
			"uniform vec2 clipBR;\n"

			"void main() {\n"

			//"	if (position.x < clipTL.x || position.x > clipBR.x)\n"
			//"		discard;\n"
			//"	if (position.y > -clipTL.y || position.y < -clipBR.y)\n"
			//"		discard;\n"

			"	vec4 textureColor = texture(textTexture, uvCoords);\n"
			"	vec4 finalColor = vec4(color.rgb, textureColor.r * color.a);\n"
			"	out_Color = finalColor;\n"
			"}\n";

		shader_id = LoadShader(vert_shader, frag_shader, sizeof(vert_shader), sizeof(frag_shader));
		font_color_uniform = glGetUniformLocation(shader_id, "font_color");
		clip_topleft_uniform = glGetUniformLocation(shader_id, "clipTL");
		clip_botright_uniform = glGetUniformLocation(shader_id, "clipBR");
	}

	TextInfo RenderText(char* text, int text_size, float xPos, float yPos, float pixelWidthLimit, bool wordFormat)
	{
		TextInfo result;
		result.start_position = glm::vec2(0, 0);
		result.end_position = glm::vec2(0, 0);

		float scaleX = 800.0f;
		float scaleY = 600.0f;

		float x = xPos / scaleX;
		float firstPos = (float)xPos;
		float limitX = (pixelWidthLimit + xPos) / scaleX;
		int yAdvance = 0;

		auto fit = [&text, pixelWidthLimit](int index, float xPos, float headStart, ftgl::texture_glyph_t* glyph)
		{
			float x = (float)xPos;
			for (; text[index + 1] != ' ' && text[index + 1] != 0; ++index)
			{
				unsigned char c = text[index];
				x += glyph->advance_x;
				if ((xPos + pixelWidthLimit) < x + headStart)
					return false;
			}
			return true;
		};

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(shader_id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlas->id);

		for (int i = 0; i < text_size; i++)
		{
			unsigned char c = text[i];
			ftgl::texture_glyph_t* glyph = ftgl::texture_font_get_glyph(font, c);
			if (glyph != NULL)
			{
				if (i > 0)
				{
					float kerning = texture_glyph_get_kerning(glyph, text[i - 1]);
					x += kerning / scaleX;
				}

				if (i > 0)
				{
					if (text[i - 1] == ' ' && text[i] != ' ')
					{
						if (!fit(i, firstPos, x * scaleX, glyph) && c != ' ')
						{
							x = xPos / scaleX;
							yPos -= font->ascender - font->descender;	// highest glyph height - lowest glyph height
							yAdvance++;
						}
					}
				}

				float x0 = x + glyph->offset_x / scaleX;
				float y0 = (yPos + glyph->offset_y) / scaleY;
				float x1 = x0 + glyph->width / scaleX;
				float y1 = y0 - glyph->height / scaleY;

				if (i == 0)
				{
					result.start_position.x = x0;
					result.start_position.y = y0;
				}
				if (x1 > result.end_position.x)
					result.end_position.x = x1;
				result.end_position.y = y1;

				positions[0] = glm::vec2(x0, y1);
				positions[1] = glm::vec2(x0, y0);
				positions[2] = glm::vec2(x1, y1);
				positions[3] = glm::vec2(x1, y0);

#if 0	// Font debug
				glUseProgram(0);

				glLineWidth(1.0);
				glBegin(GL_LINES);

				glColor3f(1, 0, 0);

				glVertex3f(x0, y1, -1.0f);	// Vertical left debug line
				glVertex3f(x0, y0, -1.0f);

				glVertex3f(x1, y1, -1.0f);	// Vertical right
				glVertex3f(x1, y0, -1.0f);

				glVertex3f(x0, y1, -1.0f);	// Horizontal bottom
				glVertex3f(x1, y1, -1.0f);

				glVertex3f(x0, y0, -1.0f);	// Horizontal top
				glVertex3f(x1, y0, -1.0f);
				glEnd();

				glUseProgram(shader_id);
#endif
				texCoords[0] = glm::vec2(glyph->s0, glyph->t1);
				texCoords[1] = glm::vec2(glyph->s0, glyph->t0);
				texCoords[2] = glm::vec2(glyph->s1, glyph->t1);
				texCoords[3] = glm::vec2(glyph->s1, glyph->t0);

				glBindVertexArray(vao);
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions) * sizeof(float) * 2, &positions[0]);

				glBindBuffer(GL_ARRAY_BUFFER, tbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(texCoords) * sizeof(float) * 2, &texCoords[0]);

				UpdateFontRenderer();

				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);

				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);

				x += glyph->advance_x / scaleX;
			}
		}

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glUseProgram(0);

		result.num_rows = yAdvance + 1;
		result.height = (font->ascender - font->descender) * (float)yAdvance;
		result.width = result.end_position.x - result.start_position.x;
		
#if 1	// Font debug
		glUseProgram(0);

		glLineWidth(1.0);

		glBegin(GL_LINES);

		glColor3f(1, 0, 0);

		glVertex3f(result.start_position.x, result.start_position.y, -1.0f);	// Vertical left debug line
		glVertex3f(result.end_position.x, result.end_position.y, -1.0f);

		glEnd();

		glUseProgram(shader_id);
#endif

		return result;
	}

	void UpdateFontRenderer()
	{
		glUniform4fv(font_color_uniform, 1, &font_color[0]);
	}

};