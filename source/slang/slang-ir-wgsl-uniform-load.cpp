#include "slang-ir-wgsl-uniform-load.h"

#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Consider this compute shader, where one thread publishes a loop bound to the workgroup:
//
// ```
// groupshared uint gCount;
//
// if (tid.x == 0)
//     gCount = counts[gid.x];
// GroupMemoryBarrierWithGroupSync();
// for (uint i = 0; i < gCount; ++i)
// {
//     ...
//     GroupMemoryBarrierWithGroupSync();
// }
// ```
//
// Every thread reads the same `gCount`, so the barrier in the loop runs uniformly, which is all
// that HLSL, SPIR-V and Metal require. WGSL's uniformity analysis instead treats the read of the
// `var<workgroup>` in the loop condition as possibly non-uniform, and rejects the
// `workgroupBarrier()` in the loop body. Reading `gCount` with `workgroupUniformLoad`, which runs
// a barrier and then the load and returns a value that WGSL treats as uniform, makes the module
// valid.
//
// We find the groupshared loads that a barrier depends on and replace each one with an
// `IRWorkgroupUniformLoad`. We follow the rules of WGSL's uniformity analysis: a barrier depends
// on the condition of every branch that decides whether the barrier's block runs, and a value
// depends on its operands and on the branches that decide whether the block computing it runs.
// We compute "decides" as control dependence on the CFG, so a loop exit, a `break` and an early
// `return` decide whether the code after them runs, while a branch that every path leaves to
// reach the barrier does not. WGSL also treats a branch that can `continue` as deciding every
// block of its loop. WGSL rejects every module in which a barrier depends on a plain groupshared
// read, so the pass only changes modules that WGSL would reject.

// Returns true if a call to the core-module function `name` lowers on WGSL to a builtin that WGSL
// only allows in uniform control flow.
static bool isUniformControlFlowBuiltin(KnownBuiltinDeclName name)
{
    switch (name)
    {
    case KnownBuiltinDeclName::AllMemoryBarrier:
    case KnownBuiltinDeclName::AllMemoryBarrierWithGroupSync:
    case KnownBuiltinDeclName::DeviceMemoryBarrier:
    case KnownBuiltinDeclName::DeviceMemoryBarrierWithGroupSync:
    case KnownBuiltinDeclName::GroupMemoryBarrier:
    case KnownBuiltinDeclName::GroupMemoryBarrierWithGroupSync:
    case KnownBuiltinDeclName::workgroupUniformLoad:
        return true;
    default:
        return false;
    }
}

static bool isWorkgroupPointer(IRInst* ptr)
{
    auto ptrType = as<IRPtrTypeBase>(ptr->getDataType());
    return ptrType && ptrType->hasAddressSpace() &&
           ptrType->getAddressSpace() == AddressSpace::GroupShared;
}

// The blocks whose terminator decides, for WGSL's uniformity analysis, whether a given block of one
// function runs.
//
// Consider this loop:
//
// ```
// for (uint i = 0; i < n; ++i) { barrier(); }
// barrier();
// ```
//
// The loop body is control dependent on the loop header, which branches on `i < n`. The barrier
// after the loop is not, because every path from the header reaches it.
//
// We compute control dependence from the post-dominator tree, following Ferrante, Ottenstein and
// Warren: for each edge `B -> S`, every block on the post-dominator tree path from `S` up to, but
// excluding, the immediate post-dominator of `B` is control dependent on `B`. A virtual exit node
// post-dominates every block, and every block without a successor branches to it. We then add the
// dependences that WGSL's rule for `continue` implies (see `addContinueDependences`).
struct ControlDependence
{
    void compute(IRFunc* func);

    List<IRBlock*> const& getControllingBlocks(IRBlock* block)
    {
        return m_controllingBlocks[m_blockIndex.getValue(block)];
    }

private:
    void addControllingBlock(IRBlock* block, IRBlock* controllingBlock)
    {
        auto& controllingBlocks = m_controllingBlocks[m_blockIndex.getValue(block)];
        if (!controllingBlocks.contains(controllingBlock))
            controllingBlocks.add(controllingBlock);
    }

    void addPostDominanceDependences(IRFunc* func);
    void addContinueDependences();

