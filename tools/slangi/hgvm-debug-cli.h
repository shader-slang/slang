#pragma once

#include "slang-hgvm/slang-hgvm-debug-session.h"

namespace SlangITool
{
SlangResult runHGVMDebugCLI(
    Slang::HGVM::DebugSession* session,
    Slang::HGVM::Program* program,
    const char* entryPoint,
    Slang::String& error);
}
