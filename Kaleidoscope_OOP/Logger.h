#pragma once
#include "stdafx.h"
#include <stdio.h>

/**
* Changes made by justice: One single logging header-only module is used to remove duplication and
* make things simpler to log in an object-oriented codebase.
*/

const void LogError(const char* Str)
{
	fprintf(stderr, "Error: %s\n", Str);
}

const void LogErrorP(const char* Str)
{
	LogError(Str);
}
