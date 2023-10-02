#include "slang-ir-array-reg-to-mem.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
    bool eliminateArrayTypeParameters(IRFunc* func)
    {
        IRBuilder builder(func);
        bool changed = false;
        List<UInt> arrayParamIds;
        UInt idx = 0;
        List<IRParam*> paramWorkList;
        for (auto param : func->getParams())
        {
            if (auto arrayType = as<IRArrayTypeBase>(param->getFullType()))
            {
                paramWorkList.add(param);
                arrayParamIds.add(idx);
            }
            idx++;
        }
        for (auto param : paramWorkList)
        {
            // We have an array type parameter, so we need to replace it with a pointer to the array
            // type.
            //
            // We will also need to insert a `load` instruction at the start of the function body
            // to load the actual pointer value from the parameter.
            //
            if (auto arrayType = as<IRArrayTypeBase>(param->getFullType()))
            {
                changed = true;
                auto ptrArrayType = builder.getPtrType(arrayType);
                auto newParam = builder.createParam(ptrArrayType);
                newParam->insertBefore(param);
                setInsertAfterOrdinaryInst(&builder, param);
                auto regVal = builder.emitLoad(newParam);
                param->replaceUsesWith(regVal);
                param->removeAndDeallocate();
            }
        }
        if (changed)
        {
            // The function is modified, we need to also update its type.
            List<IRType*> paramTypes;
            for (auto param : func->getParams())
            {
                paramTypes.add(param->getFullType());
            }
            auto newFuncType = builder.getFuncType((UInt)paramTypes.getCount(), paramTypes.getBuffer(), func->getResultType());
            func->setFullType(newFuncType);

            // Update all the call sites to pass the arrays by pointer.
            traverseUses(func, [&](IRUse* use)
                {
                    if (const auto call = as<IRCall>(use->getUser()))
                    {
                        builder.setInsertBefore(call);
                        for (auto paramId : arrayParamIds)
                        {
                            auto arg = call->getArg(paramId);
                            SLANG_ASSERT(as<IRPtrTypeBase>(paramTypes[paramId]));
                            auto var = builder.emitVar(as<IRPtrTypeBase>(paramTypes[paramId])->getValueType());
                            builder.emitStore(var, arg);
                            call->setArg(paramId, var);
                        }
                    }
                });
        }
        return changed;
    }

    bool eliminateArrayTypeSSARegisters(IRModule* module)
    {
        bool changed = false;
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                changed |= eliminateArrayTypeParameters(func);
            }
        }
        return changed;
    }
}
