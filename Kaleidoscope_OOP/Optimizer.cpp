#include "stdafx.h"
#include "Optimizer.h"

Optimizer::Optimizer(Module* IRModule, LLVMContext* TheContext)
{
	// Create a new pass manager attached to it.
	TheFPM = make_unique<legacy::FunctionPassManager>(IRModule);

	// Do simple "peephole" optimizations and bit-twiddling optzns.
	TheFPM->add(createInstructionCombiningPass());
	// Reassociate expressions.
	TheFPM->add(createReassociatePass());
	// Eliminate Common SubExpressions.
	TheFPM->add(createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	TheFPM->add(createCFGSimplificationPass());

	TheFPM->doInitialization();
}

void Optimizer::optimize(Function* TheFunction)
{
	// Optimize the function.
	TheFPM->run(*TheFunction);
}
