// Kaleidoscope_OOP.cpp : Defines the entry point for the compiler application.
//

#include "stdafx.h"
#include "Lexer.h"
#include "Parser.h"

/**
* Changes made by justice : There is no need to prime a token here. 
* That is done within the MainLoop of the parser.
*/

int main()
{
	Lexer Scanner;

	// Create a scanner drivven Parser 
	Parser _Parser(Scanner);

	// Install standard binary operators.
	// 1 is lowest precedence.
	_Parser.BinopPrecedence['<'] = 10;
	_Parser.BinopPrecedence['+'] = 20;
	_Parser.BinopPrecedence['-'] = 20;
	_Parser.BinopPrecedence['*'] = 40; // highest.

	fprintf(stderr, "ready> ");

	// Run the main "interpreter loop" now.
	_Parser.MainLoop();

	// Print out all of the generated code.
	_Parser.PrintLLIRModule();

	return 0;
}

