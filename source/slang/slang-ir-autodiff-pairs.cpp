#include "slang-ir-autodiff-pairs.h"

namespace Slang
{

struct DiffPairLoweringPass : InstPassBase
{
    IRInst* lowerPairType(IRBuilder* builder, IRType* pairType, bool* isTrivial = nullptr)
    {
        builder->setInsertBefore(pairType);
        auto loweredPairTypeInfo = pairBuilder->lowerDiffPairType(
            builder,
            pairType);
        if (isTrivial)
            *isTrivial = loweredPairTypeInfo.isTrivial;
        return loweredPairTypeInfo.loweredType;
    }

    IRInst* lowerMakePair(IRBuilder* builder, IRInst* inst)
    {

        if (auto makePairInst = as<IRMakeDifferentialPair>(inst))
        {
            bool isTrivial = false;
            auto pairType = as<IRDifferentialPairType>(makePairInst->getDataType());
            if (auto loweredPairType = lowerPairType(builder, pairType, &isTrivial))
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

            if (lowerPairType(builder, pairType, nullptr))
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

            if (lowerPairType(builder, pairType, nullptr))
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

    bool processPairTypes(IRBuilder* builder, IRInst* instWithChildren)
    {
        bool modified = false;
        // Hoist all pair types to global scope when possible.
        auto moduleInst = module->getModuleInst();
        processInstsOfType<IRDifferentialPairType>(kIROp_DifferentialPairType, [&](IRInst* originalPairType)
            {
                if (originalPairType->parent != moduleInst)
                {
                    originalPairType->removeFromParent();
                    ShortList<IRInst*> operands;
                    for (UInt i = 0; i < originalPairType->getOperandCount(); i++)
                    {
                        operands.add(originalPairType->getOperand(i));
                    }
                    auto newPairType = builder->findOrEmitHoistableInst(
                        originalPairType->getFullType(),
                        originalPairType->getOp(),
                        originalPairType->getOperandCount(),
                        operands.getArrayView().getBuffer());
                    originalPairType->replaceUsesWith(newPairType);
                    originalPairType->removeAndDeallocate();
                }
            });

        sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();

        processAllInsts([&](IRInst* inst)
            {
                // Make sure the builder is at the right level.
                builder->setInsertInto(instWithChildren);

                switch (inst->getOp())
                {
                case kIROp_DifferentialPairGetDifferential:
                case kIROp_DifferentialPairGetPrimal:
                    lowerPairAccess(builder, inst);
                    modified = true;
                    break;

                case kIROp_MakeDifferentialPair:
                    lowerMakePair(builder, inst);
                    modified = true;
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
                }
            });
        return modified;
    }

    bool simplifyDifferentialBottomType(IRBuilder* builder)
    {
        bool modified = false;
        auto diffBottom = builder->getDifferentialBottom();

        bool changed = true;
        List<IRUse*> uses;
        while (changed)
        {
            changed = false;
            // Replace all insts whose type is `DifferentialBottomType` to `diffBottom`.
            processAllInsts([&](IRInst* inst)
                {
                    if (inst->getDataType() && inst->getDataType()->getOp() == kIROp_DifferentialBottomType)
                    {
                        if (inst != diffBottom)
                        {
                            inst->replaceUsesWith(diffBottom);
                            inst->removeAndDeallocate();
                            modified = true;
                        }
                    }
                });
            // Go through all uses of diffBottom and run simplification.
            processAllInsts([&](IRInst* inst)
                {
                    if (!inst->hasUses())
                        return;

                    builder->setInsertBefore(inst);
                    IRInst* valueToReplace = nullptr;
                    switch (inst->getOp())
                    {
                    case kIROp_Store:
                        if (as<IRStore>(inst)->getVal() == diffBottom)
                        {
                            inst->removeAndDeallocate();
                            changed = true;
                        }
                        return;
                    case kIROp_MakeDifferentialPair:
                        // Our simplification could lead to a situation where
                        // bottom is used to make a pair that has a non-bottom differential type,
                        // in this case we should use zero instead.
                        if (inst->getOperand(1) == diffBottom)
                        {
                            // Only apply if we are the second operand.
                            auto pairType = as<IRDifferentialPairType>(inst->getDataType());
                            if (pairBuilder->getDiffTypeFromPairType(builder, pairType)->getOp() != kIROp_DifferentialBottomType)
                            {
                                auto zero = transcriberStorage.getDifferentialZeroOfType(builder, pairType->getValueType());
                                inst->setOperand(1, zero);
                                changed = true;
                            }
                        }
                        return;
                    case kIROp_DifferentialPairGetDifferential:
                        if (inst->getOperand(0)->getOp() == kIROp_MakeDifferentialPair)
                        {
                            valueToReplace = inst->getOperand(0)->getOperand(1);
                        }
                        break;
                    case kIROp_DifferentialPairGetPrimal:
                        if (inst->getOperand(0)->getOp() == kIROp_MakeDifferentialPair)
                        {
                            valueToReplace = inst->getOperand(0)->getOperand(0);
                        }
                        break;
                    case kIROp_Add:
                        if (inst->getOperand(0) == diffBottom)
                        {
                            valueToReplace = inst->getOperand(1);
                        }
                        else if (inst->getOperand(1) == diffBottom)
                        {
                            valueToReplace = inst->getOperand(0);
                        }
                        break;
                    case kIROp_Sub:
                        if (inst->getOperand(0) == diffBottom)
                        {
                            // If left is bottom, and right is not bottom, then we should return -right.
                            // However we can't possibly run into that case since both side of - operator
                            // must be at the same order of differentiation.
                            valueToReplace = diffBottom;
                        }
                        else if (inst->getOperand(1) == diffBottom)
                        {
                            valueToReplace = inst->getOperand(0);
                        }
                        break;
                    case kIROp_Mul:
                    case kIROp_Div:
                        if (inst->getOperand(0) == diffBottom)
                        {
                            valueToReplace = diffBottom;
                        }
                        else if (inst->getOperand(1) == diffBottom)
                        {
                            valueToReplace = diffBottom;
                        }
                        break;
                    default:
                        break;
                    }
                    if (valueToReplace)
                    {
                        inst->replaceUsesWith(valueToReplace);
                        changed = true;
                    }
                });
            modified |= changed;
        }

        return modified;
    }

    bool eliminateDifferentialBottomType(IRBuilder* builder)
    {
        simplifyDifferentialBottomType(builder);

        bool modified = false;
        auto diffBottom = builder->getDifferentialBottom();
        auto diffBottomType = diffBottom->getDataType();
        diffBottom->replaceUsesWith(builder->getVoidValue());
        diffBottom->removeAndDeallocate();
        diffBottomType->replaceUsesWith(builder->getVoidType());

        return modified;
    }

    private: 
    
    IRModule* module;
    
    DifferentialPairTypeBuilder* pairBuilder;



};
}