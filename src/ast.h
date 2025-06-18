#ifndef _AST_H
#define _AST_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
	type_void = 0,
	type_char,
	type_int,
	type_float,
} ast_type_t;

typedef struct {
	size_t size;
	ast_type_t type;
	int issigned;
} ast_type_info_t;

typedef struct ast ast_t;
struct ast {
	ast_type_info_t* type;

	union {
		struct {
			int64_t i64;
		};

		struct {
			ast_t* left;
			ast_t* right;
		};
	};
};

#endif // _AST_H
