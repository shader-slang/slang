// slang-ir-autodiff-primal-hoist.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-dominators.h"

namespace Slang
{
    
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
        IRInst* valueSrcInst;

        HoistResult(Mode mode, IRInst* target) :
            mode(mode), valueSrcInst(target)
        { }

        static HoistResult store(IRInst* inst)
        {
            return HoistResult(Mode::Store, inst);
        }

        static HoistResult recompute(IRInst* inst)
        {
            return HoistResult(Mode::Recompute, inst);
        }

        static HoistResult invert(IRInst* inst)
        {
            return HoistResult(Mode::Invert, inst);
        }
    };


    class AutodiffCheckpointPolicyBase
    {
        public:

        AutodiffCheckpointPolicyBase(IRGlobalValueWithCode* func)
            : func(func), module(func->getModule())
        { }

        void processFunc(IRGlobalValueWithCode* func);

        // Do pre-processing on the function (mainly for 
        // 'global' checkpointing methods that consider the entire
        // function)
        // 
        virtual void preparePolicy(IRGlobalValueWithCode* func) = 0;

        virtual HoistResult apply(IRUse* diffBlockUse);

        // Utility method to populate instsWithDiffUses
        void findInstsWithDiffUses();

        protected:

        IRGlobalValueWithCode*  func;
        IRModule*               module;

        List<IRInst*>           storedInsts;
        List<IRInst*>           recomputedInsts;
        List<IRInst*>           invertedInsts;

        HashSet<IRInst*>        storedInstSet;
        HashSet<IRInst*>        recomputedInstSet;

        HashSet<IRInst*>        instsWithDiffUses; 
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
            domTree(computeDominatorTree(func)),

        { 
            // TODO: Populate set of primal insts to consider as 
            // being used in a differential inst.
            //
        }
    };

    struct BlockSplitInfo
    {
        Dictionary<IRBlock*, IRBlock*> primalBlockMap;
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