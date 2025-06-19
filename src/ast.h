#ifndef _AST_H
#define _AST_H

#include <stdint.h>
#include <stddef.h>
#include <lexer.h>

typedef enum {
	TYPE_VOD = 0,
	TYPE_CHAR,
	TYPE_INT,
	TYPE_FLOAT,
} ast_type_t;

typedef struct {
	size_t size;
	ast_type_t type;
	int issigned;
} ast_type_info_t;

static ast_type_info_t* i32 = &(ast_type_info_t){.size = 4, .type = TYPE_INT, .issigned = 0};

typedef enum {
	AST_LITERAL,
	AST_BINARY_OP,
	AST_UNARY_OP,
} ast_node_type_t;

typedef struct ast ast_t;
struct ast {
	ast_node_type_t node_type;
	ast_type_info_t* type;

	union {
		struct {
			int64_t value;
		} literal;

		struct {
			ast_t* left;
			ast_t* right;
			token_t op;
		} binary;

		struct {
			struct ast* operand;
			token_t op;
		} unary;
	};
};

#endif // _AST_H
