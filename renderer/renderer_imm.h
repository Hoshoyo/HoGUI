#pragma once
#include "../common.h"
#include <gm.h>

typedef struct Vertex_3D {
	vec3 position;
	r32  texture_alpha;
	vec2 texture_coordinate;
	vec4 color;
	r32  mask;
	vec4 clipping_box;
} Vertex_3D;

typedef struct {
	Vertex_3D vertices[4];
} Quad_2D;

typedef vec4 Clipping_Rect;

// Renderer Immediate functions
Quad_2D* renderer_imm_quad(Quad_2D* q);
void renderer_imm_border(Quad_2D* q, r32 border_width, vec4 color[4]);
void renderer_imm_flush(u32 font_id);
void renderer_imm_enable_blending();
void renderer_imm_disable_blending();
void renderer_immediate_global_position(vec3 p);

// Debug rendering functions
void renderer_imm_debug_box(r32 x, r32 y, r32 width, r32 height, vec4 color);
void renderer_imm_debug_line(vec2 start, vec2 end, vec4 color);

// Quad_2D functions
Quad_2D quad_new(vec2 position, r32 width, r32 height, vec4 color);
Quad_2D quad_new_clipped(vec2 position, r32 width, r32 height, vec4 color, Clipping_Rect rect);

// Clipping_Rect functions
Clipping_Rect clipping_rect_new(r32 x, r32 y, r32 width, r32 height);
Clipping_Rect clipping_rect_merge(Clipping_Rect a, Clipping_Rect b);