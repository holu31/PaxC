#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <llvm-c/Core.h>

int codegen_init(const char* module_name);
void codegen_dispose();
LLVMModuleRef codegen(ast_t* root); 
int save_module_to_file(LLVMModuleRef module, const char* filename);
int codegen_emit_object(LLVMModuleRef module, const char* filename); 

#endif // _CODEGEN_H
