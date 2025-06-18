#include <lexer.h>
#include <parser.h>

int main(int argc, char* argv[]) {	
	lexer_t* lex = lex_file("test.pc");
	lexify(lex);

	ast_t* ast = parse_lex(lex);

	return 0;
}
