// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-generics-lowering-context.h"
#include "slang-ir-lower-existential.h"
#include "slang-ir-lower-generic-function.h"
#include "slang-ir-lower-generic-call.h"
#include "slang-ir-lower-generic-type.h"
#include "slang-ir-witness-table-wrapper.h"
#include "slang-ir-ssa.h"
#include "slang-ir-dce.h"

namespace Slang
{
    void lowerGenerics(
        IRModule* module,
        DiagnosticSink* sink)
    {
        SharedGenericsLoweringContext sharedContext;
        sharedContext.module = module;
        sharedContext.sink = sink;

        lowerGenericFunctions(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        lowerGenericType(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        lowerExistentials(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        lowerGenericCalls(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        generateWitnessTableWrapperFunctions(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;

        generateAnyValueMarshallingFunctions(&sharedContext);
        if (sink->getErrorCount() != 0)
            return;
        // We might have generated new temporary variables during lowering.
        // An SSA pass can clean up unnecessary load/stores.
        constructSSA(module);
        eliminateDeadCode(module);
    }
} // namespace Slang
