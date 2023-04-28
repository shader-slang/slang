#include "slang-ir-autodiff-pairs.h"

namespace Slang
{

struct DiffPairLoweringPass : InstPassBase
{
    DiffPairLoweringPass(AutoDiffSharedContext* context) :
        InstPassBase(context->moduleInst->getModule()), 
        pairBuilderStorage(context)
    { 
        pairBuilder = &pairBuilderStorage;
    }

    IRInst* lowerPairType(IRBuilder* builder, IRType* pairType)
    {
        builder->setInsertBefore(pairType);
        auto loweredPairType = pairBuilder->lowerDiffPairType(
            builder,
            pairType);
        return loweredPairType;
    }

    IRInst* lowerMakePair(IRBuilder* builder, IRInst* inst)
    {
        if (auto makePairInst = as<IRMakeDifferentialPairBase>(inst))
        {
            bool isTrivial = false;
            auto pairType = as<IRDifferentialPairTypeBase>(makePairInst->getDataType());
            if (auto loweredPairType = lowerPairType(builder, pairType))
            {
                builder->setInsertBefore(makePairInst);
                IRInst* result = nullptr;
                if (isTrivial)
                {
                    result = makePairInst->getPrimalValue();
                }
                else
                {
                    IRInst* operands[2] = { makePairInst->getPrimalValue(), makePairInst->getDifferentialValue() };
                    result = builder->emitMakeStruct((IRType*)(loweredPairType), 2, operands);
                }
                makePairInst->replaceUsesWith(result);
                makePairInst->removeAndDeallocate();
                return result;
            }
        }

        return nullptr;
    }

    IRInst* lowerPairAccess(IRBuilder* builder, IRInst* inst)
    {
        if (auto getDiffInst = as<IRDifferentialPairGetDifferentialBase>(inst))
        {
            auto pairType = getDiffInst->getBase()->getDataType();
            if (auto pairPtrType = as<IRPtrTypeBase>(pairType))
            {
                pairType = pairPtrType->getValueType();
            }

            if (lowerPairType(builder, pairType))
            {
                builder->setInsertBefore(getDiffInst);
                IRInst* diffFieldExtract = nullptr;
                diffFieldExtract = pairBuilder->emitDiffFieldAccess(builder, getDiffInst->getBase());
                getDiffInst->replaceUsesWith(diffFieldExtract);
                getDiffInst->removeAndDeallocate();
                return diffFieldExtract;
            }
        }
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimalBase>(inst))
        {
            auto pairType = getPrimalInst->getBase()->getDataType();
            if (auto pairPtrType = as<IRPtrTypeBase>(pairType))
            {
                pairType = pairPtrType->getValueType();
            }

            if (lowerPairType(builder, pairType))
            {
                builder->setInsertBefore(getPrimalInst);

                IRInst* primalFieldExtract = nullptr;
                primalFieldExtract = pairBuilder->emitPrimalFieldAccess(builder, getPrimalInst->getBase());
                getPrimalInst->replaceUsesWith(primalFieldExtract);
                getPrimalInst->removeAndDeallocate();
                return primalFieldExtract;
            }
        }

        return nullptr;
    }

    bool processInstWithChildren(IRBuilder* builder, IRInst* instWithChildren)
    {
        bool modified = false;

        processAllInsts([&](IRInst* inst)
            {
                // Make sure the builder is at the right level.
                builder->setInsertInto(instWithChildren);

                switch (inst->getOp())
                {
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                case kIROp_DifferentialPairGetDifferentialUserCode:
                case kIROp_DifferentialPairGetPrimalUserCode:
                    lowerPairAccess(builder, inst);
                    break;

                case kIROp_MakeDifferentialPairUserCode:
                    lowerMakePair(builder, inst);
                    break;

                default:
                    break;
                }
            });

        OrderedDictionary<IRInst*, IRInst*> pendingReplacements;
        processAllInsts([&](IRInst* inst)
            {
                if (auto pairType = as<IRDifferentialPairTypeBase>(inst))
                {
                    if (auto loweredType = lowerPairType(builder, pairType))
                    {
                        pendingReplacements.add(pairType, loweredType);
                        modified = true;
                    }
                }
            });
        for (auto replacement : pendingReplacements)
        {
            replacement.key->replaceUsesWith(replacement.value);
            replacement.key->removeAndDeallocate();
        }

        return modified;
    }