    Dictionary<IRBlock*, Index> m_blockIndex;
    List<IRBlock*> m_blocks;
    List<List<IRBlock*>> m_controllingBlocks;
};

void ControlDependence::compute(IRFunc* func)
{
    for (auto block : func->getBlocks())
    {
        m_blockIndex[block] = m_blocks.getCount();
        m_blocks.add(block);
    }
    m_controllingBlocks.setCount(m_blocks.getCount());
    addPostDominanceDependences(func);
    addContinueDependences();
}

void ControlDependence::addPostDominanceDependences(IRFunc* func)
{
    const Index blockCount = m_blocks.getCount();
    const Index exitIndex = blockCount;

    List<List<Index>> successors;
    successors.setCount(blockCount);
    for (Index b = 0; b < blockCount; b++)
    {
        for (auto successor : m_blocks[b]->getSuccessors())
        {
            Index s = m_blockIndex.getValue(successor);
            if (!successors[b].contains(s))
                successors[b].add(s);
        }
        if (successors[b].getCount() == 0)
            successors[b].add(exitIndex);
    }

    // We number the nodes in postorder of a depth-first walk of the reversed CFG that starts at
    // the virtual exit, which therefore gets the highest number. Blocks that cannot reach a
    // `return`, such as the body of a loop without a `break`, stay unnumbered.
    List<IRBlock*> postorder;
    computePostorderOnReverseCFG(func, postorder);
    List<Index> postorderNumber;
    postorderNumber.setCount(blockCount + 1);
    for (auto& n : postorderNumber)
        n = -1;
    for (Index k = 0; k < postorder.getCount(); k++)
        postorderNumber[m_blockIndex.getValue(postorder[k])] = k;
    postorderNumber[exitIndex] = postorder.getCount();

    // We compute immediate post-dominators with the iterative algorithm of Cooper, Harvey and
    // Kennedy, run on the reversed CFG.
    List<Index> ipdom;
    ipdom.setCount(blockCount + 1);
    for (auto& d : ipdom)
        d = -1;
    ipdom[exitIndex] = exitIndex;
    auto intersect = [&](Index a, Index b)
    {
        while (a != b)
        {
            while (postorderNumber[a] < postorderNumber[b])
                a = ipdom[a];
            while (postorderNumber[b] < postorderNumber[a])
                b = ipdom[b];
        }
        return a;
    };
    for (bool changed = true; changed;)
    {
        changed = false;
        for (Index k = postorder.getCount() - 1; k >= 0; k--)
        {
            Index b = m_blockIndex.getValue(postorder[k]);
            Index newIpdom = -1;
            for (auto s : successors[b])
            {
                if (ipdom[s] == -1)
                    continue;
                newIpdom = newIpdom == -1 ? s : intersect(s, newIpdom);
            }
            if (newIpdom != ipdom[b])
            {
                ipdom[b] = newIpdom;
                changed = true;
            }
        }
    }
    // A block that cannot reach the exit has no post-dominator; attaching it to the exit makes
    // every branch that leads into it control it, which keeps the relation conservative.
    for (auto& d : ipdom)
    {
        if (d == -1)
            d = exitIndex;
    }

    for (Index b = 0; b < blockCount; b++)
    {
        if (successors[b].getCount() < 2)
            continue;
        for (auto s : successors[b])
        {
            for (Index runner = s; runner != ipdom[b] && runner != exitIndex;
                 runner = ipdom[runner])
                addControllingBlock(m_blocks[runner], m_blocks[b]);
        }
    }
}

static HashSet<IRBlock*> collectReachableBlocks(
    List<IRBlock*> const& starts,
    HashSet<IRBlock*> const& stops)
{
    HashSet<IRBlock*> reached;
    List<IRBlock*> workList;
    for (auto start : starts)
    {
        if (!stops.contains(start) && reached.add(start))
            workList.add(start);
    }
    while (workList.getCount())
    {
        auto block = workList.getLast();
        workList.removeLast();
        for (auto successor : block->getSuccessors())
        {
            if (!stops.contains(successor) && reached.add(successor))
                workList.add(successor);
        }
    }
    return reached;
}

