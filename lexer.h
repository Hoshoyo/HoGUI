#pragma once
#include "../common.h"
#include "light_arena.h"

typedef enum {
	NODE_EOF = 0,
	NODE_LITERAL_INT,
	NODE_LITERAL_FLOAT,
	NODE_LITERAL_STRING,
	NODE_LITERAL_BOOL,
	NODE_SYMBOL,
	NODE_IDENTIFIER,
} Node_Type;

typedef struct {
	Node_Type type;
	union {
		s64   literal_int;
		r64   literal_float;
		char* literal_string;
		bool  literal_bool;
		u32   symbol;
		char* identifier;
	};
	s32 line;
	s32 column;
	s32 length;
} Node;

typedef struct {
	Node*        tokens;
	Light_Arena* memory;
	char*        raw;
	s32          index;
} Lexer;

Lexer lexer_cstring(char* s, s32 length);
void  lexer_free(Lexer* lexer);
Node  lexer_next(Lexer* lexer);
Node  lexer_peek(Lexer* lexer);