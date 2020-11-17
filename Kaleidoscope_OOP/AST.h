#pragma once
# include <memory>
# include <string>
# include <vector>
# include "llvm/IR/Value.h"

using namespace std;
using namespace llvm;

/**
* Changes made by justice: All unique pointers have been replaced with constant pointers to AST nodes.
* Now that this codebase is OOP, there is a desire to encapsulate the pointers contained by the AST nodes in some aspect.
* There is no real reason to use unique_ptrs as they would have to be moved constantly to and from the Parser and
* would muddy the code. The unique_ptr's make sense when there is no encapsulation at all (no const, no getters, etc) around
* the pointers as you can access a direct reference to the unique_ptr (instance variables). 
* By using pointers to const data, we can ensure that once an AST node is created, nothing will change the internal 
* structure of the node (save altering the pointer itself). This provides some level of 
* encapuslation: pointers can still be accessed by instance variables on the AST nodes and memory management has been
* explicitly added via destructors. This is the best design decision for this modified codebase in my opinion.
*/

/**
* Changes made by justice: Added an ASTVisitor which can be extended to visit the various AST nodes.
* Note that this base visitor does not use constant visit methods. This could be changed, but it felt wrong to enforce
* that the visitors that are derived must adhere to constant visit functions. There's no need to enforce implementation
* constrtaints on derived visitors in my opnion (thus no const functions).
*/

template <class ReturnType> class ExprASTVisitor
{
public:
	virtual ReturnType visit(class NumberExprAST*) = 0;
	virtual ReturnType visit(class VariableExprAST*) = 0;
	virtual ReturnType visit(class BinaryExprAST*) = 0;
	virtual ReturnType visit(class CallExprAST*) = 0;
	virtual ReturnType visit(class PrototypeAST*) = 0;
	virtual ReturnType visit(class FunctionAST*) = 0;
};

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
	virtual ~ExprAST() {}

	virtual Value* accept(ExprASTVisitor<Value*>* v) = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
public:
	NumberExprAST(double Val) : Val(Val) {}

	double Val;

	Value* accept(ExprASTVisitor<Value*>* v) { return v->visit(this); }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
	VariableExprAST(string &Name) : Name(Name) {}

	string Name;

	Value* accept(ExprASTVisitor<Value*>* v) { return v->visit(this); }
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
public:
	BinaryExprAST(char op, const ExprAST* LHS, const ExprAST* RHS)
		: Op(op), LHS(LHS), RHS(RHS) {}

	char Op;

	// Pointers to left and right hand statements
	const ExprAST* LHS;
	const ExprAST* RHS;

	Value* accept(ExprASTVisitor<Value*>* v) { return v->visit(this); }

	~BinaryExprAST() {
		delete LHS;
		delete RHS;
	}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
public:
	CallExprAST(string Callee, vector<const ExprAST*> Args)
		: Callee(Callee), Args(Args) {}

	string Callee;

	vector<const ExprAST*> Args;

	Value* accept(ExprASTVisitor<Value*>* v) { return v->visit(this); }
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {

public:
	PrototypeAST(string name, vector<string> Args)
		: Name(name), Args(Args) {}

	string Name;

	vector<string> Args;

	Value* accept(ExprASTVisitor<Value*>* v) { return v->visit(this); }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {

public:
	FunctionAST(const PrototypeAST* Proto, const ExprAST* Body)
		: Proto(Proto), Body(Body) {}

	const ExprAST* Body;

	const PrototypeAST* Proto;

	Value* accept(ExprASTVisitor<Value*>* v) { return v->visit(this); }

	~FunctionAST() {
		delete Body;
		delete Proto;
	}
};