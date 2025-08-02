#include "slang-ir-apply-copy-elision.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct ApplyCopyElisionContext
{
    IRModule* module;
    DiagnosticSink* sink;
    IRBuilder builder;
    bool changed = false;

    ApplyCopyElisionContext(IRModule* module, DiagnosticSink* sink)
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
                    switch(user->getOp())
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
                }
            );
        }
    }

    // Check if an instruction represents an addressable value
    IRInst* getAddressable(IRInst* inst)
    {
        if (!inst)
            return nullptr;

        switch (inst->getOp())
        {
        case kIROp_Var:
        case kIROp_GlobalVar:
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
            return inst;
        case kIROp_GlobalParam:
            return builder.emitLoad(inst);
        case kIROp_FieldExtract:
        case kIROp_GetElement:
        case kIROp_Param:
            return builder.emitGetAddress(builder.getPtrType(inst->getDataType()), inst);
        case kIROp_Load:
            // Check if the load is from an addressable source
            return getAddressable(inst->getOperand(0));
        default:
            return nullptr;
        }
    }

    IRInst* prepareArgForConstRefParam(IRInst* arg)
    {
        // If the arg is addressable, we can pass the arg directly.
        if(auto addr = getAddressable(arg))
            return addr;
        
        // Unable to pass the arg directly, create a temporary.
        auto tempVar = builder.emitVar(arg->getFullType());
        builder.emitStore(tempVar, arg);
        return tempVar;
    }
    // Update call sites to pass an address instead of value for each updated-param
    void updateCallSites(
        IRFunc* func,
        HashSet<IRParam*>& updatedParams)
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
            auto param = func->getFirstParam();
            auto argCount = call->getArgCount();
            while(i < argCount)
            {
                auto arg = call->getArg(i);                
                if(!updatedParams.contains(param))
                {
                    newArgs.add(arg);

                    i++;
                    param = param->getNextParam();
                    continue;
                }

                auto addr = prepareArgForConstRefParam(arg);
                newArgs.add(addr);
                
                i++;
                param = param->getNextParam();
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
        // Skip functions with target intrinsic decorations (backend-specific functions)
        if (func->findDecoration<IRTargetIntrinsicDecoration>())
            return false;

        // Skip functions without definitions
        if (!func->isDefinition())
            return false;

        // Skip entry point functions (interface with runtime)
        if (func->findDecoration<IREntryPointDecoration>())
            return false;

        // Skip CUDA kernel functions (marked with [CudaKernel])
        if (func->findDecoration<IRCudaKernelDecoration>())
            return false;

        // Skip PyTorch entry point functions
        if (func->findDecoration<IRTorchEntryPointDecoration>())
            return false;

        // Skip functions with compute shader specific decorations
        if (func->findDecoration<IRNumThreadsDecoration>())
            return true;

        // Skip functions with other shader stage decorations
        if (func->findDecoration<IRMaxVertexCountDecoration>() ||
            func->findDecoration<IRInstanceDecoration>() ||
            func->findDecoration<IRWaveSizeDecoration>() ||
            func->findDecoration<IRGeometryInputPrimitiveTypeDecoration>() ||
            func->findDecoration<IRStreamOutputTypeDecoration>())
            return false;

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
                auto constRefType = builder.getConstRefType(paramType);
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

    SlangResult processModule()
    {
        // Collect all functions that need processing
        List<IRFunc*> functionsToProcess;

        for (auto inst = module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;
            if (!shouldProcessFunction(func))
                continue;
            functionsToProcess.add(func);
        }

        // Process each function
        for (auto func : functionsToProcess)
        {
            processFunc(func);
        }

        return SLANG_OK;
    }
};

SlangResult applyCopyElision(IRModule* module, DiagnosticSink* sink)
{
    ApplyCopyElisionContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
