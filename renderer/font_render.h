#pragma once
#include <gm.h>
#include "../common.h"
#include "font_load.h"
#include "../ustring.h"

typedef struct {
	r32 x, y, width, height;
} BBox;

typedef enum {
	FONT_RENDER_INFO_DO_RENDER = FLAG(0),
	FONT_RENDER_INFO_BOUNDED   = FLAG(1),
	FONT_RENDER_INFO_SELECTING = FLAG(2),
} Font_Render_Info_Flag;

typedef struct {
	Font_Render_Info_Flag flags;
	vec2 position;
	BBox bb;
	vec4 color;
	s32  tab_space;

	// cursor
	s32      cursor_index;
	s32      start_selection_index;

	// selection
	vec2 selected_position;
} FRII;

typedef struct {
	s32 count;

	r32 max_x;
	r32 max_y;

	// positions
	vec2 final_position;

	// cursor position
	vec2 cursor_position;
	vec2 start_selection_position;

	// selection
	s32 selection_index;
} FRIO;

FRIO font_render_text(Font_Info* font_info, FRII* info, ustring str);
vec4 font_render_no_clipping();

typedef struct {
	int  index;
	vec2 position;
	int width;
	int height;
} Text_Render_Character_Position;

typedef struct {
	int  line_count;
	int  max_column_count;
	r32  width;
	r32  height;
} Text_Render_Info;

Text_Render_Info text_prerender(Font_Info* font_info, const char* text, int length, int start_index, Text_Render_Character_Position* out_positions, int positions_count);
int text_render(Font_Info* font_info, const char* text, int length, int start_index, vec2 position, vec4 clipping, vec4 color);
int text_render_debug(Font_Info* font_info, const char* text, int index, ...);