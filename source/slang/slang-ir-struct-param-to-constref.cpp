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
            return true;
        default:
            return false;
        }
    }

    // Check if an address points to immutable memory
    bool isImmutableMemory(IRInst* addr)
    {
        auto rootAddr = getRootAddr(addr);
        if (!rootAddr)
            return false;

        // Check if root is constant buffer, StructuredBuffer, or ByteAddressBuffer
        if (auto globalParam = as<IRGlobalParam>(rootAddr))
        {
            auto type = globalParam->getDataType();
            // Only read-only structured buffers are immutable, not RW buffers
            if (type->getOp() == kIROp_HLSLStructuredBufferType)
                return true;
            if (as<IRUniformParameterGroupType>(type))
                return true;
            // Add more buffer types as needed
        }

        // Check if root is IRParam with ConstRef<T> type
        if (auto param = as<IRParam>(rootAddr))
        {
            if (as<IRConstRefType>(param->getDataType()))
                return true;
        }

        return false;
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
    void transformFieldExtractUse(IRBuilder& transformBuilder, IRUse* use)
    {
        auto fieldExtract = as<IRFieldExtract>(use->getUser());
        auto param = as<IRParam>(use->get());

        transformBuilder.setInsertBefore(fieldExtract);
        auto fieldAddr = transformBuilder.emitFieldAddress(param, fieldExtract->getField());
        auto loadInst = transformBuilder.emitLoad(fieldAddr);

        fieldExtract->replaceUsesWith(loadInst);
        fieldExtract->removeAndDeallocate();
        changed = true;
    }

    // Transform use of a ConstRef parameter in get element
    void transformGetElementUse(IRBuilder& transformBuilder, IRUse* use)
    {
        auto getElement = as<IRGetElement>(use->getUser());
        auto param = as<IRParam>(use->get());

        transformBuilder.setInsertBefore(getElement);
        auto elemAddr = transformBuilder.emitElementAddress(param, getElement->getIndex());
        auto loadInst = transformBuilder.emitLoad(elemAddr);

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

    // Update function body using worklist to handle cascading transformations
    void updateFunctionBody(Dictionary<IRParam*, IRParam*>& paramMap)
    {
        if (paramMap.getCount() == 0)
            return;


        // Build worklist of all parameters that need processing
        List<IRParam*> workList;
        for (auto pair : paramMap)
        {
            workList.add(pair.first);
        }

        // Process worklist using index-based iteration to handle cascading
        for (Index workListIndex = 0; workListIndex < workList.getCount(); workListIndex++)
        {
            auto param = workList[workListIndex];

            // Process all uses of this parameter with nextUse pattern
            for (auto use = param->firstUse; use;)
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
                    transformFieldExtractUse(transformBuilder, use);
                    break;
                case kIROp_GetElement:
                    transformGetElementUse(transformBuilder, use);
                    break;
                default:
                    // For all other uses, insert a load
                    transformDirectUse(transformBuilder, use);
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

                        // Check if this is from immutable memory (optimization opportunity)
                        if (isImmutableMemory(sourceAddr))
                        {
                            newArgs.add(sourceAddr);
                        }
                        else
                        {
                            // For mutable memory, we still pass the address
                            newArgs.add(sourceAddr);
                        }
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
        updateFunctionBody(paramMap);

        // Third pass: Update call sites
        updateCallSites(func, func, paramMap);
    }

    // Process the entire module using worklist approach
    SlangResult processModule()
    {

        // First, collect all functions that need processing (avoid iterator invalidation)
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


        // Process each function using worklist approach
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
