#include "renderer_imm.h"
#include "shader.h"
#include "../input.h"
#include <ho_gl.h>
#include <string.h>
#include <float.h>
#include <stdarg.h>
#include "../font/ustring.h"

typedef struct {
	u32 quad_vao;
	u32 quad_vbo;
	u32 quad_ebo;

	u32 shader;

	bool valid;
	bool prerendering;

	void* quads;
	s32 quad_count;
	s32 quad_max_count;

	u32 shader_projection_matrix_location;
	u32 shader_translation_matrix_location;
	u32 shader_text_location;

	vec3 global_position;

	s32 win_width;
	s32 win_height;
} ImmediateContext;

static ImmediateContext imm_ctx;

static bool
valid_glid(u32 id) {
	return (id >= 0 && id < (u32)-1);
}

void
renderer_imm_update(s32 window_width, s32 window_height) {
	imm_ctx.win_width = window_width;
	imm_ctx.win_height = window_height;
}

void renderer_immediate_global_position(vec3 p) {
	imm_ctx.global_position = p;
}

void
renderer_imm_enable_blending() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
renderer_imm_disable_blending() {
	glDisable(GL_BLEND);
}

static u32 immediate_shader_load() {
	u32 shader = shader_load("shaders/imm.vs", "shaders/imm.fs");
	assert(valid_glid(imm_ctx.shader));
	return shader;
}

static void
renderer_imm_init()
{
	// Start batch size
	const u32 batch_size = 1024 * 64;
	imm_ctx.quad_max_count = batch_size;

	// Shader setup
	imm_ctx.shader = immediate_shader_load();
	glUseProgram(imm_ctx.shader);

	imm_ctx.shader_projection_matrix_location = glGetUniformLocation(imm_ctx.shader, "u_projection");
	imm_ctx.shader_translation_matrix_location = glGetUniformLocation(imm_ctx.shader, "u_translation");
	imm_ctx.shader_text_location = glGetUniformLocation(imm_ctx.shader, "u_text");

	assert(valid_glid(imm_ctx.shader_projection_matrix_location));
	assert(valid_glid(imm_ctx.shader_translation_matrix_location));
	assert(valid_glid(imm_ctx.shader_text_location));

	// Buffers setup for rendering
	glGenVertexArrays(1, &imm_ctx.quad_vao);
	glBindVertexArray(imm_ctx.quad_vao);
	assert(valid_glid(imm_ctx.quad_vao));

	glGenBuffers(1, &imm_ctx.quad_vbo);
	assert(valid_glid(imm_ctx.quad_vbo));

	glBindBuffer(GL_ARRAY_BUFFER, imm_ctx.quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Quad_2D) * batch_size, 0, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), &((Vertex_3D*)0)->texture_coordinate);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), &((Vertex_3D*)0)->color);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), &((Vertex_3D*)0)->texture_alpha);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), &((Vertex_3D*)0)->mask);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex_3D), &((Vertex_3D*)0)->clipping_box);

	u32* indices = (u32*)calloc(1, batch_size * 6 * sizeof(u32));
	for (u32 i = 0, j = 0; i < batch_size * 6; i += 6, j += 4) {
		indices[i + 0] = j;
		indices[i + 1] = j + 1;
		indices[i + 2] = j + 2;
		indices[i + 3] = j + 2;
		indices[i + 4] = j + 1;
		indices[i + 5] = j + 3;
	}

	glGenBuffers(1, &imm_ctx.quad_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imm_ctx.quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u32) * batch_size, indices, GL_STATIC_DRAW);
	free(indices);

	imm_ctx.quad_count = 0;
	imm_ctx.valid = true;
}

Quad_2D* renderer_imm_quad(Quad_2D* quad) {
	if (!imm_ctx.valid) {
		renderer_imm_init();
		if (!imm_ctx.valid) {
			printf("Could not initialize immediate context rendering\n");
			return 0;
		}
	}

	if (imm_ctx.quad_count == imm_ctx.quad_max_count) {
		return 0;
	}

	if (!imm_ctx.prerendering) {
		imm_ctx.prerendering = true;
		glBindBuffer(GL_ARRAY_BUFFER, imm_ctx.quad_vbo);
		imm_ctx.quads = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		assert(imm_ctx.quads);
	}

	Quad_2D* quads = (Quad_2D*)imm_ctx.quads;

	Quad_2D* q = quads + imm_ctx.quad_count;
	assert(q);
	memcpy(q, quad, sizeof(Quad_2D));

	imm_ctx.quad_count++;

	return q;
}