// WGSL's uniformity analysis is more conservative than control dependence for `continue`.
// Consider:
//
// ```
// for (uint i = 0; i < 4; ++i)
// {
//     barrier();
//     if (gFlag != 0)
//         continue;
//     ...
// }
// ```
//
// Both sides of the `if` reach the next iteration, so the barrier is not control dependent on it.
// WGSL instead makes the control flow after an `if` that can `continue` depend on its condition,
// and the start of each iteration depend on the end of the previous one, so every block of the
// loop depends on `gFlag`. We add that dependence for each `if` or `switch` that has a path to the
// continue block of an enclosing loop that does not go through its own merge block.
void ControlDependence::addContinueDependences()
{
    struct LoopInfo
    {
        IRLoop* loop;
        HashSet<IRBlock*> blocks;
    };
    List<LoopInfo> loops;
    for (auto block : m_blocks)
    {
        if (auto loop = as<IRLoop>(block->getTerminator()))
        {
            HashSet<IRBlock*> stops;
            stops.add(loop->getBreakBlock());
            loops.add({loop, collectReachableBlocks({loop->getTargetBlock()}, stops)});
        }
    }

    for (auto block : m_blocks)
    {
        auto terminator = block->getTerminator();
        IRBlock* mergeBlock = nullptr;
        if (auto ifElse = as<IRIfElse>(terminator))
            mergeBlock = ifElse->getAfterBlock();
        else if (auto switchInst = as<IRSwitch>(terminator))
            mergeBlock = switchInst->getBreakLabel();
        else
            continue;

        // The branch's own region ends at its merge block and at the jumps that leave the
        // enclosing loops.
        HashSet<IRBlock*> stops;
        stops.add(mergeBlock);
        for (auto& loopInfo : loops)
        {
            if (!loopInfo.blocks.contains(block))
                continue;
            stops.add(loopInfo.loop->getTargetBlock());
            stops.add(loopInfo.loop->getContinueBlock());
            stops.add(loopInfo.loop->getBreakBlock());
        }
        List<IRBlock*> successors;
        for (auto successor : block->getSuccessors())
            successors.add(successor);
        auto region = collectReachableBlocks(successors, stops);

        auto canJumpTo = [&](IRBlock* target)
        {
            if (target == mergeBlock)
                return false;
            for (auto successor : successors)
            {
                if (successor == target)
                    return true;
            }
            for (auto regionBlock : region)
            {
                for (auto successor : regionBlock->getSuccessors())
                {
                    if (successor == target)
                        return true;
                }
            }
            return false;
        };
        for (auto& loopInfo : loops)
        {
            if (!loopInfo.blocks.contains(block))
                continue;
            if (!canJumpTo(loopInfo.loop->getContinueBlock()) &&
                !canJumpTo(loopInfo.loop->getTargetBlock()))
                continue;
            for (auto loopBlock : loopInfo.blocks)
                addControllingBlock(loopBlock, block);
        }
    }
}

// Returns the value that `block`'s terminator branches on. `block` must end in a terminator with
// more than one successor.
static IRInst* getBranchCondition(IRBlock* block)
{
    auto terminator = block->getTerminator();
    if (auto conditionalBranch = as<IRConditionalBranch>(terminator))
        return conditionalBranch->getCondition();
    auto switchInst = as<IRSwitch>(terminator);
    SLANG_RELEASE_ASSERT(switchInst);
    return switchInst->getCondition();
}

struct WorkgroupUniformLoadContext
{
    // What the callers of one function need to know about it.
    struct FunctionInfo : RefObject
    {
        // Whether the function, or a function it calls, runs a builtin that WGSL only allows in
        // uniform control flow. Every call to such a function has the same requirement.
        bool requiresUniformControlFlow = false;

        // Indices of the parameters that the function's barriers depend on. The caller must pass
        // a uniform value for each of them.
        HashSet<Index> uniformParams;

        // Indices of the parameters that the function's return value depends on, filled in on
        // first use by `getReturnDependencies`.
        HashSet<Index> returnDependencies;
        bool hasReturnDependencies = false;

        bool isInProgress = true;
        ControlDependence controlDependence;
    };

