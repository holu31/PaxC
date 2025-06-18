#ifndef _PARSER_H
#define _PARSER_H

#include <ast.h>
#include <lexer.h>

ast_t* parse_lex(lexer_t* lex);

#endif // _PARSER_H
