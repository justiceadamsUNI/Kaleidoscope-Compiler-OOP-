#include "stdafx.h"
#include "IRCodeGen.h"
#include "Logger.h"

/**
* Changes made by justice: Visitor recursion is rare, but here it happens because of the nested nature of AST's.
* This requres some const_cast's to remove the const-ness from the ASTExpr objects. We want the const-ness for encapsulation
* but to call accept() with a const object requires some quick finagling. This seems to be a symptom of working with C++.,
* Again, we don't want to enforce visitors to work on only const nodes (see AST.h)
*/

ASTCodeGenVisitor::ASTCodeGenVisitor() {
	InitializeModuleAndPassManager();
}

void ASTCodeGenVisitor::InitializeModuleAndPassManager() {
	TheContext = new LLVMContext();
	TheModule = new Module("my cool jit", *TheContext);
	TheModule->setDataLayout(JIT.TheJIT->getDataLayout());
	Builder = new IRBuilder<>(*TheContext);
	IROptimizer = new Optimizer(TheModule, TheContext);
}

void ASTCodeGenVisitor::PrintIR() {
	// Print out all of the generated code.
	TheModule->print(errs(), nullptr);
}

Value* ASTCodeGenVisitor::visit(NumberExprAST* NumberExpr)
{
	return ConstantFP::get(*TheContext, APFloat(NumberExpr->Val));
}

Value* ASTCodeGenVisitor::visit(VariableExprAST* VariableExpr)
{
	// Look this variable up in the function.
	Value* V = NamedValues[VariableExpr->Name];
	if (!V)
		LogError("Unknown variable name");
	return V;
}

Value* ASTCodeGenVisitor::visit(BinaryExprAST* BinaryExpr)
{
	Value* L = const_cast<ExprAST*>(BinaryExpr->LHS)->accept(this);
	Value* R = const_cast<ExprAST*>(BinaryExpr->RHS)->accept(this);
	if (!L || !R)
		return nullptr;

	switch (BinaryExpr->Op) {
	case '+':
		return Builder->CreateFAdd(L, R, "addtmp");
	case '-':
		return Builder->CreateFSub(L, R, "subtmp");
	case '*':
		return Builder->CreateFMul(L, R, "multmp");
	case '<':
		L = Builder->CreateFCmpULT(L, R, "cmptmp");
		// Convert bool 0/1 to double 0.0 or 1.0
		return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext),
			"booltmp");
	default:
		LogError("invalid binary operator");
		return nullptr;
	}
}

Value* ASTCodeGenVisitor::visit(CallExprAST* CallExpr)
{
	// Look up the name in the global module table.
	Function* CalleeF = TheModule->getFunction(CallExpr->Callee);
	if (!CalleeF) {
		LogError("Unknown function referenced");
		return nullptr;
	}

	// If argument mismatch error.
	if (CalleeF->arg_size() != CallExpr->Args.size()) {
		LogError("Incorrect # arguments passed");
		return nullptr;
	}

	vector<Value*> ArgsV;
	for (unsigned i = 0, e = CallExpr->Args.size(); i != e; ++i) {
		ArgsV.push_back(const_cast<ExprAST*>(CallExpr->Args[i])->accept(this));
		if (!ArgsV.back())
			return nullptr;
	}

	return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

Value* ASTCodeGenVisitor::visit(PrototypeAST* ProtypeExpr)
{
	// Make the function type:  double(double,double) etc.
	vector<Type*> Doubles(ProtypeExpr->Args.size(),
		Type::getDoubleTy(*TheContext));
	FunctionType* FT =
		FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

	Function* F =
		Function::Create(FT, Function::ExternalLinkage, "__anon_expr", TheModule);

	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto& Arg : F->args())
		Arg.setName(ProtypeExpr->Args[Idx++]);

	return F;
}

Value* ASTCodeGenVisitor::visit(FunctionAST* FunctionExpr)
{
	// First, check for an existing function from a previous 'extern' declaration.
	Function* TheFunction = TheModule->getFunction(FunctionExpr->Proto->Name);

	if (!TheFunction)
		TheFunction = (Function*) const_cast<PrototypeAST*>(FunctionExpr->Proto)->accept(this);

	if (!TheFunction)
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
	Builder->SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	NamedValues.clear();
	for (auto& Arg : TheFunction->args())
		NamedValues[string(Arg.getName())] = &Arg;

	if (Value* RetVal = const_cast<ExprAST*>(FunctionExpr->Body)->accept(this)) {
		// Finish off the function.
		Builder->CreateRet(RetVal);

		// Validate the generated code, checking for consistency.
		verifyFunction(*TheFunction);

		// Optimize the function.
		IROptimizer->optimize(TheFunction);

		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();
	return nullptr;
}