    // A backward walk from values and blocks that must be uniform to everything they depend on.
    //
    // A value must be uniform when a barrier depends on it, and a block must run in uniform
    // control flow when it calls a barrier. The walk follows the operands of each value, the
    // incoming arguments and predecessor blocks of a block parameter (the predecessor that runs
    // decides which argument the parameter takes), the arguments of a call that its result
    // depends on, and the branch condition of every block that controls a block that must run in
    // uniform control flow.
    struct DependenceWalk
    {
        DependenceWalk(WorkgroupUniformLoadContext* context, IRFunc* func, FunctionInfo* info)
            : context(context), func(func), info(info)
        {
        }

        WorkgroupUniformLoadContext* context;
        IRFunc* func;
        FunctionInfo* info;

        List<IRInst*> groupSharedLoads;
        HashSet<Index> params;

        List<IRInst*> valueWorkList;
        HashSet<IRInst*> visitedValues;
        List<IRBlock*> blockWorkList;
        HashSet<IRBlock*> visitedBlocks;

        void addValue(IRInst* value)
        {
            // The walk stops at values outside `func`'s body, such as constants and module-scope
            // variables, because the pass cannot change how they are read.
            auto block = as<IRBlock>(value->getParent());
            if (!block || block->getParent() != func)
                return;
            if (visitedValues.add(value))
                valueWorkList.add(value);
        }

        void addBlock(IRBlock* block)
        {
            if (visitedBlocks.add(block))
                blockWorkList.add(block);
        }

        void visitValue(IRInst* value)
        {
            addBlock(cast<IRBlock>(value->getParent()));
            switch (value->getOp())
            {
            case kIROp_Param:
                {
                    auto param = cast<IRParam>(value);
                    auto block = cast<IRBlock>(param->getParent());
                    auto paramIndex = block->getParamIndex(param);
                    if (block == func->getFirstBlock())
                    {
                        params.add(paramIndex);
                        break;
                    }
                    for (auto predecessor : block->getPredecessors())
                    {
                        auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                        SLANG_RELEASE_ASSERT(branch);
                        addValue(branch->getArg(paramIndex));
                        addBlock(predecessor);
                    }
                }
                break;

            case kIROp_Load:
            case kIROp_AtomicLoad:
                {
                    auto ptr = value->getOperand(0);
                    if (isWorkgroupPointer(ptr))
                        groupSharedLoads.add(value);
                    addValue(ptr);
                }
                break;

            case kIROp_Call:
                {
                    auto call = cast<IRCall>(value);
                    auto callee = as<IRFunc>(call->getCallee());
                    if (callee && callee->isDefinition())
                    {
                        auto calleeInfo = context->getReturnDependencies(callee);
                        for (auto paramIndex : calleeInfo->returnDependencies)
                            addValue(call->getArg(paramIndex));
                        break;
                    }
                    for (UInt i = 0; i < call->getArgCount(); i++)
                        addValue(call->getArg(i));
                }
                break;

            default:
                for (UInt i = 0; i < value->getOperandCount(); i++)
                    addValue(value->getOperand(i));
                break;
            }
        }

        void run()
        {
            for (;;)
            {
                if (blockWorkList.getCount())
                {
                    auto block = blockWorkList.getLast();
                    blockWorkList.removeLast();
                    for (auto controllingBlock :
                         info->controlDependence.getControllingBlocks(block))
                    {
                        addBlock(controllingBlock);
                        addValue(getBranchCondition(controllingBlock));
                    }
                }
                else if (valueWorkList.getCount())
                {
                    auto value = valueWorkList.getLast();
                    valueWorkList.removeLast();
                    visitValue(value);
                }
                else
                {
                    break;
                }
            }
        }
    };

    Dictionary<IRFunc*, RefPtr<FunctionInfo>> functionInfos;
    bool hasReplacedLoads = false;

    void replaceGroupSharedLoads(DependenceWalk& walk)
    {
        for (auto load : walk.groupSharedLoads)
            replaceWithWorkgroupUniformLoad(load);
        if (walk.groupSharedLoads.getCount())
            hasReplacedLoads = true;
    }

