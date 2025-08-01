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
        case kIROp_VectorType:
        case kIROp_MatrixType:
        case kIROp_TupleType:
        case kIROp_CoopVectorType:
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

        // Add newly created instruction to worklist for cascading transformations
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

    // Check if an instruction represents an addressable value
    bool isAddressable(IRInst* inst)
    {
        if (!inst)
            return false;

        switch (inst->getOp())
        {
        case kIROp_Var:
        case kIROp_GlobalVar:
        case kIROp_GlobalParam:
        case kIROp_Param:
            return true;
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
            return isAddressable(inst->getOperand(0));
        case kIROp_Load:
            // Check if the load is from an addressable source
            return isAddressable(inst->getOperand(0));
        default:
            return false;
        }
    }

    // Get the address of an instruction if it's addressable
    IRInst* getAddressOf(IRInst* inst)
    {
        if (!inst)
            return nullptr;

        switch (inst->getOp())
        {
        case kIROp_Var:
        case kIROp_GlobalVar:
        case kIROp_GlobalParam:
            return inst;
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
            return inst;
        case kIROp_Load:
            // If this is a load from an addressable source, return the source address
            if (isAddressable(inst->getOperand(0)))
                return inst->getOperand(0);
            return nullptr;
        default:
            return nullptr;
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
                    // Check if we can pass the address directly instead of creating a temporary
                    if (auto directAddr = getAddressOf(arg))
                    {
                        newArgs.add(directAddr);
                    }
                    else
                    {
                        // For ConstRef parameters, create temporary and pass address
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
        // First check if this should be skipped entirely (includes entry points)
        if (shouldSkipFunction(func))
            return false;

        // Only process functions that have method decorations
        if (!func->findDecoration<IRMethodDecoration>())
            return false;

        // Skip constructor functions (they have special initialization semantics)
        if (func->findDecoration<IRConstructorDecoration>())
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

        // Skip PyTorch entry point functions
        if (func->findDecoration<IRTorchEntryPointDecoration>())
            return true;

        // Skip functions with compute shader specific decorations
        if (func->findDecoration<IRNumThreadsDecoration>())
            return true;

        // Skip functions with other shader stage decorations
        if (func->findDecoration<IRMaxVertexCountDecoration>() ||
            func->findDecoration<IRInstanceDecoration>() ||
            func->findDecoration<IRWaveSizeDecoration>() ||
            func->findDecoration<IRGeometryInputPrimitiveTypeDecoration>() ||
            func->findDecoration<IRStreamOutputTypeDecoration>())
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
