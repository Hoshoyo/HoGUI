#include "lexer.h"
#include "light_array.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	char* stream;
	s64   stream_offset;
	s64   line;
	s64   column;
} Tokenizer;

typedef enum {
	TOKEN_INVALID = -1,
	TOKEN_EOF = 0,
	TOKEN_INTEGER_DEC,
	TOKEN_INTEGER_HEX,
	TOKEN_INTEGER_BIN,
	TOKEN_FLOAT,
	TOKEN_STRING,
	TOKEN_BOOL_TRUE,
	TOKEN_BOOL_FALSE,
	TOKEN_SYMBOL,
	TOKEN_CHAR,
	TOKEN_IDENTIFIER,
} Token_Type;

typedef struct {
	Token_Type type;
	char*      data;
	s64        length;
	s32        line;
	s32        column;
} Token;

static
u8 hexdigit_to_u8(u8 d) {
	if (d >= 'A' && d <= 'F')
		return d - 'A' + 10;
	if (d >= 'a' && d <= 'f')
		return d - 'a' + 10;
	return d - '0';
}
static u8 
bindigit_to_u8(u8 d) {
	if (d == '1') return 1;
	return 0;
}

static bool
is_letter(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool
is_number(char c) {
	return c >= '0' && c <= '9';
}

static bool
is_hex_digit(char c) {
	return (is_number(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static bool 
string_equal_len(const char* s1, s64 s1_length, const char* s2, s64 s2_length) {
	if (s1_length != s2_length) {
		return false;
	}
	for(int i = 0; i < s1_length; ++i) {
		if (s1[i] != s2[i]) return false;
	}
	return true;
}

// unicode_next returns the unicode codepoint corresponding
// to the next unicode point in the stream 'text' encoded in utf8.
// 
// advance is an out parameter that contains the number of bytes in the codepoint
// length must be >= 1 and is the maximum length that the codepoint can have, 
// if the utf8 encoding encodes something bigger than length, than 0 is returned
// as the unicode codepoint, meaning an invalid utf8 encoding was found.
static u32 
unicode_next(u8* text, s32* advance, s32 length) {
	u32 result = 0;
	if (text[0] & 128) {
		// 1xxx xxxx
		if (text[0] >= 0xF0) {
			// 4 bytes
            if(length < 4) return 0;
			*advance = 4;
			result = ((text[0] & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
		} else if (text[0] >= 0xE0) {
			// 3 bytes
            if(length < 3) return 0;
			*advance = 3;
			result = ((text[0] & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
		} else if (text[0] >= 0xC0) {
			// 2 bytes
            if (length < 2) return 0;
			*advance = 2;
			result = ((text[0] & 0x1F) << 6) | (text[1] & 0x3F);
		} else {
			// continuation byte
			*advance = 1;
			result = text[0];
		}
	} else {
		*advance = 1;
		result = (u32)text[0];
	}
	return result;
}

static int
consume_whitespace(Tokenizer* tokenizer, int length) {
    char* at = tokenizer->stream + tokenizer->stream_offset;
    char* start = at;
    s32 advance = 0;

    while(length > 0) {
        switch(*at) {
            case 0: goto end_consume_ws;
            case ' ': case '\r': case '\v': case '\t': {
                length--;
                at++;
                tokenizer->column++;
            } break;
            case '\n': {
                length--;
                at++;
                tokenizer->line++;
                tokenizer->column = 0;
            }break;
            case '/': {
                // When this is the last character, exit
                if(length == 1) goto end_consume_ws;
                char c = at[1];
                if(c == '*') {
                    // multi line comment
                    at+= 2; // skip /*
                    length -= 2;
                    tokenizer->column += 2;
                    while(length > 0) {
                        c = *at;
                        if(c == '*' && length > 1 && at[1] == '/') {
                            // end of multi line comment
                            length -= 2;
                            at += 2;
                            tokenizer->column += 2;
                            break;
                        } else if(c == '\n') {
                            tokenizer->line++;
                            tokenizer->column = 0;
                            length--;
                            at++;
                        } else {
                            // any other character, just ignore and keep going
                            length--;
                            at++;
                            tokenizer->column++;
                        }
                    }
                } else if(c == '/') {
                    // single line comment
                    at += 2; // skip //
                    length -= 2;
                    tokenizer->column += 2;
                    while(length > 0) {
                        c = *at;
                        if(c == '\n') {
                            tokenizer->line++;
                            tokenizer->column = 0;
                            length--;
                            at++;
                            break;
                        } else {
                            length--;
                            at++;
                            tokenizer->column++;
                        }
                    }
                } else {
                    goto end_consume_ws;
                }
            } break;

            // Any other character is considered not whitespace, exit immediately.
            default: goto end_consume_ws;
        }
    }
    end_consume_ws:
    
    advance = (at - start);
    tokenizer->stream_offset += advance;
    return advance;
}

static r64
parse_float(Token t) {
	r64 result = 0.0f;
	r64 tenths = 1.0f;
	r64 frac_tenths = 0.1f;
	s64 point_index = 0;
	char* text = t.data;

	while (text[point_index] != '.') {
		point_index += 1;
	}

	for (s64 i = point_index - 1; i >= 0; i -= 1, tenths *= 10.0f) {
		result += (r64)(text[i] - 0x30) * tenths;
	}
	for (s64 i = point_index + 1; i < t.length; i += 1, frac_tenths *= 0.1f) {
		result += (r64)(text[i] - 0x30) * frac_tenths;
	}
	return result;
}

static s64
parse_int_dec(Token t) {
	s64 result = 0;
	s64 tenths = 1;
	for (s64 i = t.length - 1; i >= 0; --i, tenths *= 10)
		result += (t.data[i] - 0x30) * tenths;
	return result;
}

static u64
parse_int_hex(Token t) {
	u64 res = 0;
	u64 count = 0;
	for (s64 i = t.length - 1; i >= 0; --i, ++count) {
		if (t.data[i] == 'x') break;
		char c = hexdigit_to_u8(t.data[i]);
		res += (u64)c << (count * 4);
	}
	return res;
}

static u64
parse_int_bin(Token t) {
	u64 res = 0;
	for (s64 i = t.length - 1, count = 0; i >= 2 /* 0b */; --i, ++count) {
		char c = bindigit_to_u8(t.data[i]);
		res |= c << count;
	}
	return res;
}

static int
token_next(Tokenizer* tokenizer, int length, Token* token) {
    char* at = tokenizer->stream + tokenizer->stream_offset;
    char* start = at;

    token->data = at;
    token->line = (s32)tokenizer->line;
    token->column = (s32)tokenizer->column;

    {
        switch(*at) {
            case 0: {
                token->type = TOKEN_EOF;
                length--; at++;
            } break;
            case '"': {
                length--; at++; // skip "
                if(length == 0) {
                    token->type = TOKEN_SYMBOL;
                    break;
                }
                token->type = TOKEN_STRING;
                bool within_string = true;
                while(within_string && length > 0) {
                    switch(*at) {
                        case 0: {
                            // End of stream within a string
                            length--; at++;
                            within_string = false;
                        } break;
                        case '"': {
                            // WARNING End of string (normal)
                            length--; at++;
                            within_string = false;
                        } break;
                        case '\\': {
                            // Escaped sequence
                            length--; at++;
                            if(length == 0) {
                                // WARNING End of stream within string
                                length--; at++;
                                within_string = false;
                                break;
                            }
                            switch(*at) {
                                case 'a': case 'b': case 'f': case 'n': case 'r':
                                case 't': case 'v': case 'e': case '\\': case '\'':
                                case '"': case '?': {
                                    // Normal escape sequence of 1 character
                                    length--; at++;
                                } break;
                                case 'x': {
                                    // Only valid hex escape sequence in the form \xhh (h is a hex digit)
                                    if(length >= 3 && is_hex_digit(at[1]) && is_hex_digit(at[2])) 
                                    {
                                        length -= 3; at += 3; // skip xhh
                                    } // else -> Invalid escape sequence, consider '\' by itself
                                } break;
                                case 'u': {
                                    // Only valid escape sequence when in the format
                                    // \uhhhh (h is a hex digit)
                                    if(length >= 5 && is_hex_digit(at[1]) && is_hex_digit(at[2]) && 
                                        is_hex_digit(at[3]) && is_hex_digit(at[4])) 
                                    {
                                        length -= 5; at += 5; // skip uhhhh
                                    } // else -> Invalid escape sequence, consider '\' by itself
                                } break;
                                case 'U': {
                                    // Only valid escape sequence when in the format
                                    // \Uhhhhhhhh
                                    if(length >= 9 && is_hex_digit(at[1]) && is_hex_digit(at[2]) && 
                                        is_hex_digit(at[3]) && is_hex_digit(at[4]) && is_hex_digit(at[5]) && 
                                        is_hex_digit(at[6]) && is_hex_digit(at[7]) && is_hex_digit(at[8]))
                                    {
                                        length -= 9; at += 9; // skip Uhhhhhhhh
                                    } // else -> Invalid escape sequence, consider '\' by itself
                                } break;
                                default: {
                                    // Invalid escape sequence, consider '\' by itself
                                } break;
                            }
                        } break;
                        default: {
                            length--; at++;
                        } break;
                    }
                }
            } break;
            case '0': {
                length--; at++; // skip 0
                if(length > 0 && *at == 'x') {
                    // hex number
                    token->type = TOKEN_INTEGER_HEX;
                    length--; at++; // skip x
                    while(length > 0) {
                        if(is_hex_digit(*at)) {
                            length--; at++;
                        } else {
                            break;
                        }
                    }
                } else if(length > 0 && *at == 'b') {
                    // binary number
                    token->type = TOKEN_INTEGER_BIN;
                    length--; at++; // skip b
                    while(length > 0) {
                        char c = *at;
                        if(c == '0' || c == '1') {
                            length--; at++;
                        } else {
                            break;
                        }
                    }
                } else if(length > 0 && *at == '.') {
                    // float number
                    token->type = TOKEN_FLOAT;
                    length--; at++; // skip .
                    while(length > 0) {
                        if(is_number(*at)) {
                            length--; at++;
                        } else {
                            break;
                        }
                    }
                } else {
                    // 0
                    token->type = TOKEN_INTEGER_DEC;
                }
            } break;
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                bool is_float = false;
                while(length > 0) {
                    if(is_number(*at)) {
                        length--; at++;
                    } else if(*at == '.' && !is_float){
                        is_float = true;
                        length--; at++;
                    } else {
                        break;
                    }
                }
                if(is_float) token->type = TOKEN_FLOAT;
                else token->type = TOKEN_INTEGER_DEC;
            } break;
            case '_': case 'a': case 'b': case 'c': case 'd':
            case 'e': case 'f': case 'g': case 'h': case 'i':
            case 'j': case 'k': case 'l': case 'm': case 'n':
            case 'o': case 'p': case 'q': case 'r': case 's':
            case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z': case 'A': case 'B': case 'C':
            case 'D': case 'E': case 'F': case 'G': case 'H':
            case 'I': case 'J': case 'K': case 'L': case 'M':
            case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W':
            case 'X': case 'Y': case 'Z': {
                token->type = TOKEN_IDENTIFIER;
                while(length > 0) {
                    char c = *at;
                    if(is_letter(c) || is_number(c) || c == '_') {
                        length--; at++;
                    } else {
                        break;
                    }
                }
                int len = at - start;
                if(string_equal_len("true", sizeof("true") - 1, token->data, len)) {
                    token->type = TOKEN_BOOL_TRUE;
                } else if(string_equal_len("false", sizeof("false") - 1, token->data, len)) {
                    token->type = TOKEN_BOOL_FALSE;
                }
            } break;
            default: {
                s32 advance = 0;
                u32 unicode = unicode_next(at, &advance, length);
                if(unicode == 0) {
                    token->type = TOKEN_INVALID;
                    length = 0;
                } else {
                    token->type = TOKEN_SYMBOL;
                    length -= advance;
                    at += advance;
                }
            } break;
        }
    }

    token->length = (at - start);
    tokenizer->stream_offset += token->length;
	tokenizer->column += token->length;
	return token->length;
}

Lexer 
lexer_cstring(char* s, s32 length) {
	Tokenizer tokenizer = { 0 };
	tokenizer.stream = s;

	Lexer lexer = { 0 };
	lexer.tokens = array_new(Node);
	lexer.memory = arena_create((size_t)length * 2);

	while (length > 0) {
	    Token t = {0};
		Node node = { 0 };

		length -= consume_whitespace(&tokenizer, length);
        if(length == 0) break;
		length -= token_next(&tokenizer, length, &t);

		node.line = t.line;
		node.column = t.column;

		switch (t.type) {
		case TOKEN_EOF: break;

		case TOKEN_INTEGER_DEC: {
			node.type = NODE_LITERAL_INT;
			node.literal_int = parse_int_dec(t);
		} break;
		case TOKEN_INTEGER_BIN: {
			node.type = NODE_LITERAL_INT;
			node.literal_int = parse_int_bin(t);
		} break;
		case TOKEN_INTEGER_HEX: {
			node.type = NODE_LITERAL_INT;
			node.literal_int = parse_int_hex(t);
		} break;
		case TOKEN_FLOAT: {
			node.type = NODE_LITERAL_FLOAT;
			node.literal_float = parse_float(t);
		}break;
		case TOKEN_IDENTIFIER: {
			node.type = NODE_IDENTIFIER;
			node.identifier = arena_alloc(lexer.memory, t.length + 1);
			node.length = (s32)t.length;
			memcpy(node.identifier, t.data, t.length);
		} break;
		case TOKEN_BOOL_TRUE: {
			node.type = NODE_LITERAL_BOOL;
			node.literal_bool = true;
		}break;
		case TOKEN_BOOL_FALSE: {
			node.type = NODE_LITERAL_BOOL;
			node.literal_bool = false;
		}break;
		case TOKEN_STRING: {
			node.type = NODE_LITERAL_STRING;
			node.identifier = arena_alloc(lexer.memory, t.length + 1 - 2);
			node.length = (s32)t.length - 2;
			memcpy(node.identifier, t.data + 1, t.length - 2);
		}break;
		case TOKEN_SYMBOL: {
			node.type = NODE_SYMBOL;
			u32 adv = 0;
			node.symbol = unicode_next(t.data, &adv, 4);
		}break;
		}
		if (t.type == TOKEN_EOF) break;
		array_push(lexer.tokens, node);
	}

	lexer.raw = s;
	return lexer;
}

Node
lexer_next(Lexer* lexer) {
	if (lexer->index == array_length(lexer->tokens))
		return (Node) { 0 };

	lexer->index++;
	return lexer->tokens[lexer->index - 1];
}

Node
lexer_peek(Lexer* lexer) {
	return lexer->tokens[lexer->index];
}

void
lexer_free(Lexer* lexer) {
	arena_free(lexer->memory);
	array_free(lexer->tokens);
}

void
lexer_print_tokens(FILE* out, Lexer* lexer) {
    for(s32 i = 0; i < array_length(lexer->tokens); ++i) {
        Node t = lexer->tokens[i];
        
        fprintf(out, "[%3d : %3d:%3d] ", i, t.line + 1, t.column + 1);
        switch(t.type) {
            case NODE_EOF:              fprintf(out, "EOF"); break;
            case NODE_LITERAL_INT:      fprintf(out, "INT    %lld", t.literal_int); break;
            case NODE_LITERAL_FLOAT:    fprintf(out, "FLOAT  %f", t.literal_float); break;
            case NODE_LITERAL_STRING:   fprintf(out, "STRING %s", t.literal_string); break;
            case NODE_LITERAL_BOOL:     fprintf(out, "BOOL   %s", (t.literal_bool) ? "true": "false"); break;
            case NODE_SYMBOL:           fprintf(out, "SYMBOL %u (%c)", t.symbol, (char)t.symbol); break;
            case NODE_IDENTIFIER:       fprintf(out, "IDENT  %.*s", t.length, t.identifier); break;
            default: break;
        }
        fprintf(out, "\n");
    }
}