#ifndef _LEXER_H
#define _LEXER_H

#include <stddef.h>

typedef enum {
	tok_eof = 0,
	tok_unknown,
	tok_identifier,
	tok_number,
	tok_plus,
	tok_minus,
	tok_lparen,
	tok_rparen,
	tok_if,
	tok_else,
	tok_while,
	tok_for,
	tok_return
} token_t;

typedef struct {
	char* name;
	char* src;
	size_t size;
} lexer_file_t;

typedef struct {
	token_t type;
	char* lexeme;
	struct {
		size_t line;
		size_t start;
		size_t end;
		lexer_file_t* file;
	} meta;
} token_info_t;

typedef struct {
	token_info_t* tokens;
	size_t tokens_count;
	size_t tokens_allocated;
	size_t lineno;
	size_t col_offset;
	size_t expos;
	lexer_file_t* file;
} lexer_t;

void lexify(lexer_t* lex);
lexer_t* lex_file(char* path); 

#endif // _LEXER_H
