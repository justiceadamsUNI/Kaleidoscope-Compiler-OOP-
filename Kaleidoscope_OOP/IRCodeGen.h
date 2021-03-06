#pragma once
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Error.h"
#include "AST.h"
#include "Optimizer.h"
#include "JITRuntimeWrapper.h"

/**
* Changes made by justice: We use a visitor pattern for code generation as opposed to an overridden abstract method.
* With the visitor, all related code is confied in a single class (the visitor) and is furthermore exetendible
* to the extend that multiple vistors can be created. More AST visitors could be created which essentially optimize
* the AST. In this case, we use the GoF style visitor pattern as opposed to abstract methods implemented all over 
* the place to adhere to a cleaner codebase.
*/

using namespace llvm;

class ASTCodeGenVisitor : public ExprASTVisitor<Value*>
{
public:
	ASTCodeGenVisitor();

	// Publicly needed CodeGen elements for JIT execution
	LLVMContext* TheContext;
	Module* TheModule;
	JITRuntimeWrapper JIT;
	map<string, const PrototypeAST*> FunctionProtos;

	Value* visit(NumberExprAST* NumberExpr);
	Value* visit(VariableExprAST* VariableExpr);
	Value* visit(BinaryExprAST* BinaryExprAST);
	Value* visit(CallExprAST* CallExprAST);
	Value* visit(PrototypeAST* PrototypeAST);
	Value* visit(FunctionAST* FunctionAST);

	// public function for reinitializing a new module for JIT'ing (needed by the parser)
	void InitializeModuleAndPassManager();

	// public method for pretty-printing code-gen
	void PrintIR(); 

	// public function for keeping track of Prototypes across IR modules.
	Function* getFunction(string Name);

	~ASTCodeGenVisitor() {
		delete TheContext;
		delete TheModule;
		delete Builder;
		delete IROptimizer;
	}

private:
	IRBuilder<>* Builder;
	Optimizer* IROptimizer;
	map<string, Value*> NamedValues;
};