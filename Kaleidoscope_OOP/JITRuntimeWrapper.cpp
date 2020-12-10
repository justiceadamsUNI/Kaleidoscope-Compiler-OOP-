#include "stdafx.h"
#include "JITRuntimeWrapper.h"

JITRuntimeWrapper::JITRuntimeWrapper()
{
	// Initialize JIT Runtime for interpretation. Based on the ORC engine.
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

	TheJIT = ExitOnError(orc::KaleidoscopeJIT::Create());
}
