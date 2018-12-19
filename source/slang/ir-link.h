// ir-link.h
#pragma once

#include "compiler.h"

namespace Slang
{
    // Interface to IR specialization for use when cloning target-specific
    // IR as part of compiling an entry point.

    // `IRSpecializationState` is used as an opaque type to wrap up all
    // the data needed to perform IR specialization, without exposing
    // implementation details.
    struct IRSpecializationState;
    IRSpecializationState* createIRSpecializationState(
        EntryPointRequest*  entryPointRequest,
        ProgramLayout*      programLayout,
        CodeGenTarget       target,
        TargetRequest*      targetReq);
    void destroyIRSpecializationState(IRSpecializationState* state);
    IRModule* getIRModule(IRSpecializationState* state);

    struct ExtensionUsageTracker;

    // Clone the IR values reachable from the given entry point
    // into the IR module associated with the specialization state.
    // When multiple definitions of a symbol are found, the one
    // that is best specialized for the given `targetReq` will be
    // used.
    IRFunc* specializeIRForEntryPoint(
        IRSpecializationState*  state,
        EntryPointRequest*  entryPointRequest);
}
