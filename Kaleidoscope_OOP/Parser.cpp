#include "stdafx.h"
#include "Parser.h"

/**
* Changes made by justice: all memory allocation for AST's happens here. All AST's are collapsed into one large
* AST. This is the nature of abstract syntax trees. Thus, when the final tree is deleted from the heap, it will trigger
* the proper destructors on each of the AST nodes themselves (which are themselves AST's). The decision to use
* pointers to const data is explained in AST.h
*/

Token Parser::getNextToken()
{
	return CurTok = Scanner.getToken();
}

int Parser::GetTokPrecedence()
{
	if (CurTok.getType() != TokenType::tok_char)
		return -1;

	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[CurTok.getNumValue()];
	if (TokPrec <= 0)
		return -1;
	return TokPrec;
}

const ExprAST* Parser::ParseExpression()
{
	auto LHS = ParsePrimary();
	if (!LHS)
		return nullptr;

	return ParseBinOpRHS(0, LHS);
}

const PrototypeAST* Parser::ParsePrototype()
{
	if (CurTok.getType() != tok_identifier)
		return LogErrorP("Expected function name in prototype");

	string FnName = CurTok.getIdentifierString();
	getNextToken();

	if (CurTok.getNumValue() != '(')
		return LogErrorP("Expected '(' in prototype");

	vector<string> ArgNames;
	while (getNextToken().getType() == tok_identifier)
		ArgNames.push_back(CurTok.getIdentifierString());
	if (CurTok.getNumValue() != ')')
		return LogErrorP("Expected ')' in prototype");

	// success.
	getNextToken(); // eat ')'.

	return new PrototypeAST(FnName, move(ArgNames));
}

const FunctionAST* Parser::ParseDefinition()
{
	getNextToken(); // eat def.
	auto Proto = ParsePrototype();
	if (!Proto)
		return nullptr;

	if (auto E = ParseExpression())
		return new FunctionAST(FunctionAST(Proto, E));
	return nullptr;
}

const FunctionAST* Parser::ParseTopLevelExpr()
{
	if (auto E = ParseExpression()) {
		// Make an anonymous proto.
		auto Proto = new PrototypeAST("__anon_expr", vector<string>());
		return new FunctionAST(Proto, E);
	}
	return nullptr;
}

const PrototypeAST* Parser::ParseExtern()
{
	getNextToken(); // eat extern.
	return ParsePrototype();
}

void Parser::HandleDefinition()
{
	const FunctionAST* Definition = ParseDefinition();
	if (Definition) {
		fprintf(stderr, "Parsed a function definition.\n");
		if (auto* FnIR = const_cast<FunctionAST*>(Definition)->accept(CodeGenVisitor)) {
			fprintf(stderr, "Read function definition:");
			FnIR->print(errs());
			fprintf(stderr, "\n");
		}
		delete Definition; // Clean up the memory allocated for this AST node
	}
	else {
		// Skip token for error recovery.
		getNextToken();
	}
}

void Parser::HandleExtern()
{
	const PrototypeAST* Extern = ParseExtern();
	if (Extern) {
		fprintf(stderr, "Parsed an extern\n");
		if (auto* FnIR = const_cast<PrototypeAST*>(Extern)->accept(CodeGenVisitor)) {
			fprintf(stderr, "Read extern: ");
			FnIR->print(errs());
			fprintf(stderr, "\n");
		}

		delete Extern; // Clean up the memory allocated for this AST node
	}
	else {
		// Skip token for error recovery.
		getNextToken();
	}
}

void Parser::HandleTopLevelExpression()
{
	// Evaluate a top-level expression into an anonymous function.
	const FunctionAST* TopLevelException = ParseTopLevelExpr();
	if (TopLevelException) {
		fprintf(stderr, "Parsed a top-level expr\n");
		if (auto* FnIR = const_cast<FunctionAST*>(TopLevelException)->accept(CodeGenVisitor)) {
			fprintf(stderr, "Read top-level expression:");
			FnIR->print(errs());
			fprintf(stderr, "\n");

			// Remove the anonymous expression.
			((Function*) FnIR)->eraseFromParent();
		}
		delete TopLevelException; // Clean up the memory allocated for this AST node
	}
	else {
		// Skip token for error recovery.
		getNextToken();
	}
}

void Parser::MainLoop()
{
	getNextToken();

	while (true) {
		fprintf(stderr, "ready> ");
		switch (CurTok.getType()) {
		case tok_eof:
			return;
		case tok_char:
			if (CurTok.getNumValue() == ';') {
				// ignore top-level semicolons.
				getNextToken();
				break;
			}

			// fallthrough to default
			HandleTopLevelExpression();
		case tok_def:
			HandleDefinition();
			break;
		case tok_extern:
			HandleExtern();
			break;
		default:
			HandleTopLevelExpression();
			break;
		}
	}
}

void Parser::PrintLLIRModule()
{
	// Delegate call to the codegen module (visitor)
	CodeGenVisitor->PrintIR();
}

const ExprAST* Parser::LogError(const char * Str)
{
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

 const PrototypeAST* Parser::LogErrorP(const char * Str)
{
	LogError(Str);
	return nullptr;
}

const ExprAST* Parser::ParseNumberExpr()
{
	auto Result = new NumberExprAST(CurTok.getNumValue());
	getNextToken(); // consume the number
	return Result;
}

const ExprAST* Parser::ParseParenExpr()
{
	getNextToken(); // eat (.
	auto V = ParseExpression();
	if (!V)
		return nullptr;

	if (CurTok.getNumValue() != ')')
		return LogError("expected ')'");
	getNextToken(); // eat ).
	return V;
}

const ExprAST* Parser::ParseIdentifierExpr()
{
	string IdName = CurTok.getIdentifierString();

	getNextToken(); // eat identifier.

	if (CurTok.getNumValue() != '(') // Simple variable ref.
		return new VariableExprAST(IdName);

	// Call.
	getNextToken(); // eat (
	std::vector<const ExprAST*> Args;
	if (CurTok.getNumValue() != ')') {
		while (true) {
			if (auto Arg = ParseExpression())
				Args.push_back(Arg);
			else
				return nullptr;

			if (CurTok.getNumValue() == ')')
				break;

			if (CurTok.getNumValue() != ',')
				return LogError("Expected ')' or ',' in argument list");
			getNextToken();
		}
	}

	// Eat the ')'.
	getNextToken();

	return new CallExprAST(IdName, Args);
}

const ExprAST* Parser::ParsePrimary()
{
	switch (CurTok.getType()) {
	default:
		return LogError("unknown token when expecting an expression");
	case tok_identifier:
		return ParseIdentifierExpr();
	case tok_number:
		return ParseNumberExpr();
	case tok_char:
		if (CurTok.getNumValue() == '(') return ParseParenExpr();

		return LogError("unknown token when expecting an expression");
	}
}

const ExprAST* Parser::ParseBinOpRHS(int ExprPrec, const ExprAST* LHS)
{
	// If this is a binop, find its precedence.
	while (true) {
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec)
			return LHS;

		// Okay, we know this is a binop.
		int BinOp = CurTok.getNumValue();
		getNextToken(); // eat binop

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimary();
		if (!RHS)
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = ParseBinOpRHS(TokPrec + 1, RHS);
			if (!RHS)
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS = new BinaryExprAST(BinOp, LHS, RHS);
	}
}