void
renderer_imm_flush(u32 font_id)
{
	glUseProgram(imm_ctx.shader);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, font_id);
	glUniform1i(glGetUniformLocation(imm_ctx.shader, "u_text"), 12);

	imm_ctx.prerendering = false;
	imm_ctx.quads = 0;
	glBindBuffer(GL_ARRAY_BUFFER, imm_ctx.quad_vbo);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	GLenum error = glGetError();

	if (!imm_ctx.valid) return;

	s32 width, height;
	window_get_size(&width, &height);

	if (imm_ctx.quad_count > 0)
	{
		glUseProgram(imm_ctx.shader);
		glBindVertexArray(imm_ctx.quad_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imm_ctx.quad_ebo);
		glBindBuffer(GL_ARRAY_BUFFER, imm_ctx.quad_vbo);

		mat4 projection = gm_mat4_ortho(0, (r32)width, 0, (r32)height);
		glUniformMatrix4fv(imm_ctx.shader_projection_matrix_location, 1, GL_TRUE, (GLfloat*)projection.data);
		//mat4 translation = gm_mat4_translate(imm_ctx.global_position);
		//glUniformMatrix4fv(imm_ctx.shader_translation_matrix_location, 1, GL_TRUE, translation.data);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);

		glDrawElements(GL_TRIANGLES, 6 * imm_ctx.quad_count, GL_UNSIGNED_INT, 0);

		imm_ctx.quad_count = 0;
	}
}

void
renderer_imm_border(Quad_2D* q, r32 border_width[4], vec4 color[4]) {
	r32 width = q->vertices[1].position.x - q->vertices[0].position.x;
	r32 height = q->vertices[2].position.y - q->vertices[0].position.y;
	vec2 position = (vec2) { q->vertices[0].position.x, q->vertices[0].position.y };

	q->vertices[0].texture_alpha = 1.0f;
	q->vertices[1].texture_alpha = 1.0f;
	q->vertices[2].texture_alpha = 1.0f;
	q->vertices[3].texture_alpha = 1.0f;

	q->vertices[0].mask = 1.0f;
	q->vertices[1].mask = 1.0f;
	q->vertices[2].mask = 1.0f;
	q->vertices[3].mask = 1.0f;

	q->vertices[0].color = color[0];
	q->vertices[1].color = color[0];
	q->vertices[2].color = color[0];
	q->vertices[3].color = color[0];

	// left
	q->vertices[0].position = (vec3) { position.x, position.y, 0.0f };
	q->vertices[1].position = (vec3) { position.x + border_width[0], position.y + border_width[0], 0.0f };
	q->vertices[2].position = (vec3) { position.x, position.y + height, 0.0f };
	q->vertices[3].position = (vec3) { position.x + border_width[0], position.y + height - border_width[0], 0.0f };
	renderer_imm_quad(q);

	q->vertices[0].color = color[1];
	q->vertices[1].color = color[1];
	q->vertices[2].color = color[1];
	q->vertices[3].color = color[1];

	// right
	q->vertices[0].position = (vec3) { position.x + width - border_width[1], position.y + border_width[1], 0.0f };
	q->vertices[1].position = (vec3) { position.x + width, position.y, 0.0f };
	q->vertices[2].position = (vec3) { position.x + width - border_width[1], position.y + height - border_width[1], 0.0f };
	q->vertices[3].position = (vec3) { position.x + width, position.y + height, 0.0f };
	renderer_imm_quad(q);

	q->vertices[0].color = color[2];
	q->vertices[1].color = color[2];
	q->vertices[2].color = color[2];
	q->vertices[3].color = color[2];

	// bottom
	q->vertices[0].position = (vec3) { position.x, position.y, 0.0f };
	q->vertices[1].position = (vec3) { position.x + width, position.y, 0.0f };
	q->vertices[2].position = (vec3) { position.x + border_width[3], position.y + border_width[3], 0.0f };
	q->vertices[3].position = (vec3) { position.x + width - border_width[3], position.y + border_width[3], 0.0f };
	renderer_imm_quad(q);

	q->vertices[0].color = color[3];
	q->vertices[1].color = color[3];
	q->vertices[2].color = color[3];
	q->vertices[3].color = color[3];

	// top
	q->vertices[0].position = (vec3) { position.x + border_width[2], position.y + height - border_width[2], 0.0f };
	q->vertices[1].position = (vec3) { position.x + width - border_width[2], position.y + height - border_width[2], 0.0f };
	q->vertices[2].position = (vec3) { position.x, position.y + height, 0.0f };
	q->vertices[3].position = (vec3) { position.x + width, position.y + height, 0.0f };
	renderer_imm_quad(q);
}

