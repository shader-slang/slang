// slang-ir-autodiff-primal-hoist.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-autodiff-region.h"
#include "slang-ir-dominators.h"


namespace Slang
{
    struct IROutOfOrderCloneContext : public RefObject
    {
        IRCloneEnv cloneEnv;
        HashSet<IRUse*> pendingUses;

        IRInst* cloneInstOutOfOrder(IRBuilder* builder, IRInst* inst)
        {
            IRInst* clonedInst = cloneInst(&cloneEnv, builder, inst);

            UInt operandCount = clonedInst->getOperandCount();
            for (UInt ii = 0; ii < operandCount; ++ii)
            {
                auto oldOperand = inst->getOperand(ii);
                auto newOperand = clonedInst->getOperand(ii);

                if (oldOperand == newOperand)
                    pendingUses.Add(&clonedInst->getOperands()[ii]);
            }

            for (auto use = inst->firstUse; use;)
            {
                auto nextUse = use->nextUse;
                
                if (pendingUses.Contains(use))
                {
                    pendingUses.Remove(use);
                    builder->replaceOperand(use, clonedInst);
                }
                
                use = nextUse;
            }

            return clonedInst;
        }
    };

    struct InversionInfo
    {
        IRInst* instToInvert;
        List<IRInst*> requiredOperands;
        List<IRInst*> targetInsts;

        InversionInfo(
            IRInst* instToInvert,
            List<IRInst*> requiredOperands,
            List<IRInst*> targetInsts) :
            instToInvert(instToInvert),
            requiredOperands(requiredOperands),
            targetInsts(targetInsts)
        { }

        InversionInfo() : instToInvert(nullptr)
        { }
    };

    struct HoistedPrimalsInfo : public RefObject
    {
        HashSet<IRInst*> storeSet;
        HashSet<IRInst*> recomputeSet;
        HashSet<IRInst*> invertSet;

        HashSet<IRInst*> instsToInvert;

        Dictionary<IRInst*, InversionInfo> invertInfoMap;

        void merge(HoistedPrimalsInfo* info)
        {
            for (auto inst : info->storeSet)
            {
                SLANG_ASSERT(!storeSet.Contains(inst));
                storeSet.Add(inst);
            }

            for (auto inst : info->recomputeSet)
            {
                SLANG_ASSERT(!recomputeSet.Contains(inst));
                recomputeSet.Add(inst);
            }

            for (auto inst : info->invertSet)
            {
                SLANG_ASSERT(!invertSet.Contains(inst));
                invertSet.Add(inst);
            }

            for (auto inst : info->instsToInvert)
            {
                SLANG_ASSERT(!instsToInvert.Contains(inst));
                instsToInvert.Add(inst);
            }

            for (auto kvpair : info->invertInfoMap)
            {
                SLANG_ASSERT(!invertInfoMap.ContainsKey(kvpair.Key));
                invertInfoMap[kvpair.Key] = kvpair.Value;
            }
        }
    };

    struct HoistResult
    {
        enum Mode
        {
            Store,
            Recompute,
            Invert,

            None
        };

        Mode mode;
        
        IRInst* instToStore = nullptr;
        IRInst* instToRecompute = nullptr;
        InversionInfo inversionInfo;

        HoistResult(Mode mode, IRInst* target) :
            mode(mode)
        { 
            switch (mode)
            {
            case Mode::Store:
                instToStore = target;
                break;
            case Mode::Recompute:
                instToRecompute = target;
                break;
            case Mode::Invert:
                SLANG_UNEXPECTED("Wrong constructor for HoistResult::Mode::Invert");
                break;
            default:
                SLANG_UNEXPECTED("Unhandled hoist mode");
                break;
            }
        }

        HoistResult(InversionInfo info) : 
            mode(Mode::Invert), inversionInfo(info)
        { }

        static HoistResult store(IRInst* inst)
        {
            return HoistResult(Mode::Store, inst);
        }

        static HoistResult recompute(IRInst* inst)
        {
            return HoistResult(Mode::Recompute, inst);
        }

        static HoistResult invert(InversionInfo inst)
        {
            return HoistResult(inst);
        }
    };

    
    // Information on which insts are to be stored, recomputed
    // and inverted within a single function.
    // This data structure also holds a map of raw HoistResult
    // objects to provide more information to later passes.
    // 
    struct CheckpointSetInfo : public RefObject
    {
        HashSet<IRInst*> storeSet;
        HashSet<IRInst*> recomputeSet;
        HashSet<IRInst*> invertSet;

        Dictionary<IRInst*, InversionInfo> invInfoMap;
    };

    struct BlockSplitInfo : public RefObject
    {
        // Maps primal to differential blocks from the unzip step.
        Dictionary<IRBlock*, IRBlock*> diffBlockMap;
    };

    class AutodiffCheckpointPolicyBase : public RefObject
    {
        public:

        AutodiffCheckpointPolicyBase(IRModule* module) : module(module)
        { }

        RefPtr<HoistedPrimalsInfo> processFunc(IRGlobalValueWithCode* func, BlockSplitInfo* info);

        // Do pre-processing on the function (mainly for 
        // 'global' checkpointing methods that consider the entire
        // function)
        // 
        virtual void preparePolicy(IRGlobalValueWithCode* func) = 0;

        virtual HoistResult classify(IRUse* diffBlockUse) = 0;

        protected:

        IRModule*               module;
    };

    class DefaultCheckpointPolicy : public AutodiffCheckpointPolicyBase
    {
        public:

        DefaultCheckpointPolicy(IRModule* module)
            : AutodiffCheckpointPolicyBase(module)
        { }

        virtual void preparePolicy(IRGlobalValueWithCode* func);
        virtual HoistResult classify(IRUse* use);
    };

    struct PrimalHoistContext
    {
        IRGlobalValueWithCode* func;
        IRModule* module;
        RefPtr<IRDominatorTree> domTree;
        RefPtr<AutodiffCheckpointPolicyBase> checkpointPolicy;

        List<IRInst*> storedInsts;
        List<IRInst*> recomputedInsts;
        List<IRInst*> invertedInsts;

        PrimalHoistContext(IRGlobalValueWithCode* func) : 
            func(func),
            module(func->getModule()),
            domTree(computeDominatorTree(func))
        { 
            // TODO: Populate set of primal insts to consider as 
            // being used in a differential inst.
            // 
        }
    };

    void maybeHoistPrimalInst(
        PrimalHoistContext* context,
        BlockSplitInfo* splitInfo,
        IRBuilder* primalBuilder,
        IRBuilder* diffBuilder,
        IRInst* primalInst);

    // Determines if the inst is invertible. 
    // (Does not actually materialize the inverse)
    //
    bool isInstInvertible(IRInst* primalInst);

    RefPtr<HoistedPrimalsInfo> applyCheckpointSet(
        CheckpointSetInfo* checkpointInfo,
        IRGlobalValueWithCode* func,
        BlockSplitInfo* splitInfo,
        HashSet<IRUse*> pendingUses);

    RefPtr<HoistedPrimalsInfo> ensurePrimalAvailability(
        HoistedPrimalsInfo* hoistInfo,
        IRGlobalValueWithCode* func,
        Dictionary<IRBlock*, List<IndexTrackingInfo*>> indexedBlockInfo);

};