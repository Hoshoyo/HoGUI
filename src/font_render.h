#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H 

#include <glm/glm.hpp>

#include "graphics/shader.h"
#include "platform/platform.h"

#define NUMBER_CHARACTERS 1024

struct FontInfo
{
	int max_height;
	int max_width;
	int has_kerning;
};

struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLint      Advance;    // Offset to advance to next glyph
};

Character Characters[NUMBER_CHARACTERS];

GLuint font_vao, font_vbo;

void LoadFreetype(char* font, int pixel_height, FontInfo* info)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		PLATFORM_ERROR("Could not init FreeType Library", "Freetype Error");

	FT_Face face;
	if (FT_New_Face(ft, font, 0, &face))
		PLATFORM_ERROR("Failed to load font", "Freetype Error");

	FT_Set_Pixel_Sizes(face, 0, pixel_height);

	if (info != 0)
	{
		// Fill with font information
		info->max_height = face->max_advance_height;
		info->max_width = face->max_advance_width;
		info->has_kerning = FT_HAS_KERNING(face);
	}

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned short c = 0;
	for (unsigned short i = 0; i < NUMBER_CHARACTERS; i++, c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			PLATFORM_ERROR("Failed to load Glyph", "Freetype Error");
			continue;
		}

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters[c] = character;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	
	glGenVertexArrays(1, &font_vao);
	glGenBuffers(1, &font_vbo);
	glBindVertexArray(font_vao);
	glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void RenderText(FontShader* shader, char* text, GLfloat x, GLfloat y, GLfloat scale)
{
	glUseProgram(shader->id);
	glActiveTexture(GL_TEXTURE0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray(font_vao);

	for(unsigned char c = text[0], i = 0; text[i] != '\0'; ++i, c = text[i])
	{
		Character ch = Characters[c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;

		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};

		glUniformMatrix4fv(shader->uni_projection, 1, GL_FALSE, &shader->projection_matrix[0][0]);

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		glBindBuffer(GL_ARRAY_BUFFER, font_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindVertexArray(0);

	glDisable(GL_BLEND);
}