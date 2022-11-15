#include "slang-ir-merge-import-symbols.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-clone.h"

namespace Slang {

static void _mergeSymbol(IRBuilder& builder, IRInst* existing, IRInst* incoming)
{
    // TODO: we should consider reuse the same logic from linking.
    if (auto genericVal = as<IRGeneric>(existing))
    {
        IRCloneEnv cloneEnv;
        if (auto incomingGeneric = as<IRGeneric>(incoming))
        {
            auto incomingParam = incomingGeneric->getFirstParam();
            for (auto param = genericVal->getFirstParam(); param; param = as<IRParam>(param->next))
            {
                SLANG_ASSERT(incomingParam);
                if (!incomingParam)
                    return;
                cloneEnv.mapOldValToNew[incomingParam] = param;
                incomingParam = as<IRParam>(incomingParam->next);
            }
            if (!genericVal->getFirstOrdinaryInst())
                return;
            auto existingInnerVal = findInnerMostGenericReturnVal(genericVal);
            auto incomingInnerVal = findInnerMostGenericReturnVal(incomingGeneric);
            if (!existingInnerVal)
                return;
            if (!incomingInnerVal)
                return;
            builder.setInsertBefore(genericVal->getFirstOrdinaryInst());
            for (auto ordinary = incomingGeneric->getFirstOrdinaryInst(); ordinary; ordinary = ordinary->next)
            {
                if (as<IRReturn>(ordinary))
                    continue;
                if (ordinary == incomingInnerVal)
                    continue;
                cloneInst(&cloneEnv, &builder, ordinary);
            }
            if (existingInnerVal->getFirstChild())
            {
                builder.setInsertBefore(existingInnerVal->getFirstChild());
            }
            else
            {
                builder.setInsertInto(existingInnerVal);
            }
            for (auto decor : incomingInnerVal->getDecorations())
            {
                if (!existingInnerVal->findDecorationImpl(decor->getOp()))
                {
                    cloneInst(&cloneEnv, &builder, decor);
                }
            }
        }
    }

    List<IRInst*> decors;
    for (auto decor : incoming->getDecorations())
    {
        decors.add(decor);
    }
    for (auto decor : decors)
    {
        if (as<IRDecoration>(decor))
        {
            decor->insertAtStart(existing);
        }
    }
}

void mergeImportSymbols(IRModule* module)
{
    SharedIRBuilder sharedBuilder(module);
    IRBuilder builder(sharedBuilder);
    Dictionary<UnownedStringSlice, IRInst*> symbols;
    for (auto inst = module->getModuleInst()->getFirstChild(); inst;)
    {
        auto next = inst->next;
        if (auto importDecor = inst->findDecoration<IRImportDecoration>())
        {
            auto name = importDecor->getMangledName();
            IRInst* existingValue = nullptr;
            if (symbols.TryGetValue(name, existingValue))
            {
                _mergeSymbol(builder, existingValue, inst);
                inst->replaceUsesWith(existingValue);
                inst->removeAndDeallocate();
            }
            else
            {
                symbols[name] = inst;
            }
        }
        inst = next;
    }
}

}
