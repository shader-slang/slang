// slang-ir-reconstruct-short-circuit.cpp

#include "slang-ir-reconstruct-short-circuit.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-target.h"

namespace Slang
{

// Rebuild `&&`/`||` expressions from the short-circuit diamond that lowering
// creates for them.
//
// Consider this user code, on a target that emits C++-family source:
//
//     if (itemMin[it].x > qmax.x || itemMin[it].y > qmax.y)
//         continue;
//
// `visitLogicOperatorShortCircuitExpr` (slang-lower-to-ir.cpp) lowers the
// `||` into an if/else diamond: the true arm branches to the merge block with
// the constant `true`, the false arm evaluates the second operand and branches
// with its value, and the merge block starts with a `Bool` parameter that
// receives both. The emitter then cannot fold the parameter back into the
// surrounding code (`kIROp_Param` is `FoldPolicy::Never`), so the emitted
// source carries one `bool` temporary per link of the chain, and nvcc keeps
// those temporaries as `selp`/select chains instead of the compare-and-branch
// it generates for a source-level `||`.
//
// C, C++ and CUDA `&&`/`||` short-circuit natively, so this pass re-derives
// the logical operation before `eliminatePhis` destroys the block-parameter
// (phi) form that the diamond relies on:
//
//   * one arm ends in `branch(after, true)` and the other in
//     `branch(after, <second operand>)` → `Or(<first operand>, <second>)`;
//   * one arm ends in `branch(after, false)` and the other in
//     `branch(after, <second operand>)` → `And(<first operand>, <second>)`.
//
// The emitter already prints `kIROp_And`/`kIROp_Or` on scalar booleans as
// `&&`/`||` (the `-disable-short-circuit` option relies on that path), so the
// reconstructed chain emits as one source-level short-circuit expression and
// the downstream C++ compiler regains the branch structure.
//
// Safety relies on the fact that every instruction moved out of an arm is
// pure, single-use and folds into the emitted `&&`/`||` operand text: an
// operand that folds is only evaluated where the source expression evaluates
// it (a C-family `&&`/`||` only evaluates its right operand when the left one
// does not short-circuit), so the rewrite preserves the operand's
// conditional evaluation. A hoisted instruction that did *not* fold would
// run unconditionally, which for a memory-reading instruction would be a
// behavior change (the `i < n && a[i]` guarded-access idiom would fault), so
// memory-reading instructions are additionally held to the same conditions
// under which the emitter folds an instruction into its use site — including
// the use-site vetoes it applies for target intrinsics and for vector-typed
// values (`shouldFoldInstIntoUseSites`, `CLikeSourceEmitter` and its
// `CPPSourceEmitter` override that CUDA inherits). Anything that fails these
// gates keeps its diamond, which is only a missed optimization.

struct ReconstructShortCircuitContext : public InstPassBase
{
    TargetRequest* targetReq;
    CapabilitySet targetCaps;
    Dictionary<IRInst*, bool> sideEffectCache;

    ReconstructShortCircuitContext(IRModule* inModule, TargetRequest* inTargetReq)
        : InstPassBase(inModule), targetReq(inTargetReq), targetCaps(inTargetReq->getTargetCaps())
    {
    }

