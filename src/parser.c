#include <parser.h>
#include <ast.h>
#include <stdlib.h>

typedef struct {
	lexer_t* lex;
	size_t pos;
} parser_t;

typedef enum {
	RULE_TERMINAL,
	RULE_CHOICE,
	RULE_SEQUENCE,
	RULE_OPTIONAL,
	RULE_FUNC,
} rule_type_t;

typedef ast_t* (*parse_ast_func)(parser_t*);

typedef struct rule_t {
	rule_type_t type;
	union {
		token_t token;
		parse_ast_func func;
		struct rule_t* subrule;
		struct {
			struct rule_t** rules;
			size_t count;
		};
	};	
} rule_t;

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

rule_t* rule_terminal(token_t token) {
	rule_t* r = malloc(sizeof(rule_t));
	r->type = RULE_TERMINAL;
	r->token = token;
	return r;
}

rule_t* rule_choice(rule_t** rules, size_t count) {
	rule_t* r = malloc(sizeof(rule_t));
	r->type = RULE_CHOICE;
	r->rules = malloc(count * sizeof(rule_t*));
	for (size_t i = 0; i < count; ++i) {
		r->rules[i] = rules[i];
	}
	r->count = count;
	return r;
}

rule_t* rule_sequence(rule_t** rules, size_t count) {
	rule_t* r = malloc(sizeof(rule_t));
	r->type = RULE_SEQUENCE;
	r->rules = rules;
	r->count = count;
	return r;
}

rule_t* rule_optional(rule_t* subrule) {
    rule_t* r = malloc(sizeof(rule_t));
    r->type = RULE_OPTIONAL;
    r->subrule = subrule;
    return r;
}

rule_t* rule_func(parse_ast_func func) {
	rule_t* r = malloc(sizeof(rule_t));
	r->type = RULE_FUNC;
	r->func = func;
	return r;
}

token_info_t* parse_rule(parser_t* par, rule_t* rule) {
	switch(rule->type) {
		case RULE_TERMINAL: {
			token_info_t* tok = parser_peek(par);
			if (tok && tok->type == rule->token) {
				parser_next(par);
				return tok;
			}
			return NULL;
		}
		case RULE_CHOICE: {
			size_t start_pos = par->pos;
			token_info_t* ret = NULL;
			for (size_t i = 0; i < rule->count; i++) {
				ret = parse_rule(par, rule->rules[i]);
				if (!ret) {
					par->pos = start_pos;
				} else break;
			}
			return ret;
		}
		case RULE_SEQUENCE: {
			size_t start_pos = par->pos;
			token_info_t* last = NULL;	
			for (size_t i = 0; i < rule->count; i++) {
				last = parse_rule(par, rule->rules[i]);
				if (!last) {
					par->pos = start_pos;
					return NULL;
				}
			}
			return last;
		}
		case RULE_OPTIONAL: {
			size_t start_pos = par->pos;
			token_info_t* result = parse_rule(par, rule->subrule);
			if (!result) par->pos = start_pos;
			return result;
		}
		default:
			return NULL;
	}
}

ast_t* parse_rule_ast(parser_t* par, rule_t* rule) {
	switch (rule->type) {
		case RULE_FUNC: {
			return rule->func(par);
		}
		case RULE_CHOICE: {
			size_t start_pos = par->pos;
			for (size_t i = 0; i < rule->count; i++) {
				ast_t* result = parse_rule_ast(par, rule->rules[i]);
				if (result) return result;
				par->pos = start_pos;
			}
			return NULL;
		}
		default:
			return NULL;
	}
}

rule_t* rule_operator() {
	rule_t* plus = rule_terminal(tok_plus);
	rule_t* minus = rule_terminal(tok_minus);

	rule_t* operators[] = {plus, minus};
	return rule_choice(operators, 2);
}

rule_t* rule_term() {
	rule_t* number = rule_terminal(tok_number);

	rule_t* types[] = {number};
	return rule_choice(types, 1);
}

rule_t* rule_arg() {
	rule_t* term = rule_term();

	rule_t* types[] = {term};
	return rule_choice(types, 1);
}

ast_t* parse_expr(parser_t* par);
ast_t* parse_binop(parser_t* par);

ast_t* parse_term(parser_t* par) {
	rule_t* term = rule_term();	
	token_info_t* t = parse_rule(par, term);
	if (!t) return NULL;

	if (t->type == tok_number) {
		ast_t* node = malloc(sizeof(ast_t));
		if (!node) return NULL;

		node->node_type = AST_LITERAL;
		node->type = i32;
		node->literal.value = atol(t->lexeme);	
		return node;
	}
	return NULL;
}

ast_t* parse_binop(parser_t* par) {
	ast_t* left = parse_term(par);
	if (!left) return NULL;

	rule_t* rule = rule_operator();
	token_info_t* op = parse_rule(par, rule);

	if (!op) {
		free(left);
		return NULL;
	}
	
	rule = rule_func(parse_expr);
	ast_t* right = parse_rule_ast(par, rule);
	if (!right) return NULL;

	ast_t* binop = malloc(sizeof(ast_t));
	if (!binop) return NULL;
	binop->node_type = AST_BINARY_OP;
	binop->type = i32;
	binop->binary.left = left;
	binop->binary.right = right;
	binop->binary.op = op->type;

	return binop;
}

ast_t* parse_expr(parser_t* par) {
	rule_t* num_func = rule_func(parse_term);
	rule_t* binop_func = rule_func(parse_binop);
	rule_t* choices[] = {binop_func, num_func};
	rule_t* choice_rule = rule_choice(choices, 2);

	return parse_rule_ast(par, choice_rule);
}

ast_t* parse_lex(lexer_t* lex) {
	parser_t par;
	par.lex = lex;
	par.pos = 0;

	ast_t* ast = parse_expr(&par);

	return ast;
}
