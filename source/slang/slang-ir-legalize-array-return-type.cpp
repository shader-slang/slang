#include "slang-ir-legalize-array-return-type.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-type-layout.h"

namespace Slang
{

void makeFuncReturnViaOutParam(IRBuilder& builder, IRFunc* func)
{
    auto funcType = as<IRFuncType>(func->getFullType());
    if (!funcType)
        return;
    auto arrayType = funcType->getResultType();
    builder.setInsertBefore(funcType);
    List<IRType*> paramTypes;
    for (UInt i = 0; i < funcType->getParamCount(); i++)
    {
        paramTypes.add(funcType->getParamType(i));
    }
    auto outParamType = builder.getOutType(funcType->getResultType());
    paramTypes.add(outParamType);

    auto newFuncType = builder.getFuncType(paramTypes, builder.getVoidType());
    func->setFullType(newFuncType);
    auto firstBlock = func->getFirstBlock();
    builder.setInsertInto(firstBlock);
    auto outParam = builder.emitParam(outParamType);

    // Collect return insts.
    List<IRReturn*> returnInsts;
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_Return)
            {
                returnInsts.add(as<IRReturn>(inst));
            }
        }
    }

    // Rewrite return inst into a store + return void.
    for (auto returnInst : returnInsts)
    {
        builder.setInsertBefore(returnInst);
        builder.emitStore(outParam, returnInst->getVal());
        builder.emitReturn();
        SLANG_RELEASE_ASSERT(!returnInst->hasUses());
        returnInst->removeAndDeallocate();
    }

    // Rewrite call sites.
    List<IRCall*> callSites;
    for (auto use = func->firstUse; use; use = use->nextUse)
    {
        if (auto call = as<IRCall>(use->getUser()))
        {
            if (call->getCallee() == func)
                callSites.add(call);
        }
    }
    for (auto call : callSites)
    {
        builder.setInsertBefore(call);
        auto tmpVar = builder.emitVar(arrayType);
        List<IRInst*> args;
        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            args.add(call->getArg(i));
        }
        args.add(tmpVar);
        builder.emitCallInst(builder.getVoidType(), func, args);
        auto load = builder.emitLoad(tmpVar);
        call->replaceUsesWith(load);
        call->removeAndDeallocate();
    }
}

void legalizeArrayReturnType(IRModule* module, TargetRequest* targetReq)
{
    IRBuilder builder(module);

    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func)
            continue;

        auto resultType = func->getResultType();

        // Only process array return types
        if (resultType->getOp() != kIROp_ArrayType)
            continue;

        auto nameHint = resultType->findDecoration<IRNameHintDecoration>();
        bool isCoopVecArray = nameHint && (nameHint->getName() == UnownedStringSlice("CoopVec"));
        if (isCoopVecArray)
        {
            if (isCUDATarget(targetReq) ||
                isD3DTarget(targetReq) ||
                isCPUTarget(targetReq))
                continue;
        }

        makeFuncReturnViaOutParam(builder, func);
    }
}
} // namespace Slang
