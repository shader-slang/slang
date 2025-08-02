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

    bool isOnlyReadingFromVar(IRInst* var)
    {
        // TODO: add more insts to the list of "only reading" insts

        // Track if the param is only being read from
        bool onlyReading = true;
        traverseUsers(var, [&](IRInst* user)
        {
            if (!onlyReading)
                return;

            switch (user->getOp())
            {
            case kIROp_Load:
            case kIROp_MakeDifferentialPair:
            case kIROp_MakeDifferentialPairUserCode:
            case kIROp_MakeDifferentialPtrPair:
            case kIROp_MakeUInt64:
            case kIROp_MakeVector:
            case kIROp_MakeMatrix:
            case kIROp_MakeMatrixFromScalar:
            case kIROp_MatrixReshape:
            case kIROp_VectorReshape:
            case kIROp_MakeArray:
            case kIROp_MakeArrayFromElement:
            case kIROp_MakeCoopVector:
            case kIROp_MakeCoopVectorFromValuePack:
            case kIROp_MakeStruct:
            case kIROp_MakeTuple:
            case kIROp_MakeTargetTuple:
            case kIROp_MakeValuePack:
            case kIROp_ImageLoad:
            case kIROp_ByteAddressBufferLoad:
            case kIROp_RWStructuredBufferLoad:
            case kIROp_StructuredBufferLoad:
            case kIROp_RWStructuredBufferLoadStatus:
            case kIROp_StructuredBufferLoadStatus:
            case kIROp_RWStructuredBufferGetElementPtr:
            case kIROp_AtomicLoad:
            case kIROp_Return:
            case kIROp_Add:
            case kIROp_Sub:
            case kIROp_Mul:
            case kIROp_Div:
            case kIROp_IRem:
            case kIROp_FRem:
            case kIROp_Lsh:
            case kIROp_Rsh:
            case kIROp_BitAnd:
            case kIROp_BitOr:
            case kIROp_BitXor:
            case kIROp_And:
            case kIROp_Or:
            case kIROp_Neg:
            case kIROp_Not:
            case kIROp_BitNot:
            case kIROp_Select:
                return;

            case kIROp_Store:
            {
                auto store = as<IRStore>(user);
                if(store->getPtr() == var)
                    onlyReading = false;
                return;
            }

            case kIROp_AtomicStore:
            case kIROp_AtomicExchange:
            case kIROp_AtomicCompareExchange:
            case kIROp_AtomicAdd:
            case kIROp_AtomicSub:
            case kIROp_AtomicAnd:
            case kIROp_AtomicOr:
            case kIROp_AtomicXor:
            case kIROp_AtomicMin:
            case kIROp_AtomicMax:
            case kIROp_AtomicInc:
            case kIROp_AtomicDec:
            {
                auto atomicOp = as<IRAtomicOperation>(user);
                if(atomicOp->getPtr() == var)
                    onlyReading = false;
                return;
            }
            case kIROp_Call:
            {
                // Considered "only reading" if the argument is to a
                // `constref` parameter or `in` parameter
                auto call = as<IRCall>(user);
                auto callee = as<IRFunc>(call->getCallee());
                if(!callee)
                {
                    onlyReading = false;
                    return;
                }
                
                for(UInt argNum = 0; argNum < call->getArgCount(); argNum++)
                {
                    auto arg = call->getArg(argNum);
                    if(arg != var)
                        continue;
                    auto paramType = callee->getParamType(argNum);
                    if(as<IROutTypeBase>(paramType) || as<IRRefType>(paramType))
                    {
                        onlyReading = false;
                        return;
                    }
                    return;
                }
                return;
            }
            case kIROp_FieldExtract:
            case kIROp_GetElement:
                // recurse into the member/element
                if(!isOnlyReadingFromVar(user))
                    onlyReading = false;
                return;

            default:
                // any other inst is assumed to modify the var
                onlyReading = false;
                return;
            }
        });
        return onlyReading;
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
        //Next we must ensure that the param is only read from given the params `uses`.
        //This is necissary to ensure since otherwise we cannot optimize our `in` into a
        //`constref` (storing into a `in` is like storing into a local-var, storing in a
        //`constref` does not have the same effect).
        //return isOnlyReadingFromVar(param);
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
        case kIROp_GlobalParam:
        case kIROp_Param:
            return inst;
        
        case kIROp_FieldExtract:
        case kIROp_GetElement:
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
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
            auto nextIter = [&]()
            {
                i++;
                param = param->getNextParam();
            };
            auto argCount = call->getArgCount();
            for (; i < argCount; nextIter())
            {
                auto arg = call->getArg(i);                
                if(!updatedParams.contains(param))
                {
                    newArgs.add(arg);
                    continue;
                }

                auto addr = prepareArgForConstRefParam(arg);
                newArgs.add(addr);
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
                // This allows us to pass the address of variables directly into the function
                // if possible (this will be determined by the third pass).
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
    // TODO: implement NRVO
    ApplyCopyElisionContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
