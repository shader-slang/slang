// slang-ir-diff-jvp.h
#pragma once

#include "slang-ir.h"
#include "slang-compiler.h"

namespace Slang
{
    struct IRModule;

    struct IRJVPDerivativePassOptions
    {
        // Nothing for now..
    };

    bool processForwardDifferentiableFuncs(
        IRModule*                           module,
        DiagnosticSink*                     sink,
        IRJVPDerivativePassOptions const&   options = IRJVPDerivativePassOptions());

    void stripAutoDiffDecorations(IRModule* module);
}
