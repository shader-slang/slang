// ir-link.h
#pragma once

#include "compiler.h"

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
    // that is best specialized for the given `targetReq` will be
    // used.
    //
    LinkedIR linkIR(
        BackEndCompileRequest*  compileRequest,
        EntryPoint*             entryPoint,
        ProgramLayout*          programLayout,
        CodeGenTarget           target,
        TargetRequest*          targetReq);
}