    // Mirror of the user-side fold vetoes in
    // `CLikeSourceEmitter::shouldFoldInstIntoUseSites` and
    // `CPPSourceEmitter::shouldFoldInstIntoUseSites` (which CUDA inherits):
    // returns whether `inst`, known to be single-use and pure, will actually
    // be folded into the text of its use site by the emitter. Only
    // memory-reading instructions need this guarantee (see the file comment).
    bool canFoldIntoUseSite(IRInst* inst, IRUse* use)
    {
        auto user = use->getUser();

        // A call to a target intrinsic that mentions this parameter more than
        // once in its definition does not fold its arguments in, because the
        // folded text would evaluate them multiple times.
        if (auto callInst = as<IRCall>(user))
        {
            UnownedStringSlice intrinsicDef;
            IRInst* intrinsicInst;
            if (findTargetIntrinsicDefinition(
                    callInst->getCallee(),
                    targetCaps,
                    intrinsicDef,
                    intrinsicInst))
            {
                IRUse* args = callInst->getArgs();
                const Index paramIndex = Index(use - args);
                SLANG_ASSERT(paramIndex >= 0 && paramIndex < Index(callInst->getArgCount()));

                // Count `$0`..`$9` references to `paramIndex` in the
                // intrinsic definition text.
                Index useCount = 0;
                for (auto cur = intrinsicDef.begin(); cur < intrinsicDef.end();)
                {
                    const char c = *cur;
                    if (c == '$' && cur + 1 < intrinsicDef.end() && cur[1] >= '0' && cur[1] <= '9')
                    {
                        const Index index = Index(cur[1] - '0');
                        useCount += Index(index == paramIndex);
                        cur += 2;
                    }
                    else
                    {
                        cur++;
                    }
                }
                if (useCount > 1)
                    return false;
            }
        }

        // The C++/CUDA emitters build a multi-component swizzle result
        // element by element, re-emitting the base operand once per
        // component, and reshape/cast code likewise refers to its operand
        // multiple times; a vector- or matrix-typed value used that way is
        // kept as its own temporary instead of being folded in.
        if (as<IRVectorType>(inst->getDataType()) || as<IRMatrixType>(inst->getDataType()))
        {
            switch (user->getOp())
            {
            case kIROp_MatrixReshape:
            case kIROp_VectorReshape:
            case kIROp_IntCast:
            case kIROp_FloatCast:
            case kIROp_CastIntToFloat:
            case kIROp_CastFloatToInt:
                return false;
            default:
                break;
            }
            if (auto swizzle = as<IRSwizzle>(user))
            {
                if (swizzle->getBase() == inst && swizzle->getElementCount() > 1)
                    return false;
            }
        }

        // A call to a resource object's member function is kept as its own
        // statement for readability, so it never folds.
        if (auto call = as<IRCall>(inst))
        {
            auto callee = getResolvedInstForDecorations(call->getCallee());
            if (callee->findDecoration<IRTargetIntrinsicDecoration>())
            {
                if (auto funcType = as<IRFuncType>(callee->getDataType()))
                {
                    if (funcType->getParamCount() > 0)
                    {
                        auto firstParamType = funcType->getParamType(0);
                        if (as<IRResourceTypeBase>(firstParamType))
                            return false;
                        if (as<IRHLSLStructuredBufferTypeBase>(firstParamType))
                            return false;
                        if (as<IRUntypedBufferResourceType>(firstParamType))
                            return false;
                        if (as<IRSamplerStateTypeBase>(firstParamType))
                            return false;
                    }
                }
            }
        }

        return true;
    }

    bool isMemoryReadingInst(IRInst* inst)
    {
        // Loads of every flavor are classified pure (loads from bad addresses
        // are ignored throughout), but they still observe memory, so they
        // need the fold guarantee. A call is memory-reading unless its callee
        // is marked `[__readNone]`; we conservatively treat every pure call
        // as memory-reading.
        // The descriptor-heap ops below must stay in sync with the
        // FoldPolicy::Never bail in canHoistArmInst: this list routes them
        // into the fold-guarantee checks, and that bail keeps them out of a
        // reconstructed `||` entirely. kIROp_SPIRVLoadDescriptorFromHeap is
        // not in the C-family Never table, so it takes the check path.
        switch (inst->getOp())
        {
        case kIROp_Load:
        case kIROp_StructuredBufferLoad:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_ByteAddressBufferLoad:
        case kIROp_LoadSamplerDescriptorFromHeap:
        case kIROp_LoadResourceDescriptorFromHeap:
        case kIROp_SPIRVLoadDescriptorFromHeap:
        case kIROp_ImageSubscript:
            return true;
        default:
            return as<IRCall>(inst) != nullptr;
        }
    }

    // Returns true when `inst` is a memory-reading instruction whose callee
    // (if it is a call) is marked `[__readNone]`, meaning it provably does
    // not read memory and unconditional evaluation is unobservable.
    bool isReadNoneCall(IRInst* inst)
    {
        auto call = as<IRCall>(inst);
        if (!call)
            return false;
        auto callee = getResolvedInstForDecorations(call->getCallee());
        if (as<IRFunc>(callee))
            return callee->findDecoration<IRReadNoneDecoration>() != nullptr;
        return false;
    }

