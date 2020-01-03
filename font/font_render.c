#include "font_render.h"
#include <ho_gl.h>
#include <assert.h>
#include <float.h>
#include "../renderer/renderer_imm.h"

static bool
inside(vec2 p, r32 x, r32 y, r32 width, r32 height) {
	return (p.x >= x && p.x <= x + width && p.y >= y && p.y <= y + height);
}

static bool
inside_x(vec2 p, r32 x, r32 width) {
	return (p.x >= x && p.x <= x + width);
}

static bool
inside_y(vec2 p, r32 y, r32 height) {
	return (p.y >= y && p.y <= y + height);
}

static r32 
closest_distance_x(vec2 p, r32 x, r32 width) {
	r32 dx = MAX(fabsf(p.x - x) - width / 2.0f, 0.0f);
	return dx * dx;
}
static r32
closest_distance_y(vec2 p, r32 y, r32 height) {
	r32 dy = MAX(fabsf(p.y - y) - height / 2.0f, 0.0f);
	return dy * dy;
}
static r32
closest_distance(vec2 p, r32 x, r32 y, r32 width, r32 height) {
	r32 dx = MAX(fabsf(p.x - x) - width / 2.0f, 0.0f);
	r32 dy = MAX(fabsf(p.y - y) - height / 2.0f, 0.0f);
	return dx * dx + dy * dy;
}

