#include <parser.h>
#include <stdlib.h>

typedef struct {
	lexer_t* lex;
	size_t pos;
} parser_t;

typedef token_t* (*parse_tok_t)();

token_info_t* parser_peek(parser_t* par) {
	lexer_t* lex = par->lex;
	if (par->pos >= lex->tokens_count) {
		return NULL;
	}
	return &lex->tokens[par->pos];
}

token_info_t* parser_next(parser_t* par) {
	par->pos++;
	return parser_peek(par);
}

token_info_t* parse_tokens(parser_t* par, parse_tok_t tok[], size_t tok_len) {
	size_t pos = par->pos;
	lexer_t* lex = par->lex;

	size_t tok_pos = 0;
	while (pos < lex->tokens_count) {
		token_info_t t = lex->tokens[pos];

		if (tok_pos >= tok_len) break;
		token_t* lex_tok = tok[tok_pos]();
		int found = 0;
		for (; lex_tok; lex_tok++) {
			if (t.type == *lex_tok) {
				found = 1;
				break;	
			}
		}
		if (!found) return NULL;

		tok_pos++;
		pos++;
	}

	size_t length = pos - par->pos;
	token_info_t* arr = malloc(length * sizeof(*arr));
	if (arr == NULL) return NULL;
	for (size_t i = par->pos; i < pos; i++) {
		size_t idx = i - par->pos;
		arr[idx] = lex->tokens[i];
	}
	par->pos = pos + 1;

	return arr;
}

token_t* p_arg() {
	static token_t tokens[] = {tok_number};
	return tokens;
}	

token_t* p_operator() {
	static token_t tokens[] = {tok_plus, tok_minus};
	return tokens;
}

ast_t* parse_expr(parser_t* par) {
	token_info_t* t = parser_peek(par);
	if (!t) return NULL;
	
	ast_t* ast = malloc(sizeof(ast_t));
	if (!ast) return NULL;

	if (t->type == tok_number) {
		ast_type_info_t* type = malloc(sizeof(ast_type_info_t));
		if (!type) {
			free(ast);
			return NULL;
		}
		type->type = type_int;
		type->size = 4;
		type->issigned = 0;

		ast->type = type;
		ast->i64 = atol(t->lexeme);;
		parser_next(par);
	}

	return ast;
}

// TEST
ast_t* parse_binop(parser_t* par) {
	ast_t* ast = malloc(sizeof(ast_t));
	if (!ast) return NULL;

	parse_tok_t tok[] = {p_arg, p_operator, p_arg};
	size_t tok_len = sizeof(tok) / sizeof(tok[0]);
	token_info_t* info = parse_tokens(par, tok, tok_len);
	if (!info) {
		free(ast);
		return NULL;
	}	

	ast->left = parse_expr(par);
	ast->right = parse_expr(par);

	free(info);
	return ast;
}

ast_t* parse_lex(lexer_t* lex) {
	parser_t par;
	par.lex = lex;
	par.pos = 0;

	ast_t* ast = parse_binop(&par);

	return ast;
}
