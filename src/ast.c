#include <ast.h>
#include <stdio.h>
#include <stdlib.h>

ast_t* ast_make_literal(int64_t value, ast_type_info_t* type) {
	ast_t* node = malloc(sizeof(ast_t));
	if (!node) return NULL;

	node->node_type = AST_LITERAL;
	node->type = type;
	node->literal.value = value;

	return node;
}

ast_t* ast_make_binary(ast_t* left, ast_t* right, token_t op, ast_type_info_t* type) {
	ast_t* node = malloc(sizeof(ast_t));
	if (!node) return NULL;

    	node->node_type = AST_BINARY_OP;
	node->type = type;
	node->binary.left = left;
	node->binary.right = right;
	node->binary.op = op;

	return node;
}

ast_t* ast_make_unary(ast_t* operand, token_t op, ast_type_info_t* type) {
	ast_t* node = malloc(sizeof(ast_t));
	if (!node) return NULL;

	node->node_type = AST_UNARY_OP;
	node->type = type;
	node->unary.operand = operand;
	node->unary.op = op;

	return node;
}

void ast_free(ast_t* node) {
	if (!node) return;

	switch (node->node_type) {
		case AST_LITERAL:
			break;
		case AST_BINARY_OP:
			ast_free(node->binary.left);
			ast_free(node->binary.right);
			break;
		case AST_UNARY_OP:
			ast_free(node->unary.operand);
			break;
	}

	free(node);
}

void ast_print(ast_t* node, int depth) {
	if (!node) return;

	for (int i = 0; i < depth; i++) printf("  ");

	switch (node->node_type) {
		case AST_LITERAL:
			printf("literal: %ld\n", node->literal.value);
			break;
		case AST_UNARY_OP:
			printf("unary: %s\n", node->unary.op == tok_minus ? "-" : "+");
			ast_print(node->unary.operand, depth + 1);
			break;
		case AST_BINARY_OP:
			printf("binary: %s\n",
				node->binary.op == tok_plus ? "+" :
				node->binary.op == tok_minus ? "-" :
				node->binary.op == tok_star ? "*" :
				node->binary.op == tok_slash ? "/" : "?");
			ast_print(node->binary.left, depth + 1);
			ast_print(node->binary.right, depth + 1);
			break;
	}
}
