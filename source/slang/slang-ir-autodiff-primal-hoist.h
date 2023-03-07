// slang-ir-autodiff-primal-hoist.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-autodiff.h"
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
                    pendingUses.Add(&inst->getOperands()[ii]);
            }

            for (auto use = inst->firstUse; use;)
            {
                auto nextUse = use->nextUse;
                
                if (pendingUses.Contains(use))
                    use->set(clonedInst);
                
                use = nextUse;
            }
        }
    };

    struct InversionInfo
    {
        IRInst* instToInvert;
        List<IRInst*> requiredOperands;
        IRUse* targetUse;
    };

    struct HoistedPrimalsInfo : public RefObject
    {
        HashSet<IRInst*> storeSet;
        HashSet<IRInst*> recomputeSet;
        HashSet<IRInst*> invertSet;

        Dictionary<IRInst*, InversionInfo> invertInfoMap;
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

        // This inst that will produce the value
        union 
        {
            IRInst* instToStore;
            IRInst* instToRecompute;
            InversionInfo inversionInfo;
        };

        HoistResult(Mode mode, IRInst* target) :
            mode(mode)
        { 
            if (mode == Mode::Store)
                instToStore = target;
            else if (mode == Mode::Recompute)
                instToRecompute = target;
            else if (mode == Mode::Invert)
            {
                SLANG_ASSERT("Wrong constructor for HoistResult::Mode::Invert");
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

        Dictionary<IRUse*, HoistResult> hoistModeMap;
    };

    class AutodiffCheckpointPolicyBase
    {
        public:

        AutodiffCheckpointPolicyBase(IRGlobalValueWithCode* func)
            : func(func), module(func->getModule())
        { }

        
        RefPtr<CheckpointSetInfo> processFunc(IRGlobalValueWithCode* func, BlockSplitInfo* info);

        // Do pre-processing on the function (mainly for 
        // 'global' checkpointing methods that consider the entire
        // function)
        // 
        virtual void preparePolicy(IRGlobalValueWithCode* func) = 0;

        virtual HoistResult classify(IRUse* diffBlockUse);

        RefPtr<HoistedPrimalsInfo> applyCheckpointSet(
            CheckpointSetInfo* checkpointInfo,
            IRGlobalValueWithCode* func,
            BlockSplitInfo* splitInfo);
        
        RefPtr<HoistedPrimalsInfo> ensurePrimalAvailability(
            HoistedPrimalsInfo* hoistInfo,
            IRGlobalValueWithCode* func,
            Dictionary<IRBlock*, List<IndexTrackingInfo*>> indexedBlockInfo);

        protected:

        IRGlobalValueWithCode*  func;
        IRModule*               module;
    };

    class DefaultCheckpointPolicy : public AutodiffCheckpointPolicyBase
    {
        DefaultCheckpointPolicy(IRGlobalValueWithCode* func)
            : AutodiffCheckpointPolicyBase(func)
        { }

        virtual void preparePolicy(IRGlobalValueWithCode* func);

        virtual bool shouldStoreInst(IRInst* inst);
        virtual bool shouldStoreCallContext(IRCall* callInst);
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

    struct BlockSplitInfo
    {
        // Maps primal to differential blocks from the unzip step.
        Dictionary<IRBlock*, IRBlock*> diffBlockMap;
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
};