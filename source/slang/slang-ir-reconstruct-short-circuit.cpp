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
// every instruction on the path from a memory read to the chain root is
// additionally held to the same conditions under which the emitter folds an
// instruction into its use site — including the use-site vetoes it applies
// for target intrinsics and for vector-typed values
// (`shouldFoldInstIntoUseSites`, `CLikeSourceEmitter` and its
// `CPPSourceEmitter` override that CUDA inherits). Anything that fails these
// gates keeps its diamond, which is only a missed optimization. The fold
// guarantee holds at pass time: `removeRedundancy` (in `simplifyNonSSAIR`)
// can later dedup two movable `kIROp_Load`s (constant-buffer pointers, the
// speculatable ones) hoisted from two different diamonds into one two-use
// load that then emits as a statement — acceptable because movable loads
// are Slang's speculatable class, which loop-invariant hoisting already
// speculates unconditionally.

struct ReconstructShortCircuitContext : public InstPassBase
{
    CapabilitySet targetCaps;
    // Memoizes `mightHaveSideEffects(SideEffectAnalysisOptions::None, cache)`
    // results per instruction; the same handle is passed to every call in
    // this pass.
    Dictionary<IRInst*, bool> sideEffectCache;

    ReconstructShortCircuitContext(IRModule* inModule, TargetRequest* inTargetReq)
        : InstPassBase(inModule), targetCaps(inTargetReq->getTargetCaps())
    {
    }

    // Mirror of the user-side fold vetoes in
    // `CLikeSourceEmitter::shouldFoldInstIntoUseSites` and
    // `CPPSourceEmitter::shouldFoldInstIntoUseSites` (which CUDA inherits):
    // returns whether `inst`, known to be single-use and pure, will actually
    // be folded into the text of its use site by the emitter. Every
    // instruction on the path from a memory read to the chain root needs
    // this guarantee (see the file comment).
    //
    // Two emitter checks are intentionally NOT mirrored here because this
    // pass guarantees them structurally: `isSafeToFoldIntoUseSites`' scan
    // (the use must be in the same block as the definition with no
    // side-effecting instruction between them, and not into an
    // unconditional branch terminator) is provided by the rewrite hoisting
    // a contiguous pure chain into one block and merging the pass-through
    // blocks, and the `IRSwizzledStore` user veto is unreachable because a
    // store is impure and can never sit in a hoisted chain. Keep this
    // mirror and `isNeverFoldedOnCLikeTargets` (which mirrors
    // `getFoldPolicy`'s Never opcodes) in sync with the emitter functions
    // when they change.
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

            // The second switch in `CPPSourceEmitter::shouldFoldInstIntoUseSites`:
            // a vector- or matrix-typed reshape/cast instruction is itself
            // never folded, regardless of its user.
            switch (inst->getOp())
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

