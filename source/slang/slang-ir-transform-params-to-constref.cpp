#include "slang-ir-transform-params-to-constref.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct TransformParamsToConstRefContext
{
    IRModule* module;
    DiagnosticSink* sink;
    IRBuilder builder;
    bool changed = false;

    TransformParamsToConstRefContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink), builder(module)
    {
    }

    // Check if a type should be transformed (struct, array, or other composite types)
    bool shouldTransformParam(IRParam* param)
    {
        auto type = param->getDataType();
        if (!type)
            return false;

        switch (type->getOp())
        {
        case kIROp_StructType:
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_TupleType:
            // valid type, continue to check
            break;
        default:
            return false;
        }

        return true;
    }

    void rewriteValueUsesToAddrUses(IRInst* newAddrInst)
    {
        HashSet<IRInst*> workListSet;
        workListSet.add(newAddrInst);
        List<IRInst*> workList;
        workList.add(newAddrInst);
        auto _addToWorkList = [&](IRInst* inst)
        {
            if (workListSet.add(inst))
                workList.add(inst);
        };
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto inst = workList[i];
            traverseUses(
                inst,
                [&](IRUse* use)
                {
                    auto user = use->getUser();
                    if (workListSet.contains(user))
                        return;
                    switch (user->getOp())
                    {
                    case kIROp_FieldExtract:
                        {
                            // Transform the IRFieldExtract into a IRFieldAddress
                            if (!isUseBaseAddrOperand(use, user))
                                break;
                            auto fieldExtract = as<IRFieldExtract>(user);
                            builder.setInsertBefore(fieldExtract);
                            auto fieldAddr = builder.emitFieldAddress(
                                fieldExtract->getBase(),
                                fieldExtract->getField());
                            fieldExtract->replaceUsesWith(fieldAddr);
                            _addToWorkList(fieldAddr);
                            return;
                        }
                    case kIROp_GetElement:
                        {
                            // Transform the IRGetElement into a IRGetElementPtr
                            if (!isUseBaseAddrOperand(use, user))
                                break;
                            auto getElement = as<IRGetElement>(user);
                            builder.setInsertBefore(getElement);
                            auto elemAddr = builder.emitElementAddress(
                                getElement->getBase(),
                                getElement->getIndex());
                            getElement->replaceUsesWith(elemAddr);
                            _addToWorkList(elemAddr);
                            return;
                        }
                    case kIROp_Store:
                        {
                            // If the current value is being stored into a write-once temp var that
                            // is immediately passed into a constref location in a call, we can get
                            // rid of the temp var and replace it with `inst` directly.
                            // (such temp var can be introduced during `updateCallSites` when we
                            // were processing the callee.)
                            //
                            auto dest = as<IRStore>(user)->getPtr();
                            if (dest->findDecorationImpl(kIROp_TempCallArgImmutableVarDecoration))
                            {
                                user->removeAndDeallocate();
                                dest->replaceUsesWith(inst);
                                dest->removeAndDeallocate();
                                return;
                            }
                            break;
                        }
                    }
                    // Insert a load before the user and replace the user with the load
                    builder.setInsertBefore(user);
                    auto loadInst = builder.emitLoad(inst);
                    use->set(loadInst);
                });
        }
    }

    void rewriteParamUseSitesToSupportConstRefUsage(HashSet<IRParam*>& updatedParams)
    {
        // Traverse the uses of our updated params to rewrite them.
        // Assume a `in` parameter has been converted to a `constref` parameter.
        for (auto param : updatedParams)
        {
            rewriteValueUsesToAddrUses(param);
        }
    }

    // Check if `load` is an `IRLoad(addr)` where `addr` is a immutable location.
    IRInst* isLoadFromImmutableAddress(IRInst* load)
    {
        if (load->getOp() != kIROp_Load)
            return nullptr;
        auto addr = load->getOperand(0);
        auto root = getRootAddr(addr);
        if (!root)
            return nullptr;
        if (!root->getDataType())
            return nullptr;
        switch (root->getDataType()->getOp())
        {
        case kIROp_ConstantBufferType:
        case kIROp_BorrowInParamType:
        case kIROp_ParameterBlockType:
            return addr;
        default:
            // Note that we should in general not assume a read-only StructuredBuffer or
            // a pointer with read-only access as an immutable location due to potential aliasing.
            // We could introduce a compiler flag to turn on optimizations on these buffer types
            // assuming there is no aliasing.
            break;
        }
        return nullptr;
    }

    // Update call sites to pass an address instead of value for each updated-param
    void updateCallSites(IRFunc* func, HashSet<IRParam*>& updatedParams)
    {
        // Find all calls which use `func`.
        List<IRCall*> callsToUpdate;
        traverseUsers<IRCall>(func, [&](IRCall* call) { callsToUpdate.add(call); });

        // Update each call site
        for (auto call : callsToUpdate)
        {
            builder.setInsertBefore(call);
            List<IRInst*> newArgs;

            // Transform arguments to match the updated-parameter
            UInt i = 0;
            for (IRParam* param = func->getFirstParam(); param; param = param->getNextParam(), i++)
            {
                auto arg = call->getArg(i);
                if (!updatedParams.contains(param))
                {
                    newArgs.add(arg);
                    continue;
                }
                if (auto addr = isLoadFromImmutableAddress(arg))
                {
                    // If existing argument is a load from an immutable buffer address,
                    // we can pass in the address as is, without making a temporary copy.
                    newArgs.add(addr);
                }
                else
                {
                    auto tempVar = builder.emitVar(arg->getFullType());
                    builder.addDecoration(tempVar, kIROp_TempCallArgImmutableVarDecoration);
                    builder.emitStore(tempVar, arg);
                    newArgs.add(tempVar);
                }
            }

            // Create new call with updated arguments
            auto newCall = builder.emitCallInst(call->getFullType(), func, newArgs);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }
    }

    // Check if function should be excluded from transformation
    bool shouldProcessFunction(IRFunc* func)
    {
        // Skip functions without definitions
        if (!func->isDefinition())
            return false;

        // Skip if we find any of these decorations
        for (auto decoration : func->getDecorations())
        {
            // Skip functions with target intrinsic decorations.
            // These functions cannot be properly legalized after
            // transformation.
            if (as<IRTargetIntrinsicDecoration>(decoration))
                return false;

            // Skip entry-point and pseudo-entry-point functions
            // since we cannot legalize the input parameters.
            if (as<IREntryPointDecoration>(decoration) || as<IRCudaKernelDecoration>(decoration) ||
                as<IRAutoPyBindCudaDecoration>(decoration))
                return false;

            // Skip functions with CudaDeviceExport decoration.
            // These functions have externally visible signatures that should not be changed.
            if (func->findDecorationImpl(kIROp_CudaDeviceExportDecoration))
                return false;
        }

        // Skip functions with `kIROp_GenericAsm` since
        // these instructions inject target specific code
        // using parameters in an unpredictable way, relying
        // on assumptions that parameters do not change type.
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (!as<IRGenericAsm>(inst))
                    continue;
                return false;
            }
        }

        return true;
    }

    // Process a single function
    void processFunc(IRFunc* func)
    {
        HashSet<IRParam*> updatedParams;

        // If the function is used in any way that is not understood by the
        // compiler, do not modify it.
        // For example, if the function is used as callback, we must preserve
        // its signature.
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (as<IRDecoration>(user))
                continue;
            if (auto call = as<IRCall>(user))
            {
                if (call->getCalleeUse() == use)
                    continue;
            }
            // If we reach here, we encountered a non-call use of the func,
            // we will stop processing.
            return;
        }

        // First pass: Transform parameter types
        for (auto param = func->getFirstParam(); param; param = param->getNextParam())
        {
            if (shouldTransformParam(param))
            {
                // Our goal here is to transform `in T` parameters to `borrow in T`.
                // We are selective about what we will transform for a few reasons:
                // 1. no reason to transform simple primitives like `int`.
                // 2. not every type makes sense as constref. For example, `ParameterBlock`.
                // 3. `borrow in` is not 100% stable, so we need to be selective on what we let
                //    transform into `borrow in`.
                //
                // This allows us to pass the address of variables directly into a function,
                // giving us the choice to remove copies into a parameter.
                auto paramType = param->getDataType();
                auto constRefType = builder.getBorrowInParamType(paramType, AddressSpace::Generic);
                param->setFullType(constRefType);

                changed = true;
                updatedParams.add(param);
            }
        }

        if (updatedParams.getCount() == 0)
        {
            return;
        }

        fixUpFuncType(func);

        // Second pass: Update function body according to the new `constref` parameters
        rewriteParamUseSitesToSupportConstRefUsage(updatedParams);

        // Third pass: Update call sites
        updateCallSites(func, updatedParams);
    }

    void addFuncsToCallListInTopologicalOrder(
        IRFunc* root,
        List<IRFunc*>& functionsToProcess,
        HashSet<IRFunc*>& visitedCandidates)
    {
        // We added 'root' already, leave
        if (visitedCandidates.contains(root))
            return;

        visitedCandidates.add(root);

        for (auto block : root->getBlocks())
        {
            for (auto blockInst : block->getChildren())
            {
                auto call = as<IRCall>(blockInst);
                if (!call)
                    continue;

                auto callee = as<IRFunc>(call->getCallee());
                if (!callee)
                    continue;

                addFuncsToCallListInTopologicalOrder(callee, functionsToProcess, visitedCandidates);
            }
        }

        if (!shouldProcessFunction(root))
            return;
        functionsToProcess.add(root);
    }

    SlangResult processModule()
    {
        // Collect all functions that need processing.
        // Process all callee's before callers; otherwise we introduce bugs

        HashSet<IRFunc*> visitedCandidates;
        List<IRFunc*> functionsToProcess;
        for (auto inst = module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;
            addFuncsToCallListInTopologicalOrder(func, functionsToProcess, visitedCandidates);
        }

        // Process each function
        for (auto func : functionsToProcess)
        {
            processFunc(func);
        }

        return SLANG_OK;
    }
};

SlangResult transformParamsToConstRef(IRModule* module, DiagnosticSink* sink)
{
    TransformParamsToConstRefContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