FRIO 
font_render_text(Font_Info* font_info, FRII* info, ustring str) {
	FRIO result = { 0 };

	Character* characters = font_info->characters;
	vec2 start_pos = info->position;

	r32 limit_x = start_pos.x + info->bb.width;
	r32 limit_y = start_pos.y + info->bb.height;
	result.max_x = start_pos.x;
	result.max_y = info->position.y;

	r32 extra_height = 0.0f;

	r32  distance_min_x = FLT_MAX;
	r32  distance_min_x_same_y = FLT_MAX;
	r32  distance_min_y = FLT_MAX;
	r32  distance_min = FLT_MAX;
	s32  index_distance_min_x = -1;
	s32  index_distance_min_x_same_y = -1;
	s32  index_distance_min_y = -1;
	s32  index_distance_min = -1;
	s32  index_inside = -1;
	BBox bb_distance_min_x = { 0 };
	BBox bb_distance_min_x_same_y = { 0 };
	BBox bb_distance_min_y = { 0 };
	BBox bb_distance_min = { 0 };

	bool selected = false;

	Clipping_Rect clipping = (Clipping_Rect) { info->bb.x, info->bb.y, info->bb.width, info->bb.height };
	//Clipping_Rect clipping = (Clipping_Rect) { 0, 0, FLT_MAX, FLT_MAX };

	for (s32 i = 0, c = 0; c < str.length; ++i) {
		u32 index = str.data[c];
		s32 repeat = 1;
		c += 1;

		bool new_line = false;

		if (info->start_selection_index < info->cursor_index) {
			selected = (i >= info->start_selection_index && i < info->cursor_index);
		} else {
			selected = (i >= info->cursor_index && i < info->start_selection_index);
		}

		if (index == '\t') {
			index = ' ';
			repeat = info->tab_space;
		} else if (!characters[index].renderable) {
			index = ' ';
			new_line = true;
		}

		BBox rect = { 0 };

		for (int r = 0; r < repeat; ++r)
		{
			GLfloat xpos = (GLfloat)info->position.x + characters[index].bearing[0];
			GLfloat ypos = (GLfloat)info->position.y - (characters[index].size[1] - characters[index].bearing[1]) + extra_height;

			if (r == 0) {
				rect.x = info->position.x;
				rect.y = info->position.y + extra_height - 3.0f;
			}

			GLfloat w = (GLfloat)characters[index].size[0];
			GLfloat h = (GLfloat)characters[index].size[1];

			rect.width += (r32)font_info->max_width;
			rect.height = (r32)font_info->max_height;

			// TODO(psv): activate debug rendering through console
			if (selected && info->flags & FONT_RENDER_INFO_SELECTING) {
				Quad_2D q = quad_new_clipped((vec2) { rect.x, rect.y }, rect.width, rect.height, (vec4) { 0.4f, 0.48f, 0.51f, 0.75f }, clipping);
				renderer_imm_quad(&q);
			}
#if 0
			renderer_imm_debug_box(rect.x, rect.y, rect.width, rect.height, (vec4) { 1.0f, 1.0f, 0.0f, 0.5f });
#endif

			if (info->cursor_index == result.count && r == 0) {
				result.cursor_position = (vec2) { xpos, info->position.y + extra_height };
			}
			if (info->start_selection_index == result.count) {
				result.start_selection_position = (vec2) { xpos, info->position.y + extra_height };
			}

			if (h > result.max_y) result.max_y = h;

			if (index != '\n' && index != '\t') {
				Quad_2D q = {
					(Vertex_3D) { (vec3) {roundf(xpos),     roundf(ypos + h), 0.0f}, 1.0f, characters[index].botl, info->color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos + w), roundf(ypos + h), 0.0f}, 1.0f, characters[index].botr, info->color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos),     roundf(ypos),    0.0f }, 1.0f, characters[index].topl, info->color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos + w), roundf(ypos),    0.0f }, 1.0f, characters[index].topr, info->color, 0.0f, clipping }
				};

				if (info->flags & FONT_RENDER_INFO_DO_RENDER) {
					renderer_imm_quad(&q);
				}
			}

			if (xpos + w > result.max_x) {
				result.max_x = xpos + w;
			}
			result.max_y = ypos;

			if (new_line) {
				extra_height -= font_info->font_size;
				info->position.x = start_pos.x;
				result.max_y = ypos - font_info->font_size;
				//if (ypos < start_pos.y - info->bb.height + font_info->font_size * 2) goto end_render;
			} else {
				info->position.x += characters[index].advance >> 6;
			}
		}

		// distance detection

		if (inside(info->selected_position, rect.x, rect.y, rect.width, rect.height)) {
			index_inside = result.count;
		} else {
			r32 cdistance_x = closest_distance_x(info->selected_position, rect.x, rect.width);
			r32 cdistance_y = closest_distance_y(info->selected_position, rect.y, rect.height);
			r32 cdistance = closest_distance(info->selected_position, rect.x, rect.y, rect.width, rect.height);

			if (cdistance < distance_min) {
				distance_min = cdistance;
				index_distance_min = result.count;
				bb_distance_min = rect;
			}

			if (cdistance_x < distance_min_x) {
				distance_min_x = cdistance_x;
				index_distance_min_x = result.count;
				bb_distance_min_x = rect;
			}

			if (cdistance_y < distance_min_y) {
				distance_min_y = cdistance_y;
				index_distance_min_y = result.count;
				bb_distance_min_y = rect;
			}

			if (cdistance_y == 0.0f) {
				if (cdistance_x < distance_min_x_same_y) {
					distance_min_x_same_y = cdistance_x;
					index_distance_min_x_same_y = result.count;
					bb_distance_min_x_same_y = rect;
				}
			}
		}
		result.count += 1;
	}

	if (info->cursor_index == result.count) {
		result.cursor_position = (vec2){ info->position.x, info->position.y + extra_height };
	}
	if (info->start_selection_index == result.count) {
		result.start_selection_position = (vec2){ info->position.x, info->position.y + extra_height };
	}

	if (index_inside != -1) {
		result.selection_index = index_inside;
	} else if (index_distance_min_x_same_y != -1) {
		result.selection_index = index_distance_min_x_same_y;
	} else {
		result.selection_index = result.count;
	}
	
	result.final_position = info->position;

	return result;
}

