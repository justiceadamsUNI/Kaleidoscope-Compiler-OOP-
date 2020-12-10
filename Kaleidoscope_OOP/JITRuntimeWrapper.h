#pragma once
#include <memory>
#include "KaleidoscopeJIT.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

using namespace std;
using namespace llvm;

class JITRuntimeWrapper {
public:
	JITRuntimeWrapper();

	unique_ptr<orc::KaleidoscopeJIT> TheJIT;

	 ExitOnError ExitOnError;
};
