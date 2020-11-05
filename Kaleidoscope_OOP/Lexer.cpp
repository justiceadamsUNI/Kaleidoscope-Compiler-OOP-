#include "stdafx.h"
#include "Lexer.h"

/**
* Changes made by justice: getToken() returns a Token object which wraps the enumeration in an object with a public
* set of functions. This as opposed to returning the enum itself like in the tutorial. All elements of the Token 
* objects are encapsulated properly.
*/

Token Lexer::getToken()
{
	// Skip any whitespace.
	while (isspace(LastChar))
		LastChar = getchar();

	if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
		std::string IdentifierStr;
		IdentifierStr = LastChar;

		while (isalnum((LastChar = getchar())))
			IdentifierStr += LastChar;

		if (IdentifierStr == "def")
			return Token(TokenType::tok_def);

		if (IdentifierStr == "extern")
			return Token(TokenType::tok_extern);

		// Create a token with the Identifier string encapsulated within
		Token _Token = Token(TokenType::tok_identifier);
		_Token.setIdentifierString(IdentifierStr);
		return _Token;
	}

	if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
		std::string NumStr;
		do {
			NumStr += LastChar;
			LastChar = getchar();
		} while (isdigit(LastChar) || LastChar == '.');

		int NumVal = strtod(NumStr.c_str(), nullptr);
		Token _Token = Token(TokenType::tok_number);
		_Token.setNumValue(NumVal);
		return _Token;
	}

	if (LastChar == '#') {
		// Comment until end of line.
		do
			LastChar = getchar();
		while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF)
			return getToken();
	}

	// Check for end of file.  Don't eat the EOF.
	if (LastChar == EOF)
		return Token(TokenType::tok_eof);

	// Otherwise, just return the character as its ascii value.
	int ThisChar = LastChar;
	LastChar = getchar();
	Token _Token = Token(TokenType::tok_char);
	_Token.setNumValue(ThisChar);
	return _Token;
}
