// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-augment-make-existential.h"
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

        // Replace all `makeExistential` insts with `makeExistentialWithRTTI`
        // before making any other changes. This is necessary because a parameter of
        // generic type will be lowered into `AnyValueType`, and after that we can no longer
        // access the original generic type parameter from the lowered parameter value.
        // This steps ensures that the generic type parameter is available via an
        // explicit operand in `makeExistentialWithRTTI`, so that type parameter
        // can be translated into an RTTI object during `lower-generic-type`,
        // and used to create a tuple representing the existential value.
        augmentMakeExistentialInsts(module);

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
