#pragma once
#include <string>
#include "Token.h"

using namespace std;

class Lexer {
public:
	Token getToken();

private:
	static int LastChar;
};