    // Check one instruction that is part of the value produced by the
    // short-circuit diamond's non-constant arm, and which this pass would move
    // to the block before the diamond. `use` is the single use of `inst` when
    // it has one.
    bool canHoistArmInst(IRInst* inst)
    {
        // Debug instructions are left in place: with debug info enabled the
        // arm keeps its shape, which only forgoes this optimization.
        if (as<IRDebugLine>(inst) || as<IRDebugVar>(inst) || as<IRDebugScope>(inst))
            return false;

        // Descriptor-heap loads are pure, so the side-effect check below does
        // not reject them, but the C-family emitter classifies them as
        // FoldPolicy::Never (slang-emit-c-like.cpp): their result cannot be
        // emitted as a sub-expression of the reconstructed `||`. Keeping the
        // diamond for them mirrors that classification instead of relying on
        // the lowering paths that produce them not reaching C-family targets.
        switch (inst->getOp())
        {
        case kIROp_LoadResourceDescriptorFromHeap:
        case kIROp_LoadSamplerDescriptorFromHeap:
            return false;
        default:
            break;
        }

        if (inst->mightHaveSideEffects(SideEffectAnalysisOptions::None, &sideEffectCache))
            return false;

        if (inst->findDecoration<IRPreciseDecoration>())
            return false;

        return true;
    }

    // Collect the instructions of `armBlock` that transitively define
    // `value` (the arm's branch argument). Every other non-terminator
    // instruction of the arm must be unused, so the arm carries no side
    // effects and nothing besides this value. Returns false (and changes
    // nothing) when the arm does not qualify.
    bool tryCollectArmChain(
        IRBlock* armBlock,
        IRBlock* constArmBlock,
        IRBlock* afterBlock,
        IRInst* value,
        List<IRInst*>& outChain)
    {
        // The branch argument must be computed in this arm or already
        // dominate the diamond; a value defined in the sibling blocks would
        // be freed with them (SSA validity makes that impossible in practice,
        // but we check rather than assume).
        {
            auto valueBlock = as<IRBlock>(value->getParent());
            if (valueBlock == constArmBlock || valueBlock == afterBlock)
                return false;
        }

        // The arm must not have parameters: its only predecessor is the
        // `ifElse`, which passes no arguments.
        for (auto inst = armBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (as<IRParam>(inst))
                return false;
        }

        List<IRInst*> workList;
        HashSet<IRInst*> inChain;
        workList.add(value);
        inChain.add(value);
        while (workList.getCount() > 0)
        {
            auto inst = workList.getLast();
            workList.removeLast();

            auto operandCount = inst->getOperandCount();
            for (UInt i = 0; i < operandCount; i++)
            {
                auto operand = inst->getOperand(i);
                auto operandBlock = as<IRBlock>(operand->getParent());
                if (operandBlock != armBlock)
                {
                    // A value defined outside the arm must not be defined in
                    // the diamond's other blocks: the constant arm is deleted
                    // and the merge block's parameter is being replaced, so
                    // only values that already dominate the diamond qualify.
                    // (SSA validity rules out everything but dominators of
                    // the arm anyway, since neither the constant arm nor the
                    // merge block can reach it.)
                    if (operandBlock == constArmBlock || operandBlock == afterBlock)
                        return false;
                    continue;
                }
                if (inChain.add(operand))
                    workList.add(operand);
            }
            // A type defined inside the arm cannot be moved meaningfully;
            // reject that (rare) shape.
            auto fullType = inst->getFullType();
            if (fullType && as<IRBlock>(fullType->getParent()) == armBlock)
                return false;
        }

        // Verify every instruction of the arm.
        List<IRInst*> armInsts;
        for (auto inst = armBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == armBlock->getTerminator())
                continue;
            bool isChain = inChain.contains(inst);
            if (!isChain && inst->firstUse)
                return false; // unrelated live instruction: bail
            if (isChain)
            {
                if (!canHoistArmInst(inst))
                    return false;
                // Single use, and the use must be within the arm's chain.
                if (!inst->firstUse || inst->firstUse->nextUse)
                    return false;
                auto user = inst->firstUse->getUser();
                if (!inChain.contains(user) && user != armBlock->getTerminator())
                    return false;
                if (isMemoryReadingInst(inst) && !isReadNoneCall(inst))
                {
                    if (!canFoldIntoUseSite(inst, inst->firstUse))
                        return false;
                }
            }
            else if (inst->mightHaveSideEffects(SideEffectAnalysisOptions::None, &sideEffectCache))
            {
                // Unused but impure (for example a branch-coverage counter):
                // keep the diamond.
                return false;
            }
        }

