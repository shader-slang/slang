#include "slang-ir-autodiff-pairs.h"
#include "slang-ir-hoist-local-types.h"

namespace Slang
{

struct DiffPairLoweringPass : InstPassBase
{
    DiffPairLoweringPass(AutoDiffSharedContext* context) :
        InstPassBase(context->moduleInst->getModule()), 
        pairBuilderStorage(context),
        autodiffContext(context)
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
        if (auto makePairInst = as<IRMakeDifferentialPair>(inst))
        {
            bool isTrivial = false;
            auto pairType = as<IRDifferentialPairType>(makePairInst->getDataType());
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
        if (auto getDiffInst = as<IRDifferentialPairGetDifferential>(inst))
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
        else if (auto getPrimalInst = as<IRDifferentialPairGetPrimal>(inst))
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

        // Hoist all pair types to global scope when possible.
        hoistLocalTypes(module);

        autodiffContext->sharedBuilder->deduplicateAndRebuildGlobalNumberingMap();

        processAllInsts([&](IRInst* inst)
            {
                // Make sure the builder is at the right level.
                builder->setInsertInto(instWithChildren);

                switch (inst->getOp())
                {
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                    lowerPairAccess(builder, inst);
                    break;

                case kIROp_MakeDifferentialPair:
                    lowerMakePair(builder, inst);
                    break;

                default:
                    break;
                }
            });

        processInstsOfType<IRDifferentialPairType>(kIROp_DifferentialPairType, [&](IRDifferentialPairType* inst)
            {
                if (auto loweredType = lowerPairType(builder, inst))
                {
                    inst->replaceUsesWith(loweredType);
                    inst->removeAndDeallocate();
                    modified = true;
                }
            });
        return modified;
    }

    bool processModule()
    {
        IRBuilder builder(autodiffContext->sharedBuilder);
        return processInstWithChildren(&builder, module->getModuleInst());
    }

    private: 
    
    AutoDiffSharedContext* autodiffContext;
    
    DifferentialPairTypeBuilder* pairBuilder;

    DifferentialPairTypeBuilder pairBuilderStorage;

};

bool processPairTypes(AutoDiffSharedContext* context)
{
    DiffPairLoweringPass pairLoweringPass(context);
    return pairLoweringPass.processModule();
}

}
