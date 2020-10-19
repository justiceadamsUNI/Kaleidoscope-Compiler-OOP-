#pragma once
#include <string>

/**
* Changes made by justice: The token enum is now consumed by the Token class which has a public API.
* Also there is now a tok_char enum to differentiate from an tok_identifier
*/

enum TokenType {
	tok_eof = -1,

	// commands
	tok_def = -2,
	tok_extern = -3,

	// primary
	tok_identifier = -4,
	tok_number = -5,
	tok_char = -6,
};

class Token {
public:
	Token(TokenType Type) : Type(Type) {}

	std::string getIdentifierString() { return IdentifierStr; }

	double getNumValue() { return NumVal; }
	
	void setIdentifierString(std::string Identifier) { IdentifierStr = Identifier; }

	void setNumValue(double Value) { NumVal = Value; }

	TokenType getType() { return Type; }
	
private:
	std::string IdentifierStr; // Filled in if tok_identifier
	double NumVal;             // Filled in if tok_number
	TokenType Type;            // Always filled. Desribes token data type.
};