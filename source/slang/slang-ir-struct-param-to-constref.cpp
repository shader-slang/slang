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
            // Check if it's already a pointer type (don't double-transform)
            if (as<IRPtrTypeBase>(type) || as<IRConstRefType>(type))
                return false;
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
            if (as<IRHLSLStructuredBufferTypeBase>(type))
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

    // Update function body to handle ConstRef parameters
    void updateFunctionBody(IRFunc* func, Dictionary<IRParam*, IRParam*>& paramMap)
    {
        if (paramMap.getCount() == 0)
            return;

        for (auto block = func->getFirstBlock(); block; block = block->getNextBlock())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                builder.setInsertBefore(inst);

                // Transform fieldExtract(param, field) -> load(fieldAddress(param, field))
                if (auto fieldExtract = as<IRFieldExtract>(inst))
                {
                    auto baseParam = fieldExtract->getBase();
                    if (paramMap.containsKey(as<IRParam>(baseParam)))
                    {
                        auto newParam = paramMap[as<IRParam>(baseParam)];
                        auto fieldAddr =
                            builder.emitFieldAddress(newParam, fieldExtract->getField());
                        auto loadInst = builder.emitLoad(fieldAddr);

                        fieldExtract->replaceUsesWith(loadInst);
                        fieldExtract->removeAndDeallocate();
                        changed = true;
                        continue;
                    }
                }

                // Transform getElement(param, index) -> load(getElementPtr(param, index))
                if (auto getElement = as<IRGetElement>(inst))
                {
                    auto baseParam = getElement->getBase();
                    if (paramMap.containsKey(as<IRParam>(baseParam)))
                    {
                        auto newParam = paramMap[as<IRParam>(baseParam)];
                        auto elemPtr = builder.emitElementAddress(newParam, getElement->getIndex());
                        auto loadInst = builder.emitLoad(elemPtr);

                        getElement->replaceUsesWith(loadInst);
                        getElement->removeAndDeallocate();
                        changed = true;
                        continue;
                    }
                }
            }
        }
    }

    // Update call sites to pass addresses instead of values
    void updateCallSites(
        IRFunc* originalFunc,
        IRFunc* newFunc,
        Dictionary<IRParam*, IRParam*>& /*paramMap*/)
    {
        // Find all calls to the original function
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

        return false;
    }

    // Process a single function
    void processFunc(IRFunc* func)
    {
        // Skip built-in utility functions
        if (shouldSkipFunction(func))
            return;

        Dictionary<IRParam*, IRParam*> paramMap;
        bool hasTransformedParams = false;

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
            return;

        // Update function body to handle new parameter types
        updateFunctionBody(func, paramMap);

        // Update all call sites
        updateCallSites(func, func, paramMap);
    }

    // Process the entire module
    SlangResult processModule()
    {
        // Process all functions in the module
        for (auto inst = module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            if (auto func = as<IRFunc>(inst))
            {
                processFunc(func);
            }
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
