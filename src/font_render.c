#include "font_render.h"
#include <ho_gl.h>
#include <assert.h>
#include <float.h>
#include "batcher.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

static u32
ustring_get_unicode(u8* text, u32* advance)
{
  u32 result = 0;
  if (text[0] & 128)
  {
    // 1xxx xxxx
    if (text[0] >= 0xF0)
    {
      // 4 bytes
      *advance = 4;
      result = ((text[0] & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
    }
    else if (text[0] >= 0xE0)
    {
      // 3 bytes
      *advance = 3;
      result = ((text[0] & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
    }
    else if (text[0] >= 0xC0)
    {
      // 2 bytes
      *advance = 2;
      result = ((text[0] & 0x1F) << 6) | (text[1] & 0x3F);
    }
    else
    {
      // continuation byte
      *advance = 1;
      result = text[0];
    }
  }
  else
  {
    *advance = 1;
    result = (u32) text[0];
  }
  return result;
}

Text_Render_Info
text_prerender(Font_Info* font_info, const char* text, int length, 
  int start_index, Text_Render_Character_Position* out_positions, int positions_count)
{
  Text_Render_Info result = { 0 };
  result.line_count = 1;

  Character* characters = font_info->characters;

  int idx = 0;
  r32 extra_height = 0.0f;
  r32 min_height = FLT_MAX;
  vec2 max_position = { 0 };
  vec2 position = (vec2) { 0.0f, 0.0f };
  vec4 clipping = (vec4) { 0, 0, FLT_MAX, FLT_MAX };

  int selection_count = 0;

  for (s32 i = 0, c = 0, columns = 0; c < length; ++i)
  {
    u32 advance = 0;
    u32 unicode = ustring_get_unicode((u8 *) text + ((idx + start_index) % length), &advance);
    idx += advance;

    s32 repeat = 1;
    c += (s32) advance;

    Text_Render_Character_Position *fill_indexed_position = 0;
    if (out_positions && selection_count < positions_count && out_positions[selection_count].index == (idx - advance))
    {
      fill_indexed_position = out_positions + selection_count;
      selection_count++;
    }
    else
    {
      fill_indexed_position = 0;
    }

    bool new_line = false;

    if (unicode == '\t')
    {
      unicode = ' ';
      repeat = 3;
    }
    else if (!characters[unicode].renderable)
    {
      unicode = ' ';
      new_line = true;
    }

    for (int r = 0; r < repeat; ++r)
    {
      GLfloat xpos = position.x + characters[unicode].bearing[0];
      GLfloat ypos = position.y - (characters[unicode].size[1] - characters[unicode].bearing[1]);
      GLfloat w = (GLfloat) characters[unicode].size[0];
      GLfloat h = (GLfloat) characters[unicode].size[1];

      if (fill_indexed_position)
      {
        if (r == 0)
        {
          fill_indexed_position->position.x = xpos;
          fill_indexed_position->position.y = ypos;
        }
        if (r == repeat - 1)
        {
          fill_indexed_position->width = xpos + w - fill_indexed_position->position.x;
          fill_indexed_position->height = ypos + h - fill_indexed_position->position.y;
#if 0
          renderer_imm_debug_box(fill_indexed_position->position.x,
                                 fill_indexed_position->position.y,
                                 fill_indexed_position->width, fill_indexed_position->height, (vec4) {
                                 1.0f, 1.0f, 0.0f, 1.0f}
          );
#endif
        }
      }
#if 0
      if (unicode != '\n' && unicode != '\t') {
        vec4 color = (vec4) { 1.0f, 1.0f, 1.0f, 1.0f };
        Quad_2D q = {
          (Vertex_3D) {(vec3) {roundf(xpos), roundf(ypos + h), 0.0f}, 1.0f, characters[unicode].botl, color, 0.0f,
                       clipping},
          (Vertex_3D) {(vec3) {roundf(xpos + w), roundf(ypos + h), 0.0f}, 1.0f, characters[unicode].botr, color, 0.0f,
                       clipping},
          (Vertex_3D) {(vec3) {roundf(xpos), roundf(ypos), 0.0f}, 1.0f, characters[unicode].topl, color, 0.0f,
                       clipping},
          (Vertex_3D) {(vec3) {roundf(xpos + w), roundf(ypos), 0.0f}, 1.0f, characters[unicode].topr, color, 0.0f,
                       clipping}
        };

        renderer_imm_quad(&q);
      }
#endif
      r32 advance = (r32) (characters[unicode].advance >> 6);
      if (xpos + advance > max_position.x)
      {
        max_position.x = xpos + advance;
      }
      if (ypos + h > max_position.y)
      {
        max_position.y = ypos + h;
      }
      if (ypos < min_height)
      {
        min_height = ypos;
      }

      if (new_line)
      {
        if (result.max_column_count > columns)
        {
          result.max_column_count = columns;
        }
        position.y -= font_info->font_size;
        position.x = 0.0f;
        result.line_count++;
      }
      else
      {
        position.x += advance;
      }
    }
  }
  result.width = max_position.x;
  result.height = MAX(0.0f, max_position.y - min_height);

  if (out_positions && selection_count < positions_count && out_positions[selection_count].index == idx)
  {
    Text_Render_Character_Position *fill_indexed_position = out_positions + selection_count;
    fill_indexed_position->position.x = position.x;
    fill_indexed_position->position.y = position.y;
  }

  return result;
}

static u8 
hexdigit_to_u8(u8 d) {
    if (d >= 'A' && d <= 'F')
        return d - 'A' + 10;
    if (d >= 'a' && d <= 'f')
        return d - 'a' + 10;
    return d - '0';
}
u32
str_hex_to_u64(char* text, int length) {
    u32 res = 0;
    u32 count = 0;
    for (s32 i = length - 1; i >= 0; --i, ++count) {
		if (text[i] == 'x') break;
		char c = hexdigit_to_u8(text[i]);
		res += (u32)c << (count * 4);
    }
    return res;
}

int
text_render(Hobatch_Context* ctx, Font_Info* font_info, const char* text, int length, int start_index,
  vec2 position, vec4 clipping, vec4 color)
{
  Text_Render_Info result = { 0 };
  Character* characters = font_info->characters;

  int idx = 0;
  r32 extra_height = 0.0f;
  vec2 max_position = { 0 };
  vec2 start_position = position;

  int selection_count = 0;
  bool escaping = false;

  for (s32 i = 0, c = 0, columns = 0; c < length; ++i)
  {
    u32 advance = 0;
    u32 unicode = ustring_get_unicode((u8 *) text + ((idx + start_index) % length), &advance);
    idx += advance;

    s32 repeat = 1;
    c += (s32) advance;

    bool new_line = false;

    if(unicode == '\\' && !escaping)
    {
      escaping = true;
      continue;
    }

    if(unicode == '#')
    {
      if(!escaping)
      {
        // parse color
        u8* at = text + ((idx + start_index) % length);
        at += 1;
        u32 cc = str_hex_to_u64(at, 8);
        color = batch_render_color_from_hex(cc);
        idx += 8;
        c += 8;
        continue;
      }
    }

    if(escaping) escaping = false;

    if (unicode == '\t')
    {
      unicode = ' ';
      repeat = 3;
    }
    else if (!characters[unicode].renderable)
    {
      unicode = ' ';
      new_line = true;
    }

    for (int r = 0; r < repeat; ++r)
    {
      GLfloat xpos = position.x + characters[unicode].bearing[0];
      GLfloat ypos = position.y - (characters[unicode].size[1] - characters[unicode].bearing[1]);
      GLfloat w = (GLfloat) characters[unicode].size[0];
      GLfloat h = (GLfloat) characters[unicode].size[1];

#if 1
      if (unicode != '\n' && unicode != '\t')
      {
        r32 blend_factor[4] = {1,1,1,1};
        vec4 color_arr[4] = { color, color, color, color };
        vec2 texcoords[4] = 
        {
          characters[unicode].topl,
          characters[unicode].topr,
          characters[unicode].botl,
          characters[unicode].botr,
        };
        batch_render_quad(ctx, (vec3){xpos, ypos}, w, h, font_info->atlas_full_id,
          clipping, blend_factor, color_arr, texcoords, 1.0f);

      }
#endif
      if (new_line)
      {
        position.y -= font_info->font_size;
        position.x = start_position.x;
      }
      else
      {
        position.x += characters[unicode].advance >> 6;
      }
    }
  }

  return 0;
}