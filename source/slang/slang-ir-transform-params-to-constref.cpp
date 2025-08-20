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
        case kIROp_VectorType:
        case kIROp_MatrixType:
        case kIROp_TupleType:
        case kIROp_CoopVectorType:
            // valid type, continue to check
            break;
        default:
            return false;
        }

        return true;
    }

    void rewriteParamUseSitesToSupportConstRefUsage(HashSet<IRParam*>& updatedParams)
    {
        // Traverse the uses of our updated params to rewrite them.
        // Assume a `in` parameter has been converted to a `constref` parameter.
        for (auto param : updatedParams)
        {
            traverseUses(
                param,
                [&](IRUse* use)
                {
                    auto user = use->getUser();
                    switch (user->getOp())
                    {
                    case kIROp_FieldExtract:
                        {
                            // Transform the IRFieldExtract into a IRFieldAddress
                            auto fieldExtract = as<IRFieldExtract>(use->getUser());
                            builder.setInsertBefore(fieldExtract);
                            auto fieldAddr = builder.emitFieldAddress(
                                fieldExtract->getBase(),
                                fieldExtract->getField());
                            auto loadInst = builder.emitLoad(fieldAddr);
                            fieldExtract->replaceUsesWith(loadInst);
                            fieldExtract->removeAndDeallocate();
                            break;
                        }
                    case kIROp_GetElement:
                        {
                            // Transform the IRGetElement into a IRGetElementPtr
                            auto getElement = as<IRGetElement>(use->getUser());

                            builder.setInsertBefore(getElement);
                            auto elemAddr = builder.emitElementAddress(
                                getElement->getBase(),
                                getElement->getIndex());
                            auto loadInst = builder.emitLoad(elemAddr);
                            getElement->replaceUsesWith(loadInst);
                            getElement->removeAndDeallocate();
                            break;
                        }
                    default:
                        {
                            // Insert a load before the user and replace the user with the load
                            builder.setInsertBefore(user);
                            auto loadInst = builder.emitLoad(param);
                            use->set(loadInst);
                            break;
                        }
                    }
                });
        }
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
            IRParam* param = func->getFirstParam();
            UInt i = 0;
            auto iterate = [&]()
            {
                param = param->getNextParam();
                i++;
            };
            for (; param; iterate())
            {
                auto arg = call->getArg(i);
                if (!updatedParams.contains(param))
                {
                    newArgs.add(arg);
                    continue;
                }

                auto tempVar = builder.emitVar(arg->getFullType());
                builder.emitStore(tempVar, arg);
                newArgs.add(tempVar);
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
        bool hasTransformedParams = false;

        // First pass: Transform parameter types
        for (auto param = func->getFirstParam(); param; param = param->getNextParam())
        {
            if (shouldTransformParam(param))
            {
                // Our goal here is to transform `in T` parameters to const-ref.
                // We are selective about what we will transform for a few reasons:
                // 1. no reason to transform simple primitives like `int`.
                // 2. not every type makes sense as constref. For example, `ParameterBlock`.
                // 3. constref is not 100% stable, so we need to be selective on what we let
                //    transform into constref.
                //
                // This allows us to pass the address of variables directly into a function,
                // giving us the choice to remove copies into a parameter.
                auto paramType = param->getDataType();
                auto constRefType = builder.getConstRefType(paramType, AddressSpace::ThreadLocal);
                param->setFullType(constRefType);

                hasTransformedParams = true;
                changed = true;
                updatedParams.add(param);
            }
        }

        if (!hasTransformedParams)
        {
            return;
        }

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
