#include "slang-ir-fix-entrypoint-callsite.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
// An entry point has a launch ABI that later legalization may change. When the program also calls
// the entry point as an ordinary function, we lazily create an ordinary-function clone and redirect
// every direct call to it. Later passes can then change the original entry point without having to
// adapt ordinary call sites to its launch ABI.
void fixEntryPointCallsites(IRFunc* entryPoint)
{
    IRFunc* clonedEntryPointForCall = nullptr;

    // We defer cloning until we find the first direct call. An entry point that is only launched
    // does not need an ordinary-function copy.
    auto ensureClonedEntryPointForCall = [&]() -> IRFunc*
    {
        if (clonedEntryPointForCall)
            return clonedEntryPointForCall;
        IRCloneEnv cloneEnv;
        IRBuilder builder(entryPoint);
        builder.setInsertBefore(entryPoint);
        clonedEntryPointForCall = (IRFunc*)cloneInst(&cloneEnv, &builder, entryPoint);
        // The clone is intended to be reached only by the direct calls that we redirect below. We
        // remove the copied launch metadata and the linkage and retention decorations listed below.
        // The redirected calls themselves keep the clone reachable.
        List<IRInst*> decorsToRemove;
        for (auto decor : clonedEntryPointForCall->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_EntryPointDecoration:
            case kIROp_CudaKernelDecoration:
            case kIROp_LayoutDecoration:
            case kIROp_NumThreadsDecoration:
            case kIROp_ImportDecoration:
            case kIROp_ExportDecoration:
            case kIROp_UserExternDecoration:
            case kIROp_PublicDecoration:
            case kIROp_KeepAliveDecoration:
            case kIROp_HLSLExportDecoration:
            case kIROp_DllImportDecoration:
            case kIROp_DllExportDecoration:
            case kIROp_ExternCDecoration:
            case kIROp_ExternCppDecoration:
            case kIROp_CudaDeviceExportDecoration:
            case kIROp_DownstreamModuleExportDecoration:
            case kIROp_DownstreamModuleImportDecoration:
                decorsToRemove.add(decor);
                break;
            }
        }
        for (auto decor : decorsToRemove)
            decor->removeAndDeallocate();
        return clonedEntryPointForCall;
    };
    // We redirect only ordinary calls. Launch instructions and metadata continue to refer to the
    // original entry point and its launch ABI.
    traverseUses(
        entryPoint,
        [&](IRUse* use)
        {
            auto user = use->getUser();
            auto call = as<IRCall>(user);
            if (!call)
                return;

            // An `IRCall` may refer to the entry point either as its callee or as an argument. We
            // redirect the call only when this exact use is its callee. If the use were an
            // argument, writing operand zero below would replace an unrelated callee.
            if (call->getCalleeUse() != use)
                return;

            auto callee = ensureClonedEntryPointForCall();
            call->setOperand(0, callee);

            // Fix up argument types: if the callee entrypoint is expecting a constref
            // and the caller is passing a value, we need to wrap the value in a temporary var
            // and pass the temporary var.
            //
            // TODO(tfoley): Wait, what? The situation this code is trying to fix should
            // never be allowed to occur in the first place. This code shouldn't be
            // trying to defend against the bad input; instead we should be *fixing*
            // the source of the problem.
            //
            auto funcType = as<IRFuncType>(callee->getDataType());
            SLANG_ASSERT(funcType);
            IRBuilder builder(call);
            builder.setInsertBefore(call);
            List<IRParam*> params;
            for (auto param : callee->getParams())
                params.add(param);
            if ((UInt)params.getCount() != call->getArgCount())
                return;
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                auto paramType = params[i]->getDataType();
                auto arg = call->getArg(i);
                if (auto refType = as<IRBorrowInParamType>(paramType))
                {
                    if (!as<IRPtrTypeBase>(arg->getDataType()))
                    {
                        auto tempVar = builder.emitVar(refType->getValueType());
                        builder.emitStore(tempVar, arg);
                        call->setArg(i, tempVar);
                    }
                }
            }
        });
}

void fixEntryPointCallsites(IRModule* module)
{
    // We process both shader entry points and CUDA kernels. A `[CUDAKernel]` function has a launch
    // ABI even when it has no shader-entry-point decoration, so its ordinary callers need the same
    // split.
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->findDecoration<IREntryPointDecoration>() ||
            globalInst->findDecoration<IRCudaKernelDecoration>())
            fixEntryPointCallsites((IRFunc*)globalInst);
    }
}

} // namespace Slang
