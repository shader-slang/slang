#include "slang-ir-struct-param-to-constref.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct StructParamToConstRefContext
{
    IRModule* module;
    DiagnosticSink* sink;
    IRBuilder builder;
    bool changed = false;

    StructParamToConstRefContext(IRModule* module, DiagnosticSink* sink)
        : module(module), sink(sink), builder(module)
    {
    }

    // Check if a function is differentiable (has autodiff decorations)
    bool isDifferentiableFunc(IRFunc* func)
    {
        for (auto decoration : func->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_ForwardDifferentiableDecoration:
            case kIROp_BackwardDifferentiableDecoration:
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDerivativePrimalDecoration:
            case kIROp_UserDefinedBackwardDerivativeDecoration:
                return true;
            default:
                break;
            }
        }
        return false;
    }

    // Check if a type should be transformed (struct, array, or other composite types)
    bool shouldTransformParamType(IRType* type)
    {
        if (!type)
            return false;

        switch (type->getOp())
        {
        case kIROp_StructType:
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
            // case kIROp_VectorType:     // ADD
            // case kIROp_MatrixType:     // ADD
            // case kIROp_TupleType:      // ADD
            // case kIROp_CoopVectorType: // ADD (if targeting cooperative operations)
            return true;
        default:
            return false;
        }
    }

    // Transform a function parameter from struct to ConstRef<struct>
    IRParam* transformParam(IRParam* param)
    {
        auto paramType = param->getDataType();
        if (!shouldTransformParamType(paramType))
            return param;

        // Create ConstRef<T> type as specified
        auto constRefType = builder.getConstRefType(paramType);

        // Replace the parameter type directly instead of creating new parameter
        param->setFullType(constRefType);
        return param;
    }

    // Transform use of a ConstRef parameter in field extract
    void transformFieldExtractUse(IRBuilder& transformBuilder, IRUse* use, List<IRInst*>& workList)
    {
        auto fieldExtract = as<IRFieldExtract>(use->getUser());
        auto param = as<IRParam>(use->get());

        transformBuilder.setInsertBefore(fieldExtract);
        auto fieldAddr = transformBuilder.emitFieldAddress(param, fieldExtract->getField());
        auto loadInst = transformBuilder.emitLoad(fieldAddr);

        // Add newly created instructions to worklist for cascading transformations
        workList.add(fieldAddr);
        workList.add(loadInst);

        fieldExtract->replaceUsesWith(loadInst);
        fieldExtract->removeAndDeallocate();
        changed = true;
    }

    // Transform use of a ConstRef parameter in get element
    void transformGetElementUse(IRBuilder& transformBuilder, IRUse* use, List<IRInst*>& workList)
    {
        auto getElement = as<IRGetElement>(use->getUser());
        auto param = as<IRParam>(use->get());

        transformBuilder.setInsertBefore(getElement);
        auto elemAddr = transformBuilder.emitElementAddress(param, getElement->getIndex());
        auto loadInst = transformBuilder.emitLoad(elemAddr);

        // Add newly created instructions to worklist for cascading transformations
        workList.add(elemAddr);
        workList.add(loadInst);

        getElement->replaceUsesWith(loadInst);
        getElement->removeAndDeallocate();
        changed = true;
    }

    // Transform direct use of a ConstRef parameter (needs load)
    void transformDirectUse(IRBuilder& transformBuilder, IRUse* use)
    {
        auto user = use->getUser();
        auto param = as<IRParam>(use->get());

        // Skip decorations and other non-value uses
        if (as<IRDecoration>(user))
            return;

        transformBuilder.setInsertBefore(user);
        auto loadInst = transformBuilder.emitLoad(param);

        // Replace this specific use with the load
        use->set(loadInst);
        changed = true;
    }

    // Update function body using robust worklist pattern from eliminateAddressInstsImpl
    void updateFunctionBody(Dictionary<IRParam*, IRParam*>& paramMap, IRFunc* func)
    {
        if (paramMap.getCount() == 0)
            return;

        IRBuilder functionBuilder(module);
        List<IRInst*> workList;

        // Collect all instructions that need processing (similar to eliminateAddressInstsImpl)
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                // Add transformed parameters to worklist
                if (auto param = as<IRParam>(inst))
                {
                    if (paramMap.containsKey(param))
                    {
                        workList.add(inst);
                    }
                }
                // Add address-based instructions that might operate on transformed parameters
                else if (
                    inst->getOp() == kIROp_FieldAddress || inst->getOp() == kIROp_GetElementPtr ||
                    inst->getOp() == kIROp_FieldExtract || inst->getOp() == kIROp_GetElement)
                {
                    auto rootAddr = getRootAddr(inst);
                    if (auto rootParam = as<IRParam>(rootAddr))
                    {
                        if (paramMap.containsKey(rootParam))
                        {
                            workList.add(inst);
                        }
                    }
                }
            }
        }

        // Process worklist with index-based iteration to handle cascading transformations
        for (Index workListIndex = 0; workListIndex < workList.getCount(); workListIndex++)
        {
            auto inst = workList[workListIndex];

            // Process all uses of this instruction
            for (auto use = inst->firstUse; use;)
            {
                auto nextUse = use->nextUse;
                auto user = use->getUser();

                // Skip decorations and other non-value uses
                if (as<IRDecoration>(user))
                {
                    use = nextUse;
                    continue;
                }

                IRBuilder transformBuilder(module);
                IRBuilderSourceLocRAII sourceLocationScope(&transformBuilder, user->sourceLoc);

                switch (user->getOp())
                {
                case kIROp_FieldExtract:
                    if (auto param = as<IRParam>(use->get()))
                    {
                        if (paramMap.containsKey(param))
                        {
                            transformFieldExtractUse(transformBuilder, use, workList);
                        }
                    }
                    break;
                case kIROp_GetElement:
                    if (auto param = as<IRParam>(use->get()))
                    {
                        if (paramMap.containsKey(param))
                        {
                            transformGetElementUse(transformBuilder, use, workList);
                        }
                    }
                    break;
                default:
                    // For direct parameter uses, insert a load
                    if (auto param = as<IRParam>(use->get()))
                    {
                        if (paramMap.containsKey(param))
                        {
                            transformDirectUse(transformBuilder, use);
                        }
                    }
                    break;
                }

                use = nextUse;
            }
        }
    }

    // Update call sites to pass addresses instead of values
    void updateCallSites(
        IRFunc* originalFunc,
        IRFunc* newFunc,
        Dictionary<IRParam*, IRParam*>& /*paramMap*/)
    {
        // Find all calls to the original function (collect first to avoid iterator invalidation)
        List<IRCall*> callsToUpdate;

        for (auto use = originalFunc->firstUse; use; use = use->nextUse)
        {
            if (auto call = as<IRCall>(use->getUser()))
            {
                if (call->getCallee() == originalFunc)
                {
                    callsToUpdate.add(call);
                }
            }
        }

        // Update each call site
        for (auto call : callsToUpdate)
        {
            builder.setInsertBefore(call);
            List<IRInst*> newArgs;

            // Transform arguments to match the new parameter types
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                auto arg = call->getArg(i);
                auto argType = arg->getDataType();

                if (shouldTransformParamType(argType))
                {
                    // For ConstRef parameters, we need to pass address
                    // Handle different argument patterns according to ConstRef semantics

                    if (auto loadInst = as<IRLoad>(arg))
                    {
                        // If argument is a load, pass the address being loaded from
                        // This handles: f(load(addr)) -> f(addr)
                        auto sourceAddr = loadInst->getPtr();

                        // Pass the address directly
                        newArgs.add(sourceAddr);
                    }
                    else if (as<IRFieldExtract>(arg))
                    {
                        // For non-addressable field access, create temporary
                        // This handles: f(s.field) -> { temp = s.field; f(&temp); }
                        auto tempVar = builder.emitVar(arg->getFullType());
                        builder.emitStore(tempVar, arg);
                        newArgs.add(tempVar);
                    }
                    else if (as<IRGetElement>(arg))
                    {
                        // For non-addressable element access, create temporary
                        // This handles: f(arr[i]) -> { temp = arr[i]; f(&temp); }
                        auto tempVar = builder.emitVar(arg->getFullType());
                        builder.emitStore(tempVar, arg);
                        newArgs.add(tempVar);
                    }
                    else if (
                        argType && (argType->getOp() == kIROp_PtrType ||
                                    argType->getOp() == kIROp_ConstRefType))
                    {
                        // Already an address/reference, use directly
                        newArgs.add(arg);
                    }
                    else
                    {
                        // For other cases (non-addressable values), create temporary
                        // This handles: f(expr) -> { temp = expr; f(&temp); }
                        auto tempVar = builder.emitVar(arg->getFullType());
                        builder.emitStore(tempVar, arg);
                        newArgs.add(tempVar);
                    }
                }
                else
                {
                    newArgs.add(arg);
                }
            }


            // Create new call with updated arguments
            auto newCall = builder.emitCallInst(call->getFullType(), newFunc, newArgs);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
            changed = true;
        }
    }

    bool shouldProcessFunction(IRFunc* func)
    {
        // Only process functions that have method decorations
        if (!func->findDecoration<IRMethodDecoration>())
            return false;

        // Skip constructor functions (they have special initialization semantics)
        if (func->findDecoration<IRConstructorDecoration>())
            return false;

        // Skip functions that are already handled by shouldSkipFunction
        if (shouldSkipFunction(func))
            return false;

        return true;
    }
    // Check if function should be excluded from transformation
    bool shouldSkipFunction(IRFunc* func)
    {
        // Skip functions with readNone decoration (pure utility functions)
        if (func->findDecoration<IRReadNoneDecoration>())
            return true;

        // Skip functions with target intrinsic decorations (backend-specific functions)
        if (func->findDecoration<IRTargetIntrinsicDecoration>())
            return true;

        // Skip functions without definitions (external/intrinsic functions)
        if (!func->isDefinition())
            return true;

        // Skip entry point functions (interface with runtime)
        if (func->findDecoration<IREntryPointDecoration>())
            return true;

        // Skip CUDA kernel functions (marked with [CudaKernel])
        if (func->findDecoration<IRCudaKernelDecoration>())
            return true;

        // Skip differentiable functions (they have special ConstRef semantics)
        if (isDifferentiableFunc(func))
            return true;

        // Skip backward derivative propagate functions (special autodiff-generated functions)
        if (func->findDecoration<IRBackwardDerivativePropagateDecoration>())
            return true;

        // Skip constructor functions (they have special semantics)
        if (func->findDecoration<IRConstructorDecoration>())
            return true;

        return false;
    }

    // Process a single function
    void processFunc(IRFunc* func)
    {
        if (!shouldProcessFunction(func))
            return;


        Dictionary<IRParam*, IRParam*> paramMap;
        bool hasTransformedParams = false;

        // First pass: Transform parameter types
        for (auto param = func->getFirstParam(); param; param = param->getNextParam())
        {
            if (shouldTransformParamType(param->getDataType()))
            {
                transformParam(param); // Transform in place
                hasTransformedParams = true;
                changed = true;
                paramMap[param] = param; // Same parameter, different type
            }
        }

        if (!hasTransformedParams)
        {
            return;
        }

        // Second pass: Update function body using worklist for cascading transformations
        updateFunctionBody(paramMap, func);

        // Third pass: Update call sites
        updateCallSites(func, func, paramMap);
    }

    // Process the entire module using robust worklist approach
    SlangResult processModule()
    {
        // Collect all functions that need processing to avoid iterator invalidation
        List<IRFunc*> functionsToProcess;

        for (auto inst = module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            if (auto func = as<IRFunc>(inst))
            {
                if (shouldProcessFunction(func))
                {
                    functionsToProcess.add(func);
                }
            }
        }

        // Process each function
        for (auto func : functionsToProcess)
        {
            processFunc(func);
        }

        return SLANG_OK;
    }
};

SlangResult transformStructParamsToConstRef(IRModule* module, DiagnosticSink* sink)
{
    StructParamToConstRefContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
