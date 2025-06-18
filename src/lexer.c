#include <lexer.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_TOKENS_CAPACITY 32

typedef struct {
	const char *keyword;
	token_t token_type;
} keyword_t;

static const keyword_t keywords[] = {
	{"if", tok_if},
	{"else", tok_else},
	{"while", tok_while},
	{"for", tok_for},
	{"return", tok_return},
};

token_info_t* lex_token(lexer_t* lex, token_t type, char* lexeme) {
	if (lex->tokens_count >= lex->tokens_allocated) {
		size_t new_alloc = lex->tokens_allocated * 2;
		token_info_t* rl_tokens = realloc(
			lex->tokens,
			new_alloc * sizeof(token_info_t)
		);
		if (!rl_tokens) return NULL;

		lex->tokens = rl_tokens;
		lex->tokens_allocated = new_alloc;
	}

	size_t len = strlen(lexeme);
	lex->tokens[lex->tokens_count] = (token_info_t){
		.type = type,
		.lexeme = lexeme,
		.meta = {
			.line = lex->lineno,
			.start = lex->col_offset,
			.end = lex->col_offset + len,
			.file = lex->file
		}
	};
	lex->tokens_count++;
	lex->col_offset += len;

	return &lex->tokens[lex->tokens_count - 1];
}

char lex_peek(lexer_t* lex) {
	lexer_file_t* file = lex->file;
	if (lex->expos >= file->size) {
		return '\0';
	}
	return file->src[lex->expos];
}

char lex_next(lexer_t* lex) {
	lex->expos++;
	char c = lex_peek(lex);
	
	if (c == '\n') {
		lex->lineno++;
		lex->col_offset = 0;
	} else {
		lex->col_offset++;
	}
	
	return c;
}

static token_info_t* lex_number(lexer_t* lex) {
	const size_t start_pos = lex->expos;
	size_t length = 0;

	while (isdigit(lex_peek(lex))) {
		lex_next(lex);
		length++;
	}

	if (length == 0) return NULL;

	char* buffer = malloc(length + 1);
	if (!buffer) return NULL;

	memcpy(buffer, lex->file->src + start_pos, length);
	buffer[length] = '\0';

	return lex_token(lex, tok_number, buffer);
}

static token_info_t* lex_identifier(lexer_t* lex) {
	const size_t start_pos = lex->expos;
	size_t length = 0;
	char c;

	c = lex_peek(lex);
	if (!isalpha(c) && c != '_') return NULL;
	lex_next(lex);
	length++;

	while ((c = lex_peek(lex)) && (isalnum(c) || c == '_')) {
		lex_next(lex);
		length++;
	}

	char* buffer = malloc(length + 1);
	if (!buffer) return NULL;

	memcpy(buffer, lex->file->src + start_pos, length);
	buffer[length] = '\0';

	token_t type = tok_identifier;
	size_t n = sizeof(keywords) / sizeof(keywords[0]);
	for (size_t i = 0; i < n; i++) {
		if (strcmp(buffer, keywords[i].keyword) == 0) {
			type = keywords[i].token_type;
			break;
		}
	}

	return lex_token(lex, type, buffer);
}

void lexify(lexer_t* lex) {
	while (lex_peek(lex) != '\0') {
		const char c = lex_peek(lex);

		if (isdigit(c)) {
			if (!lex_number(lex)) break;
		}
		else if (isalpha(c) || c == '_') {
			if (!lex_identifier(lex)) break;
		}
		else if (isspace(c)) lex_next(lex);
		else {
			char op[2] = {c, '\0'};
			token_t t = tok_unknown;
			switch (op[0]) {
				case '+': t = tok_plus; break;
				case '-': t = tok_minus; break;
				case '(': t = tok_lparen; break;
				case ')': t = tok_rparen; break;
			}
			lex_token(lex, t, op);

			lex_next(lex);
		}
	}

	lex_token(lex, tok_eof, "");
}

lexer_t* lex_file(char* path) {
	lexer_t* lex = malloc(sizeof(lexer_t));
	lex->tokens = malloc(INITIAL_TOKENS_CAPACITY * sizeof(token_info_t));
	lex->tokens_allocated = INITIAL_TOKENS_CAPACITY;
	lex->tokens_count = 0;
	lex->lineno = 0;
	lex->col_offset = 0;
	lex->expos = 0;

	FILE* fptr;
	if ((fptr = fopen(path, "r")) == NULL) {
		printf("faild to open file '%s'\n", path);
		return NULL;
	}

	fseek(fptr, 0, SEEK_END); 
	size_t size = ftell(fptr);
	fseek(fptr, 0, SEEK_SET); 
	
	lexer_file_t* file = malloc(size);
	file->name = path;
	file->src = malloc(size);
	file->size = size;
	fread(file->src, 1, size, fptr);
	fclose(fptr);

	lex->file = file;

	return lex;
}
