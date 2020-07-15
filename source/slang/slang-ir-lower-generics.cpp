// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-lower-generic-function.h"
#include "slang-ir-lower-generic-call.h"
#include "slang-ir-lower-generic-var.h"
#include "slang-ir-witness-table-wrapper.h"

namespace Slang
{
    void lowerGenerics(
        IRModule* module)
    {
        SharedGenericsLoweringContext sharedContext;
        sharedContext.module = module;
        lowerGenericFunctions(&sharedContext);
        lowerGenericCalls(&sharedContext);
        lowerGenericVar(&sharedContext);
        generateWitnessTableWrapperFunctions(&sharedContext);
    }
} // namespace Slang