    // Callees are analysed before their callers, because a call to a function that requires
    // uniform control flow is itself such a call.
    FunctionInfo* getFunctionInfo(IRFunc* func)
    {
        if (auto existing = functionInfos.tryGetValue(func))
            return *existing;
        RefPtr<FunctionInfo> info = new FunctionInfo();
        functionInfos[func] = info;

        DependenceWalk walk(this, func, info);
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                // WGSL also requires the pointer passed to `workgroupUniformLoad` to be uniform.
                if (inst->getOp() == kIROp_WorkgroupUniformLoad)
                {
                    walk.addBlock(block);
                    walk.addValue(inst->getOperand(0));
                    continue;
                }
                auto call = as<IRCall>(inst);
                if (!call)
                    continue;
                auto builtin = getBuiltinFuncEnum(call->getCallee());
                if (isUniformControlFlowBuiltin(builtin))
                {
                    walk.addBlock(block);
                    if (builtin == KnownBuiltinDeclName::workgroupUniformLoad)
                        walk.addValue(call->getArg(0));
                    continue;
                }
                auto callee = as<IRFunc>(call->getCallee());
                if (!callee || !callee->isDefinition())
                    continue;
                auto calleeInfo = getFunctionInfo(callee);
                if (!calleeInfo->requiresUniformControlFlow)
                    continue;
                walk.addBlock(block);
                for (auto paramIndex : calleeInfo->uniformParams)
                    walk.addValue(call->getArg(paramIndex));
            }
        }

        if (walk.blockWorkList.getCount())
        {
            info->requiresUniformControlFlow = true;
            info->controlDependence.compute(func);
            walk.run();
            info->uniformParams = _Move(walk.params);
            replaceGroupSharedLoads(walk);
        }
        info->isInProgress = false;
        return info;
    }

    // A caller needs the result of `func` to be uniform, so the groupshared loads that the result
    // depends on are replaced as well.
    FunctionInfo* getReturnDependencies(IRFunc* func)
    {
        auto info = getFunctionInfo(func);
        // WGSL does not allow recursion, so a function still in progress here can only be reached
        // from itself; it contributes no dependencies.
        if (info->hasReturnDependencies || info->isInProgress)
            return info;
        info->hasReturnDependencies = true;
        if (!info->requiresUniformControlFlow)
            info->controlDependence.compute(func);

        DependenceWalk walk(this, func, info);
        for (auto block : func->getBlocks())
        {
            auto terminator = block->getTerminator();
            if (auto returnInst = as<IRReturn>(terminator))
            {
                walk.addValue(returnInst->getVal());
                walk.addBlock(block);
            }
            else if (as<IRGenericAsm>(terminator))
            {
                // A function whose body is target assembly computes its result from all of its
                // parameters.
                for (Index i = 0; i < Index(func->getParamCount()); i++)
                    walk.params.add(i);
            }
        }
        walk.run();
        info->returnDependencies = _Move(walk.params);
        replaceGroupSharedLoads(walk);
        return info;
    }

    // WGSL cannot take the address of a vector component, so a load of `v[i]` becomes
    // `workgroupUniformLoad(&v)[i]`.
    static void replaceWithWorkgroupUniformLoad(IRInst* load)
    {
        IRBuilder builder(load);
        builder.setInsertBefore(load);
        IRInst* ptr = load->getOperand(0);
        IRInst* vectorIndex = nullptr;
        if (auto elementPtr = as<IRGetElementPtr>(ptr))
        {
            auto basePtrType = as<IRPtrTypeBase>(elementPtr->getBase()->getDataType());
            if (basePtrType && as<IRVectorType>(basePtrType->getValueType()))
            {
                vectorIndex = elementPtr->getIndex();
                ptr = elementPtr->getBase();
            }
        }
        IRType* loadedType = vectorIndex ? as<IRPtrTypeBase>(ptr->getDataType())->getValueType()
                                         : load->getFullType();
        IRInst* value = builder.emitIntrinsicInst(loadedType, kIROp_WorkgroupUniformLoad, 1, &ptr);
        if (vectorIndex)
            value = builder.emitElementExtract(load->getFullType(), value, vectorIndex);
        load->transferDecorationsTo(value);
        load->replaceUsesWith(value);
        load->removeAndDeallocate();
    }
};

void insertWorkgroupUniformLoadsForWGSL(IRModule* module)
{
    // A replaced load makes its function require uniform control flow, which callers analysed
    // earlier did not account for, so we repeat the analysis until nothing changes.
    for (bool changed = true; changed;)
    {
        WorkgroupUniformLoadContext context;
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst); func && func->isDefinition())
                context.getFunctionInfo(func);
        }
        changed = context.hasReplacedLoads;
    }
}

} // namespace Slang
