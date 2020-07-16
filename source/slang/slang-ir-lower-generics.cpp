// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-lower-generic-function.h"
#include "slang-ir-lower-generic-call.h"
#include "slang-ir-lower-generic-var.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir-ssa.h"
#include "slang-ir-dce.h"

namespace Slang
{
    void lowerGenerics(
        IRModule* module)
    {
        SharedGenericsLoweringContext sharedContext;
        sharedContext.module = module;
        lowerGenericFunctions(&sharedContext);
        lowerGenericCalls(&sharedContext);
        // We might have generated new temporary variables during lowering.
        // An SSA pass can clean up unncessary load/stores.
        constructSSA(module);
        eliminateDeadCode(module);
        lowerGenericVar(&sharedContext);
        // After lowerGenericVar, there could be some unused `undef` values.
        // We eliminate them in a DCE pass.
        eliminateDeadCode(module);
        generateWitnessTableWrapperFunctions(&sharedContext);
    }
} // namespace Slang