Quad_2D
quad_new_clipped(vec2 position, r32 width, r32 height, vec4 color, Clipping_Rect rect) {
	Quad_2D q =
	{
		(Vertex_3D) { 
			(vec3) { position.x, position.y, 0 }, 1.0f, (vec2) { 0.0f, 0.0f }, color, 1.0f, rect
		},
		(Vertex_3D) {
			(vec3) { position.x + width, position.y, 0}, 1.0f, (vec2) { 1.0f, 0.0f }, color, 1.0f, rect
		},
		(Vertex_3D) {
			(vec3) { position.x, position.y + height, 0 }, 1.0f, (vec2) { 0.0f, 1.0f }, color, 1.0f, rect
		},
		(Vertex_3D) {
			(vec3) { position.x + width, position.y + height, 0 }, 1.0f, (vec2) { 1.0f, 1.0f }, color, 1.0f, rect
		}
	};
	return q;
}

Quad_2D
quad_new(vec2 position, r32 width, r32 height, vec4 color) {
	vec4 clipping = (vec4) { 0, 0, FLT_MAX, FLT_MAX };

	return quad_new_clipped(position, width, height, color, clipping);
}

void
renderer_imm_debug_box(r32 x, r32 y, r32 width, r32 height, vec4 color) {
	Quad_2D q = quad_new((vec2) { x, y }, width, height, color);
	vec4 bcolor[4];
	bcolor[0] = color;
	bcolor[1] = color;
	bcolor[2] = color;
	bcolor[3] = color;
	renderer_imm_border(&q, (r32[4]){1.0f, 1.0f, 1.0f, 1.0f}, bcolor);
}

void
renderer_imm_debug_line(vec2 start, vec2 end, vec4 color) {
	vec4 clipping = (vec4) { 0, 0, FLT_MAX, FLT_MAX };
	Quad_2D q =
	{
		(Vertex_3D) { (vec3) { start.x, start.y, 0 }, 1.0f, (vec2) { 0.0f, 0.0f }, color, 1.0f, clipping
	},
		(Vertex_3D) { (vec3) { start.x + 1.0f, start.y + 1.0f, 0 }, 1.0f, (vec2) { 1.0f, 0.0f }, color, 1.0f, clipping
	},
		(Vertex_3D) { (vec3) { end.x, end.y, 0 }, 1.0f, (vec2) { 0.0f, 1.0f }, color, 1.0f, clipping
	},
		(Vertex_3D) { (vec3) { end.x + 1.0f,   end.y + 1.0f, 0 }, 1.0f, (vec2) { 1.0f, 1.0f }, color, 1.0f, clipping
	}
	};
	renderer_imm_quad(&q);
}

Clipping_Rect
clipping_rect_new(r32 x, r32 y, r32 width, r32 height) {
	return (Clipping_Rect) { x, y, width, height };
}

Clipping_Rect
clipping_rect_merge(Clipping_Rect a, Clipping_Rect b) {
	Clipping_Rect result = {0};

	result.x = MAX(a.x, b.x);	// x
	result.y = MAX(a.y, b.y);	// y
	result.z = MIN(a.z - (result.x - a.x), b.z - (result.x - b.x));
	result.w = MIN(a.w - (result.y - a.y), b.w - (result.y - b.y));

	return result;
}

int 
renderer_imm_debug_text(Font_Info* font_info, vec2 position, char* fmt, ...) {
	char buffer[1024] = {0};
	va_list args;
	va_start(args, fmt);
	int num_written = vsprintf(buffer, fmt, args);
	va_end(args);

	ustring s = ustring_new_utf8(buffer);

	FRII ii = {
        .flags = FONT_RENDER_INFO_DO_RENDER,
        .bb = (BBox){0.0f, 0.0f, FLT_MAX, FLT_MAX},
        .position = position,
        .color = (vec4){1.0f, 1.0f, 1.0f, 1.0f},
        .tab_space = 3,
    };

    font_render_text(font_info, &ii, s);
    ustring_free(&s);

	return num_written;
}