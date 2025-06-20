#ifndef _AST_H
#define _AST_H

#include <llvm-c/Core.h>
#include <stdint.h>
#include <stddef.h>
#include <lexer.h>

typedef struct ast_type_info {
	enum {
		AST_TYPE_VOID,
		AST_TYPE_INT,
		AST_TYPE_FLOAT,
		AST_TYPE_PTR,
		AST_TYPE_STRUCT,
		AST_TYPE_ARRAY,
	} kind;

	int issigned;

	union {
		struct {
			unsigned bits;
		} int_info;

		struct {
			unsigned bits;
		} float_info;
		
		struct ast_type_info* ptr;

		struct {
			struct ast_type_info** members;
			size_t count;
		} struct_info;
		
		struct {
			struct ast_type_info* element_type;
			size_t count;
		} array_info;
	};

	// Cache
	LLVMTypeRef llvm_type;
} ast_type_info_t;

static ast_type_info_t* i32 = &(ast_type_info_t){
	.kind = AST_TYPE_INT,
	.issigned = 1,
	.int_info = {
		.bits = 32
	}
};

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

ast_t* ast_make_literal(int64_t value, ast_type_info_t* type);
ast_t* ast_make_binary(ast_t* left, ast_t* right, token_t op, ast_type_info_t* type);
ast_t* ast_make_unary(ast_t* operand, token_t op, ast_type_info_t* type);
void ast_free(ast_t* node);
void ast_print(ast_t* node, int depth);

#endif // _AST_H
