#pragma once
# include <memory>
# include <string>
# include <vector>

using namespace std;

/**
* Changes made by justice: All unique pointers have been replaced with constant pointers to AST nodes.
* Now that this codebase is OOP, there is a desire to encapsulate the pointers contained by the AST nodes in some aspect.
* There is no real reason to use unique_ptrs as they would have to be moved constantly to and from the Parser and
* would muddy the code. The unique_ptr's make sense when there is no encapsulation at all (no const, no getters, etc) around
* the pointers as you can access a direct reference to the unique_ptr (instance variables). 
* By using pointers to const data, we can ensure that once an AST node is created, nothing will change the internal 
* structure of the node (save altering the pointer itself). This provides some level of 
* encapuslation, pointers can still be accessed by instance variables on the AST nodes, and memory management has been
* explicitly added via destructors. This is the best design decision for this modified codebase in my opinion.
*/

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
	virtual ~ExprAST() {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
public:
	NumberExprAST(double Val) : Val(Val) {}

	double Val;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
	VariableExprAST(string &Name) : Name(Name) {}

	string Name;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
public:
	BinaryExprAST(char op, const ExprAST* LHS, const ExprAST* RHS)
		: Op(op), LHS(LHS), RHS(RHS) {}

	char Op;

	// Kept public so this class can serve as the sole container and owner of the unique_ptr's
	const ExprAST* LHS;
	const ExprAST* RHS;

	~BinaryExprAST() {
		delete LHS;
		delete RHS;
	}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
public:
	CallExprAST(string &Callee, vector<const ExprAST*> Args)
		: Callee(Callee), Args(Args) {}

	string& Callee;

	vector<const ExprAST*> Args;
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
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {

public:
	FunctionAST(const PrototypeAST* Proto, const ExprAST* Body)
		: Proto(Proto), Body(Body) {}

	const ExprAST* Body;

	const PrototypeAST* Proto;

	~FunctionAST() {
		delete Body;
		delete Proto;
	}
};