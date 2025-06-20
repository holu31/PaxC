#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <ast.h>
#include <stdio.h>
#include <stdlib.h>

LLVMModuleRef codegen_module;
LLVMBuilderRef builder;
LLVMContextRef context;
static LLVMTargetMachineRef target_machine = NULL;

LLVMValueRef codegen_expr(ast_t* node);

int codegen_init(const char* module_name) {
	LLVMInitializeNativeTarget();
	LLVMInitializeAllTargetMCs();
	LLVMInitializeNativeAsmPrinter();
	LLVMInitializeNativeAsmParser();

	context = LLVMContextCreate();
	codegen_module = LLVMModuleCreateWithNameInContext(module_name, context);
	builder = LLVMCreateBuilderInContext(context);

	char* error = NULL;
	char* target_triple = LLVMGetDefaultTargetTriple();

	LLVMTargetRef target;	
	if (LLVMGetTargetFromTriple(target_triple, &target, &error) != 0) {
		fprintf(stderr, "error getting target: %s\n", error);
		LLVMDisposeMessage(error);
		return -1;
	}

	target_machine = LLVMCreateTargetMachine(
		target,
		target_triple,
		"",
		"",
		LLVMCodeGenLevelDefault,
		LLVMRelocDefault,
		LLVMCodeModelDefault
	);

	LLVMSetTarget(codegen_module, target_triple);
	LLVMDisposeMessage(target_triple);

	return 0;
}

void codegen_dispose() {
	if (builder) LLVMDisposeBuilder(builder);
	if (codegen_module) LLVMDisposeModule(codegen_module);
	if (context) LLVMContextDispose(context);
	if (target_machine) LLVMDisposeTargetMachine(target_machine);

	builder = NULL;
	codegen_module = NULL;
	context = NULL;
	target_machine = NULL;
}

LLVMTypeRef get_llvm_type(ast_type_info_t* type) {
	if (type->llvm_type)
		return type->llvm_type;

	switch (type->kind) {
		case AST_TYPE_VOID:
			type->llvm_type = LLVMVoidTypeInContext(context);
			break;
		case AST_TYPE_INT:
			switch(type->int_info.bits) {
				case 8: type->llvm_type = LLVMInt8TypeInContext(context); break;
				case 16: type->llvm_type = LLVMInt16TypeInContext(context); break;
				case 32: type->llvm_type = LLVMInt32TypeInContext(context); break;
				case 64: type->llvm_type = LLVMInt64TypeInContext(context); break;
				default: /*todo: error*/ type->llvm_type = NULL;
			}
			break;
		case AST_TYPE_FLOAT:
			switch(type->float_info.bits) {
				case 32: type->llvm_type = LLVMFloatTypeInContext(context); break;
				case 64: type->llvm_type = LLVMDoubleTypeInContext(context); break;
				default: type->llvm_type = NULL;
			}
			break;
		case AST_TYPE_PTR:
			type->llvm_type = LLVMPointerType(get_llvm_type(type->ptr_to), 0);
			break;
		case AST_TYPE_STRUCT: {
			LLVMTypeRef* elems = malloc(sizeof(LLVMTypeRef)*type->struct_info.count);
			for (size_t i = 0; i < type->struct_info.count; i++) {
				elems[i] = get_llvm_type(type->struct_info.members[i]);
			}
			type->llvm_type = LLVMStructTypeInContext(context, elems, type->struct_info.count, 0);
			free(elems);
			break;
		}
		case AST_TYPE_ARRAY:
			type->llvm_type = LLVMArrayType(get_llvm_type(type->array_info.element_type), type->array_info.count);
			break;
		default:
			type->llvm_type = NULL;
	}
	
	return type->llvm_type;
}


LLVMValueRef codegen_expr(ast_t* node) {
	switch (node->node_type) {
		case AST_LITERAL: {
			LLVMTypeRef ty = get_llvm_type(node->type);
			return LLVMConstInt(ty, node->literal.value, node->type->issigned);
		}
		case AST_UNARY_OP: {
			LLVMValueRef val = codegen_expr(node->unary.operand);
			switch (node->unary.op) {
				case tok_minus:
					return LLVMBuildNeg(builder, val, "neg_tmp");
				case tok_plus:
					return val;
				default:
					return NULL;
			}
		}
		case AST_BINARY_OP: {
			LLVMValueRef lhs = codegen_expr(node->binary.left);
			LLVMValueRef rhs = codegen_expr(node->binary.right);
			switch (node->binary.op) {
				case tok_plus:
					return LLVMBuildAdd(builder, lhs, rhs, "add_tmp");
				case tok_minus:
					return LLVMBuildSub(builder, lhs, rhs, "sub_tmp");
				case tok_star:
					return LLVMBuildMul(builder, lhs, rhs, "mul_tmp");
				case tok_slash:
					if (node->type->issigned)
						return LLVMBuildSDiv(builder, lhs, rhs, "div_tmp");
					else
						return LLVMBuildUDiv(builder, lhs, rhs, "div_tmp");
				default:
					return NULL;
			}
		}
		default:
				    return NULL;
	}
}

LLVMModuleRef codegen(ast_t* root) {
	LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32TypeInContext(context), NULL, 0, 0);
	LLVMValueRef main_func = LLVMAddFunction(codegen_module, "main", main_type);
	LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context, main_func, "entry");
	LLVMPositionBuilderAtEnd(builder, entry);

	LLVMValueRef retval = codegen_expr(root);
	if (!retval) retval = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
	
	LLVMBuildRet(builder, retval);

	if (LLVMVerifyModule(codegen_module, LLVMReturnStatusAction, NULL)) {
		fprintf(stderr, "module verification failed!\n");
		return NULL;
	}

	return codegen_module;
}

int save_module_to_file(LLVMModuleRef module, const char* filename) {
	char* error = NULL;
	if (LLVMPrintModuleToFile(module, filename, &error) != 0) {
		fprintf(stderr, "error saving module to file: %s\n", error);
		LLVMDisposeMessage(error);
		return -1;
	}
	return 0;
}

int codegen_emit_object(LLVMModuleRef module, const char* filename) {
	char* error = NULL;
	if (!target_machine) {
		fprintf(stderr, "target machine not initialized\n");
		return -1;
	}
	if (LLVMTargetMachineEmitToFile(target_machine, module, filename, LLVMObjectFile, &error) != 0) {
		fprintf(stderr, "error emitting object file: %s\n", error);
		LLVMDisposeMessage(error);
		return -1;
	}
	return 0;
}
