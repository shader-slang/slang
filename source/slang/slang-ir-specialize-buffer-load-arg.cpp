// slang-ir-specialize-buffer-load-arg.cpp
#include "slang-ir-specialize-buffer-load-arg.h"

#include "slang-ir-defer-buffer-load.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// This file implements a pass that translates function call sites where
// the result of a buffer load from a global shader parameter (e.g., a
// global constant buffer) is being passed through to the callee. It
// replaces those with calls to specialized callee functions that directly
// reference the chosen global.
//
// As swith most of our IR passes, we encapsulate the logic here in a context
// type so that the data that needs to be shared throughout the pass can
// be conveniently scoped.
//

// Note that this pass also ensures other more contrived cases are properly
// handled. For example:
//
// * A load of a large structure from field in a constant buffer, so that
//   the value loaded is not the entire buffer contents.
//
// * A load of a large structure from a structured buffer, or any other kind
//   of buffer that requires an index.
//

struct FuncBufferLoadSpecializationCondition : FunctionCallSpecializeCondition
{
    typedef FunctionCallSpecializeCondition Super;

    // Generally, we want to specialize arguments that are large in size, or arguments that
    // are arrays or composite type that contains arrays.
    // This is because:
    // 1. Struct types without arrays will eventually be SROA's into registers and then effectively
    //    DCE'd, so they usually won't cause performance issues. In fact, front loading structs
    //    and reusing the loaded value instead of repetitively loading from constant memory is
    //    usually beneficial to performance. However large struct values can be SROA'd into a large
    //    number of registers, causing slow downstream compilation. Therefore we should avoid/defer
    //    loading them into registers if we can.
    // 2. Arrays usually cannot be SROA'd into individual registers, which usually leads to
    //    large register consumption if they ever get loaded, so we want to defer loading array
    //    typed values as much as possible.

    // If the argument data is bigger than this threshold, it is considered a large object
    // and we will try to specialize it even if it doesn't contain arrays.
    static const int kBufferLoadElementSizeSpecializationThreshold = 256;

    // If the argument data is smaller than this threshold, it is considered a tiny object
    // and we will not consider specializing it, even if it contains arrays.
    static const int kBufferLoadElementSizeSpecializationMinThreshold = 16;

    CodeGenContext* codegenContext;

    virtual bool doesParamWantSpecialization(IRParam* param, IRInst* arg)
    {
        // We only want to specialize for `struct` types and not base types.
        //
        auto paramType = (IRType*)unwrapAttributedType(param->getDataType());
        if (!isTypePreferrableToDeferLoad(codegenContext, paramType))
            return false;

        // We want to handle loads from arbitrary access chains rooting from a shader parameter.
        //
        IRInst* a = arg;
        bool seenLoad = false;
        for (;;)
        {
            if (auto argGetElement = as<IRGetElement>(a))
            {
                a = argGetElement->getBase();
            }
            else if (auto argFieldExtract = as<IRFieldExtract>(a))
            {
                a = argFieldExtract->getBase();
            }
            else if (auto argGetElementPtr = as<IRGetElementPtr>(a))
            {
                a = argGetElementPtr->getBase();
            }
            else if (auto argFieldAddr = as<IRFieldAddress>(a))
            {
                a = argFieldAddr->getBase();
            }
            else if (auto argLoad = as<IRLoad>(a))
            {
                // For now, we can only handle one level of dereference.
                if (seenLoad)
                    return false;
                a = argLoad->getPtr();
                seenLoad = true;
            }
            else
            {
                break;
            }
        }

        // The "root" of the parameter must be a reference to a global-scope
        // shader parameter, so that we know we can substitute it into the callee.
        //
        if (const auto argGlobalParam = as<IRGlobalParam>(a))
        {
            // We can only specialize if the buffer is immutable.
            if (isImmutableLocation(argGlobalParam))
                return true;
        }
        return false;
    }
};

void specializeFuncsForBufferLoadArgs(CodeGenContext* codegenContext, IRModule* module)
{
    FuncBufferLoadSpecializationCondition condition;
    condition.codegenContext = codegenContext;
    specializeFunctionCalls(codegenContext, module, &condition);
}

} // namespace Slang
