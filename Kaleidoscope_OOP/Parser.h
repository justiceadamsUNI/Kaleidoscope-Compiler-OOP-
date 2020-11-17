#pragma once
#include <map>
#include "Lexer.h"
#include "AST.h"
#include "IRCodeGen.h"

using namespace std;

/**
* Changes made by justice: Obviously wrapping the Parser into a proper class. The public API for the parser now is limited to
* BinopPrecedence and the main function MainLoop.
*/

class Parser {
public:
	Parser(Lexer _Scanner) : Scanner(_Scanner), CurTok(Token(TokenType::tok_eof)) {};

	/// BinopPrecedence - This holds the precedence for each binary operator that is
	/// defined.
	map<char, int> BinopPrecedence;

	/// top ::= definition | external | expression | ';'
	void MainLoop();

	// Dumps code gen LLIR from code gen Module to stdout
	void PrintLLIRModule();

	~Parser() {
		// free up memory from code gen module
		delete CodeGenVisitor;
	};

private:
	// Handle to the Scanner instance which will be used by this Parser
	Lexer Scanner;

	// Create object to handle LLIR code generation via visitor pattern
	ASTCodeGenVisitor* CodeGenVisitor = new ASTCodeGenVisitor;

	/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
	/// token the parser is looking at.  getNextToken reads another token from the
	/// lexer and updates CurTok with its results.
	Token CurTok;

	// LogError* - These are little helper functions for error handling.
	const ExprAST* LogError(const char *Str);

	const PrototypeAST* LogErrorP(const char *Str);

	// Method for getting the next scanner token
	Token getNextToken();

	/// GetTokPrecedence - Get the precedence of the pending binary operator token.
	int GetTokPrecedence();

	/// numberexpr ::= number
	const ExprAST* ParseNumberExpr();

	/// parenexpr ::= '(' expression ')'
	const ExprAST* ParseParenExpr();

	/// identifierexpr
	///   ::= identifier
	///   ::= identifier '(' expression* ')'
	const ExprAST* ParseIdentifierExpr();

	/// primary
	///   ::= identifierexpr
	///   ::= numberexpr
	///   ::= parenexpr
	const ExprAST* ParsePrimary();

	/// binoprhs
	///   ::= ('+' primary)*
	const ExprAST* ParseBinOpRHS(int ExprPrec, const ExprAST* LHS);

	/// expression
	///   ::= primary binoprhs
	///
	const ExprAST* ParseExpression();

	/// prototype
	///   ::= id '(' id* ')'
	const PrototypeAST* ParsePrototype();

	/// definition ::= 'def' prototype expression
	const FunctionAST* ParseDefinition();

	/// toplevelexpr ::= expression
	const FunctionAST* ParseTopLevelExpr();

	/// external ::= 'extern' prototype
	const PrototypeAST* ParseExtern();

	//===----------------------------------------------------------------------===//
	// Top-Level parsing
	//===----------------------------------------------------------------------===//

	void HandleDefinition();

	void HandleExtern();

	void HandleTopLevelExpression();
};