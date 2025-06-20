#include <lexer.h>
#include <parser.h>
#include <ast.h>
#include <codegen.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {	
	lexer_t* lex = lex_file("test.pc");
	lexify(lex);

	ast_t* ast = parse_lex(lex);
	ast_print(ast, 0);

	codegen_init("main_module");
	LLVMModuleRef module = codegen(ast);
	if (!module) {
		fprintf(stderr, "failed codegen\n");
		return 1;
	}

	if (save_module_to_file(module, "output.ll") != 0) {
		fprintf(stderr, "failed to save IR to output.ll\n");
		return 1;
	}

	if (codegen_emit_object(module, "output.o") != 0) {
		fprintf(stderr, "failed to emit object file\n");
		return 1;
	}

	if (system("clang output.o -o output")) {
		fprintf(stderr, "failed to link with clang\n");
		return 1;
	}

	ast_free(ast);
	codegen_dispose();

	return 0;
}
