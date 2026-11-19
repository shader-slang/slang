// slang-ir-specialize-buffer-load-arg.cpp
#include "slang-ir-specialize-buffer-load-arg.h"

#include "slang-ir-defer-buffer-load.h"
#include "slang-ir-specialize-function-call.h"
#include "slang/ir/slang-ir-insts.h"
#include "slang/ir/slang-ir-layout.h"
#include "slang/ir/slang-ir-util.h"
#include "slang/ir/slang-ir.h"

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

    CodeGenContext* codegenContext;

    virtual bool doesParamWantSpecialization(IRParam* param, IRInst* arg, IRCall* callInst)
    {
        // We only want to specialize for `struct` types and not base types.
        //
        auto paramType = (IRType*)unwrapAttributedType(param->getDataType());
        if (!isTypePreferrableToDeferLoad(codegenContext, paramType))
            return false;

        // We want to handle loads from arbitrary access chains rooting from a shader parameter.
        //
        IRInst* a = arg;
        for (;;)
        {
            // A user pointer can be directly passed into the function, so we no
            // longer need to trace up further.
            if (isUserPointerType(a->getDataType()))
                break;

            if (auto argGetElement = as<IRGetElement>(a))
            {
                a = argGetElement->getBase();
            }
            else if (auto argSbLoad = as<IRStructuredBufferLoad>(a))
            {
                a = argSbLoad->getOperand(0);
            }
            else if (as<IRByteAddressBufferLoad>(a))
            {
                // We leave a load from a byte-address buffer at the call site, and pass the
                // value that it loads to the callee, for two reasons:
                //
                // - A load that moves into a callee takes its offset from a parameter. The
                //   byte-address legalizer reads a vector or an array with a single access
                //   only when it can prove that the access is aligned, from an explicit
                //   alignment or from a literal offset, and reads it in smaller pieces
                //   otherwise.
                //
                // - Moving a load into a callee sinks it past the instructions between it and
                //   the call, and those can write the buffer when it is writable.
                //
                return false;
            }
            else if (auto argFieldExtract = as<IRFieldExtract>(a))
            {
                a = argFieldExtract->getBase();
            }
            else if (auto argGetElementPtr = as<IRGetElementPtr>(a))
            {
                a = argGetElementPtr->getBase();
            }
            else if (auto argSBGetElementPtr = as<IRRWStructuredBufferGetElementPtr>(a))
            {
                a = argSBGetElementPtr->getBase();
            }
            else if (auto argFieldAddr = as<IRFieldAddress>(a))
            {
                a = argFieldAddr->getBase();
            }
            else if (auto argLoad = as<IRLoad>(a))
            {
                a = argLoad->getPtr();

                // We can safely defer a load to the callee if the source dest is immutable.
                if (isPointerToImmutableLocation(getRootAddr(a)))
                    continue;

                // Otherwise, we check if there is no other instructions in between the load and the
                // call that can modify the memory location. If so, we can still safely defer the
                // load to the callee.
                if (!isMemoryLocationUnmodifiedBetweenLoadAndUser(
                        codegenContext->getTargetReq(),
                        argLoad,
                        callInst))
                    return false;
            }
            else
            {
                break;
            }
        }

        // The "root" of the parameter must be one of the following:
        // 1. A reference to a global-scope shader parameter that can be referenced directly from
        //    the callee.
        // 2. A user pointer or bindless resource handle that can be passed to the callee as
        //    ordinary argument.
        //
        if (const auto argGlobalParam = as<IRGlobalParam>(a); argGlobalParam)
        {
            return true;
        }
        else if (isUserPointerType(a->getDataType()) || as<IRCastDescriptorHandleToResource>(a))
        {
            return true;
        }
        return false;
    }
};

void specializeFuncsForBufferLoadArgs(IRModule* module, CodeGenContext* codegenContext)
{
    FuncBufferLoadSpecializationCondition condition;
    condition.codegenContext = codegenContext;
    specializeFunctionCalls(codegenContext, module, &condition);
}

} // namespace Slang