Text_Render_Info
text_prerender(Font_Info* font_info, const char* text, int length, Text_Render_Character_Position* out_positions, int positions_count) {
	Text_Render_Info result = {0};
	result.line_count = 1;

	Character* characters = font_info->characters;

	int idx = 0;
	r32 extra_height = 0.0f;
	vec2 max_position = { 0 };
	vec2 position = (vec2){0.0f, 0.0f};
	Clipping_Rect clipping = (Clipping_Rect) { 0, 0, FLT_MAX, FLT_MAX };

	int selection_count = 0;

	for (s32 i = 0, c = 0, columns = 0; c < length; ++i) {
		u32 advance = 0;
		u32 unicode = ustring_get_unicode((u8*)text + idx, &advance);
		idx += advance;

		s32 repeat = 1;
		c += 1;

		Text_Render_Character_Position* fill_indexed_position = 0;
		if(out_positions && selection_count < positions_count && out_positions[selection_count].index == i) {
			fill_indexed_position = out_positions + selection_count;
			selection_count++;
		} else {
			fill_indexed_position = 0;
		}

		bool new_line = false;

		if (unicode == '\t') {
			unicode = ' ';
			repeat = 3;
		} else if (!characters[unicode].renderable) {
			unicode = ' ';
			new_line = true;
		}

		for (int r = 0; r < repeat; ++r)
		{
			GLfloat xpos = position.x + characters[unicode].bearing[0];
			GLfloat ypos = position.y -(characters[unicode].size[1] - characters[unicode].bearing[1]);
			GLfloat w = (GLfloat)characters[unicode].size[0];
			GLfloat h = (GLfloat)characters[unicode].size[1];

			if(fill_indexed_position) {
				if(r == 0) {
					fill_indexed_position->position.x = xpos;
					fill_indexed_position->position.y = ypos;
				}
				if(r == repeat - 1) {
					fill_indexed_position->width = xpos + w - fill_indexed_position->position.x;
					fill_indexed_position->height = ypos + h - fill_indexed_position->position.y;
#if 0
					renderer_imm_debug_box(
						fill_indexed_position->position.x, 
						fill_indexed_position->position.y, 
						fill_indexed_position->width, 
						fill_indexed_position->height, 
						(vec4) { 1.0f, 1.0f, 0.0f, 1.0f });
#endif
				}
			}

#if 0
			if (unicode != '\n' && unicode != '\t') {
				vec4 color = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
				Quad_2D q = {
					(Vertex_3D) { (vec3) {roundf(xpos),     roundf(ypos + h), 0.0f}, 1.0f, characters[unicode].botl, color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos + w), roundf(ypos + h), 0.0f}, 1.0f, characters[unicode].botr, color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos),     roundf(ypos),    0.0f }, 1.0f, characters[unicode].topl, color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos + w), roundf(ypos),    0.0f }, 1.0f, characters[unicode].topr, color, 0.0f, clipping }
				};

				renderer_imm_quad(&q);
			}
#endif
			r32 advance = (r32)(characters[unicode].advance >> 6);
			if (xpos + advance > max_position.x) {
				max_position.x = xpos + advance;
			}
			if (ypos + h > max_position.y) {
				max_position.y = ypos + h;
			}

			if (new_line) {
				if(result.max_column_count > columns) {
					result.max_column_count = columns;
				}
				position.y -= font_info->font_size;
				position.x = 0.0f;
				result.line_count++;
			} else {
				position.x += advance;
			}
		}
	}
	result.width = max_position.x;
	result.height = max_position.y;

	return result;
}

int
text_render(Font_Info* font_info, const char* text, int length, vec2 position, Clipping_Rect clipping) {
	Text_Render_Info result = {0};
	Character* characters = font_info->characters;

	int idx = 0;
	r32 extra_height = 0.0f;
	vec2 max_position = { 0 };

	int selection_count = 0;

	for (s32 i = 0, c = 0, columns = 0; c < length; ++i) {
		u32 advance = 0;
		u32 unicode = ustring_get_unicode((u8*)text + idx, &advance);
		idx += advance;

		s32 repeat = 1;
		c += (s32)advance;

		bool new_line = false;

		if (unicode == '\t') {
			unicode = ' ';
			repeat = 3;
		} else if (!characters[unicode].renderable) {
			unicode = ' ';
			new_line = true;
		}

		for (int r = 0; r < repeat; ++r)
		{
			GLfloat xpos = position.x + characters[unicode].bearing[0];
			GLfloat ypos = position.y -(characters[unicode].size[1] - characters[unicode].bearing[1]);
			GLfloat w = (GLfloat)characters[unicode].size[0];
			GLfloat h = (GLfloat)characters[unicode].size[1];

#if 1
			if (unicode != '\n' && unicode != '\t') {
				vec4 color = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
				Quad_2D q = {
					(Vertex_3D) { (vec3) {roundf(xpos),     roundf(ypos + h), 0.0f}, 1.0f, characters[unicode].botl, color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos + w), roundf(ypos + h), 0.0f}, 1.0f, characters[unicode].botr, color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos),     roundf(ypos),    0.0f }, 1.0f, characters[unicode].topl, color, 0.0f, clipping },
					(Vertex_3D) { (vec3) {roundf(xpos + w), roundf(ypos),    0.0f }, 1.0f, characters[unicode].topr, color, 0.0f, clipping }
				};

				renderer_imm_quad(&q);
			}
#endif
			if (new_line) {
				position.y -= font_info->font_size;
				position.x = 0.0f;
			} else {
				position.x += characters[unicode].advance >> 6;
			}
		}
	}

	return 0;
}

Clipping_Rect font_render_no_clipping() {
	return (Clipping_Rect) { 0, 0, FLT_MAX, FLT_MAX };
}