// slang-ir-link.h
#pragma once

#include "slang-compiler.h"

namespace Slang
{
    struct LinkedIR
    {
        RefPtr<IRModule>    module;
        IRFunc*             entryPoint;
    };


    // Clone the IR values reachable from the given entry point
    // into the IR module associated with the specialization state.
    // When multiple definitions of a symbol are found, the one
    // that is best specialized for the appropriate compilation
    // target will be used.
    //
    LinkedIR linkIR(
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        CodeGenTarget           target,
        TargetProgram*          targetProgram);

    // Replace any global constants in the IR module with their
    // definitions, if possible.
    //
    // This pass should always be run shortly after linking the
    // IR, to ensure that constants with identical values are
    // treated as identical for the purposes of specialization.
    //
    void replaceGlobalConstants(IRModule* module);
}