    // Mirror of the `FoldPolicy::Never` opcodes in
    // `CLikeSourceEmitter::getFoldPolicy` that can appear inside an
    // expression chain (declaration-like ops such as `Var`/`Param` cannot).
    // The `Make*` entries are `Never` only for function-local values
    // (`isInnerGlobalAggregate`), and chain instructions are always
    // function-local.
    bool isNeverFoldedOnCLikeTargets(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_SwizzleSet:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeCoopVector:
        case kIROp_UpdateElement:
        case kIROp_DefaultConstruct:
        case kIROp_MetalCastToDepthTexture:
        case kIROp_CoopMatMulAdd:
        case kIROp_CoopVecMatMulAdd:
        case kIROp_CoopVecOuterProductAccumulate:
        case kIROp_CoopVecReduceSumAccumulate:
            return true;
        default:
            return false;
        }
    }

    // Returns whether `inst` observes memory: loads of every flavor are
    // classified pure (loads from bad addresses are ignored throughout) but
    // still observe memory, and every call is conservatively treated as
    // memory-reading. The fold-path check composes this with
    // `!isReadNoneCall` to exempt calls to `[__readNone]` callees, which
    // provably read nothing.
    // The descriptor-heap ops below must stay in sync with the
    // FoldPolicy::Never bail in canHoistArmInst: this list routes them
    // into the fold-guarantee checks, and that bail keeps them out of a
    // reconstructed `||` entirely. kIROp_SPIRVLoadDescriptorFromHeap is
    // not in the C-family Never table, so it takes the check path.
    bool isMemoryReadingInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_Load:
        case kIROp_CUDALDG:
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

    // Returns whether `inst` is a call whose callee resolves to a function
    // marked `[__readNone]` — it provably does not read memory, so its
    // unconditional evaluation is unobservable.
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

    // Debug markers are not in `mightHaveSideEffects`' pure list, so its
    // conservative default already rejects them; the explicit checks in the
    // arm scans are a self-documenting short-circuit of that classification,
    // stating the policy that with debug info enabled an arm keeps its shape.
    static bool isDebugInst(IRInst* inst)
    {
        return as<IRDebugLine>(inst) || as<IRDebugVar>(inst) || as<IRDebugScope>(inst);
    }

    // Check one instruction that is part of the value produced by the
    // short-circuit diamond's non-constant arm, and which this pass would move
    // to the block before the diamond. `use` is the single use of `inst` when
    // it has one.
    bool canHoistArmInst(IRInst* inst)
    {
        if (isDebugInst(inst))
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
        // be freed with them. SSA dominance makes that shape unreachable in
        // valid input (neither the constant arm nor the merge block can
        // dominate the value arm), and for a matcher a conservative bail is
        // the right treatment of an out-of-contract shape: the miss only
        // forgoes the optimization, so a malformed module degrades to the
        // old code shape instead of asserting.
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

        // Verify every instruction of the arm. Arm order is def-before-use,
        // so operands are visited before the instructions that use them and
        // `foldPathInsts` is complete for each instruction when it is
        // checked.
        HashSet<IRInst*> foldPathInsts;
        for (auto inst = armBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == armBlock->getTerminator())
                continue;
            if (isDebugInst(inst))
                return false;
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
                // A memory read only stays conditionally evaluated if its
                // text lands inside the emitted `&&`/`||` operand, so every
                // instruction on the single-use path from the read to the
                // chain root must fold into its own use site: the emitter
                // prints a non-folding instruction as a statement in the
                // block before the `if`, and anything folded into that
                // statement is then evaluated unconditionally. A call to a
                // `[__readNone]` callee provably reads nothing, so it joins
                // the fold path only through its operands.
                bool onFoldPath = isMemoryReadingInst(inst) && !isReadNoneCall(inst);
                for (UInt i = 0; i < inst->getOperandCount() && !onFoldPath; i++)
                    onFoldPath = foldPathInsts.contains(inst->getOperand(i));
                if (onFoldPath)
                {
                    if (isNeverFoldedOnCLikeTargets(inst))
                        return false;
                    if (!canFoldIntoUseSite(inst, inst->firstUse))
                        return false;
                    foldPathInsts.add(inst);
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
            if (isDebugInst(inst))
                return false;
            if (inst->mightHaveSideEffects(SideEffectAnalysisOptions::None, &sideEffectCache))
                return false;
        }
        return true;
    }

    // Returns whether `block` ends in an unconditional branch directly to
    // `target` passing exactly one block argument — the property the diamond
    // matcher needs, since each arm must feed exactly the merged value — and
    // returns that argument in `outArg`.
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
        // A merge block that is the `ifElse`'s own parent block (possible
        // only for the entry block) would turn the rewrite below into an
        // unconditional self-branch; no known producer creates that shape,
        // but we check rather than assume.
        if (block == afterBlock)
            return false;
        if (thenBlock == elseBlock || thenBlock == afterBlock || elseBlock == afterBlock)
            return false;

        // Each arm must be entered only through this `ifElse`.
        if (thenBlock->getPredecessors().getCount() != 1 ||
            elseBlock->getPredecessors().getCount() != 1)
            return false;

        // The merge block's only predecessors are the two arms.
        if (afterBlock->getPredecessors().getCount() != 2)
            return false;

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
    // and the emitter folds an instruction into a use site only when the use
    // is in the same block as the definition with no side-effecting
    // instruction between them
    // (`CLikeSourceEmitter::isSafeToFoldIntoUseSitesUncached`), so
    // flattening the chain is what lets the whole `&&`/`||` chain print as
    // one expression. Each merge deallocates only the snapshot entry being
    // visited, after its last use, so iterating the snapshot is safe.
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

    // The fixpoint converges because both mutators are strictly reductive:
    // each diamond rewrite destroys one `IRIfElse`, each merge removes one
    // block, and neither ever creates one (the nested-chain case shows the
    // rounds interleaving — the inner diamond must be rewritten before the
    // outer arm becomes single-block).
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
                // A rewrite deallocates the diamond's arm blocks, which are
                // further snapshot entries (arms follow the condition block
                // in layout order), so stop at the first success and let the
                // outer fixpoint rescan from a fresh snapshot — never
                // visiting a deallocated block.
                if (tryReconstructDiamond(block, ifElse))
                {
                    changed = true;
                    break;
                }
            }

            if (mergeTrivialBlocks(func))
                changed = true;
        }
    }
};

void reconstructShortCircuit(IRModule* module, TargetRequest* targetReq)
{
    // The rewrite turns the diamond into a source-level short-circuiting
    // `&&`/`||`. The fold vetoes this pass mirrors are those of the
    // C-family emitters (`CLikeSourceEmitter`/`CPPSourceEmitter`), and the
    // temp-per-link shape the rewrite removes is only known to hurt the
    // C-family targets, so the gate stays there: the GLSL/HLSL/Metal/WGSL
    // emitters are not covered yet.
    if (!(isCUDATarget(targetReq) || (isCPUTarget(targetReq) && !isCPUTargetViaLLVM(targetReq))))
        return;

    ReconstructShortCircuitContext context(module, targetReq);
    context.processInstsOfType<IRFunc>(
        kIROp_Func,
        [&context](IRFunc* f) { context.processFunc(f); });
}

} // namespace Slang