        // Emit the chain in arm order so the moved instructions keep their
        // def-before-use order in the destination block.
        for (auto inst = armBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inChain.contains(inst))
                outChain.add(inst);
        }
        return true;
    }

    bool isBoolConst(IRInst* inst, bool expectedValue)
    {
        auto lit = as<IRBoolLit>(inst);
        if (!lit)
            return false;
        return lit->getValue() == expectedValue;
    }

    // Verify that an arm only contains instructions that are unused and pure
    // (they are deleted with the arm), plus its terminator.
    bool isArmRemovable(IRBlock* armBlock)
    {
        for (auto inst = armBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == armBlock->getTerminator())
                break;
            if (as<IRParam>(inst))
                return false;
            if (inst->firstUse)
                return false;
            if (as<IRDebugLine>(inst) || as<IRDebugVar>(inst) || as<IRDebugScope>(inst))
                return false;
            if (inst->mightHaveSideEffects(SideEffectAnalysisOptions::None, &sideEffectCache))
                return false;
        }
        return true;
    }

    bool isSimpleBranchTo(IRBlock* block, IRBlock* target, IRInst*& outArg)
    {
        auto terminator = block->getTerminator();
        if (!terminator || terminator->getOp() != kIROp_UnconditionalBranch)
            return false;
        auto branch = cast<IRUnconditionalBranch>(terminator);
        if (branch->getTargetBlock() != target)
            return false;
        if (branch->getArgCount() != 1)
            return false;
        outArg = branch->getArg(0);
        return true;
    }

    // Try to rebuild one short-circuit diamond rooted at `ifElse` (the
    // terminator of `block`) into an `And`/`Or`. Returns true when the
    // diamond was rewritten.
    bool tryReconstructDiamond(IRBlock* block, IRIfElse* ifElse)
    {
        auto afterBlock = ifElse->getAfterBlock();
        auto thenBlock = ifElse->getTrueBlock();
        auto elseBlock = ifElse->getFalseBlock();
        if (!afterBlock || !thenBlock || !elseBlock)
            return false;
        if (thenBlock == elseBlock || thenBlock == afterBlock || elseBlock == afterBlock)
            return false;

        // Each arm must be entered only through this `ifElse`.
        if (thenBlock->getPredecessors().getCount() != 1 ||
            elseBlock->getPredecessors().getCount() != 1)
            return false;

        // The merge block's only predecessors are the two arms.
        {
            UInt predCount = 0;
            for (auto pred [[maybe_unused]] : afterBlock->getPredecessors())
                predCount++;
            if (predCount != 2)
                return false;
        }

        // The merge block has a single `Bool` parameter, used exactly once.
        IRParam* param = nullptr;
        UInt paramCount = 0;
        for (auto inst = afterBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (as<IRParam>(inst))
            {
                paramCount++;
                if (!param)
                    param = as<IRParam>(inst);
            }
        }
        if (paramCount != 1 || !param)
            return false;
        if (!param->getDataType() || param->getDataType()->getOp() != kIROp_BoolType)
            return false;
        if (!param->firstUse || param->firstUse->nextUse)
            return false;

        IRInst* thenVal = nullptr;
        IRInst* elseVal = nullptr;
        if (!isSimpleBranchTo(thenBlock, afterBlock, thenVal))
            return false;
        if (!isSimpleBranchTo(elseBlock, afterBlock, elseVal))
            return false;

        IROp logicOp = kIROp_Nop;
        IRBlock* valueArm = nullptr;
        IRBlock* constArm = nullptr;
        IRInst* secondOperand = nullptr;
        if (isBoolConst(thenVal, true))
        {
            // `ifElse(c) { true } else { v }` is `c || v`.
            logicOp = kIROp_Or;
            valueArm = elseBlock;
            constArm = thenBlock;
            secondOperand = elseVal;
        }
        else if (isBoolConst(elseVal, false))
        {
            // `ifElse(c) { v } else { false }` is `c && v`.
            logicOp = kIROp_And;
            valueArm = thenBlock;
            constArm = elseBlock;
            secondOperand = thenVal;
        }
        else
        {
            return false;
        }

        // The arm carrying the second operand must only compute that operand.
        List<IRInst*> chain;
        if (!tryCollectArmChain(valueArm, constArm, afterBlock, secondOperand, chain))
            return false;

        // The constant arm must be removable as-is.
        if (!isArmRemovable(constArm))
            return false;

        // Rebuild: move the second operand's computation in front of the
        // diamond, fold it into a logical operation with the first operand,
        // and let the merge block's parameter become the operation's result.
        for (auto inst : chain)
        {
            inst->insertBefore(ifElse);
        }

        IRBuilder builder(module);
        builder.setInsertBefore(ifElse);
        auto firstOperand = ifElse->getCondition();
        IRInst* logicInst = nullptr;
        if (logicOp == kIROp_Or)
            logicInst = builder.emitOr(param->getFullType(), firstOperand, secondOperand);
        else
            logicInst = builder.emitAnd(param->getFullType(), firstOperand, secondOperand);

        // The parameter carries the user-visible name of the result (for
        // example `passed` in `bool passed = a && b;`); keep it on the
        // reconstructed operation.
        param->transferDecorationsTo(logicInst);
        param->replaceUsesWith(logicInst);
        param->removeAndDeallocate();
        ifElse->removeAndDeallocate();
        builder.setInsertInto(block);
        builder.emitBranch(afterBlock);

        constArm->removeAndDeallocate();
        valueArm->removeAndDeallocate();
        return true;
    }

    // Fold single-predecessor blocks with no parameters into their
    // predecessor. The diamond rewrite leaves chains of such pass-through
    // blocks behind (each former merge block now only forwards control flow),
    // and the emitter folds an instruction into a use site only within the
    // same basic block, so flattening the chain is what lets the whole
    // `&&`/`||` chain print as one expression.
    bool mergeTrivialBlocks(IRGlobalValueWithCode* func)
    {
        bool changed = false;
        List<IRBlock*> blocks;
        for (auto block : func->getBlocks())
            blocks.add(block);

        for (auto block : blocks)
        {
            if (block == func->getFirstBlock())
                continue;

            bool hasParams = false;
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                if (as<IRParam>(inst))
                {
                    hasParams = true;
                    break;
                }
            }
            if (hasParams)
                continue;

            if (block->getPredecessors().getCount() != 1)
                continue;
            auto predBlock = *block->getPredecessors().begin();
            auto predTerminator = predBlock->getTerminator();
            if (!predTerminator || predTerminator->getOp() != kIROp_UnconditionalBranch)
                continue;
            auto predBranch = cast<IRUnconditionalBranch>(predTerminator);
            if (predBranch->getTargetBlock() != block || predBranch->getArgCount() != 0)
                continue;

            // A block referenced by anything other than the predecessor's
            // branch (for example as the `afterBlock` operand of a live
            // `ifElse`, which is a use but not an edge, and so is invisible
            // to the predecessor count) must not be deleted.
            UInt useCount = 0;
            for (auto use = block->firstUse; use; use = use->nextUse)
                useCount++;
            if (useCount != 1 || block->firstUse->getUser() != predBranch)
                continue;

            predTerminator->removeAndDeallocate();
            List<IRInst*> insts;
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
                insts.add(inst);
            for (auto inst : insts)
                inst->insertAtEnd(predBlock);
            block->removeAndDeallocate();
            changed = true;
        }
        return changed;
    }

    void processFunc(IRGlobalValueWithCode* func)
    {
        bool changed = true;
        while (changed)
        {
            changed = false;

            List<IRBlock*> blocks;
            for (auto block : func->getBlocks())
                blocks.add(block);
            for (auto block : blocks)
            {
                auto ifElse = as<IRIfElse>(block->getTerminator());
                if (!ifElse)
                    continue;
                if (tryReconstructDiamond(block, ifElse))
                    changed = true;
            }

            if (mergeTrivialBlocks(func))
                changed = true;
        }
    }
};

void reconstructShortCircuit(IRModule* module, TargetRequest* targetReq)
{
    // The rewrite turns the diamond into a source-level short-circuiting
    // `&&`/`||`, which is only valid where the emitted language
    // short-circuits: the C-family source targets. WGSL's `&&`/`||` do not
    // short-circuit, and the GLSL/HLSL/Metal emitters are not covered yet.
    if (!(isCUDATarget(targetReq) || (isCPUTarget(targetReq) && !isCPUTargetViaLLVM(targetReq))))
        return;

    ReconstructShortCircuitContext context(module, targetReq);
    context.processInstsOfType<IRFunc>(
        kIROp_Func,
        [&context](IRFunc* f) { context.processFunc(f); });
}

} // namespace Slang