    bool processModule()
    {
        IRBuilder builder(module);
        return processInstWithChildren(&builder, module->getModuleInst());
    }

    private: 
    
    DifferentialPairTypeBuilder* pairBuilder;

    DifferentialPairTypeBuilder pairBuilderStorage;

};

bool processPairTypes(AutoDiffSharedContext* context)
{
    DiffPairLoweringPass pairLoweringPass(context);
    return pairLoweringPass.processModule();
}

struct DifferentialPairUserCodeTranscribePass : public InstPassBase
{
    DifferentialPairUserCodeTranscribePass(IRModule* module)
        :InstPassBase(module)
    {}

    IRInst* rewritePairType(IRBuilder* builder, IRType* pairType)
    {
        builder->setInsertBefore(pairType);
        auto originalPairType = as<IRDifferentialPairType>(pairType);
        return builder->getDifferentialPairUserCodeType(originalPairType->getValueType(), originalPairType->getWitness());
    }

    IRInst* rewriteMakePair(IRBuilder* builder, IRMakeDifferentialPair* inst)
    {
        auto pairType = as<IRDifferentialPairType>(inst->getFullType());
        builder->setInsertBefore(inst);
        auto newInst = builder->emitMakeDifferentialPairUserCode(
            (IRType*)pairType, inst->getPrimalValue(), inst->getDifferentialValue());
        inst->replaceUsesWith(newInst);
        inst->removeAndDeallocate();
        return newInst;
    }

    IRInst* rewritePairAccess(IRBuilder* builder, IRInst* inst)
    {
        if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(inst))
        {
            builder->setInsertBefore(inst);

            auto newInst = builder->emitDifferentialPairGetDifferentialUserCode(
                (IRType*)inst->getFullType(), getDiffInst->getBase());
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
        }
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(inst))
        {
            builder->setInsertBefore(inst);
            auto newInst = builder->emitDifferentialPairGetPrimalUserCode(getPrimalInst->getBase());
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
        }
        return inst;
    }

    bool processInstWithChildren(IRBuilder* builder, IRInst* instWithChildren)
    {
        SLANG_UNUSED(instWithChildren);

        bool modified = false;

        processAllInsts([&](IRInst* inst)
            {
                switch (inst->getOp())
                {
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                    rewritePairAccess(builder, inst);
                    break;

                case kIROp_MakeDifferentialPair:
                    rewriteMakePair(builder, as<IRMakeDifferentialPair>(inst));
                    break;

                default:
                    break;
                }
            });

        OrderedDictionary<IRInst*, IRInst*> pendingReplacements;
        processInstsOfType<IRDifferentialPairType>(kIROp_DifferentialPairType, [&](IRDifferentialPairType* inst)
            {
                if (auto loweredType = rewritePairType(builder, inst))
                {
                    pendingReplacements.add(inst, loweredType);
                    modified = true;
                }
            });
        for (auto replacement : pendingReplacements)
        {
            replacement.key->replaceUsesWith(replacement.value);
            replacement.key->removeAndDeallocate();
        }

        return modified;
    }

    bool processModule()
    {
        IRBuilder builder(module);
        return processInstWithChildren(&builder, module->getModuleInst());
    }
};

void rewriteDifferentialPairToUserCode(IRModule* module)
{
    DifferentialPairUserCodeTranscribePass pairRewritePass(module);
    pairRewritePass.processModule();
}

}
