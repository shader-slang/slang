#include "slang-ir-wgsl-legalize.h"

#include "slang-ir-insts.h"
#include "slang-ir-legalize-binary-operator.h"
#include "slang-ir-legalize-global-values.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Returns true if the `argIndex`-th parameter of `call`'s callee is passed with
// copy-in/out (`out`/`inout`) semantics. Such an argument may be bridged through a
// `function`-space temporary, whereas a `ref`/borrow parameter aliases the caller's
// object (e.g. `workgroupUniformLoad` on groupshared memory) and must receive the
// real pointer. Returns false when the callee signature is unavailable, so an
// argument of unknown mode is left untouched.
static bool paramHasCopyInOutSemantics(IRCall* call, UInt argIndex)
{
    auto funcType = as<IRFuncType>(call->getCallee()->getFullType());
    if (!funcType || argIndex >= funcType->getParamCount())
        return false;
    auto paramType = funcType->getParamType(argIndex);
    return as<IROutParamType>(paramType) || as<IRBorrowInOutParamType>(paramType);
}

static void legalizeCall(IRCall* call)
{
    // WGSL does not allow forming a pointer to a sub part of a composite value.
    // For example, if we have
    // ```
    // struct S { float x; float y; };
    // void foo(inout float v) { v = 1.0f; }
    // void main() { S s; foo(s.x); }
    // ```
    // The call to `foo(s.x)` is illegal in WGSL because `s.x` is a sub part of `s`.
    // And trying to form `&s.x` in WGSL is illegal.
    // To work around this, we will create a local variable to hold the sub part of
    // the composite value.
    // And then pass the local variable to the function.
    // After the call, we will write back the local variable to the sub part of the
    // composite value.
    //
    IRBuilder builder(call);
    builder.setInsertBefore(call);
    struct WritebackPair
    {
        IRInst* dest;
        IRInst* value;
    };
    ShortList<WritebackPair> pendingWritebacks;

    for (UInt i = 0; i < call->getArgCount(); i++)
    {
        auto arg = call->getArg(i);
        auto ptrType = as<IRPtrTypeBase>(arg->getDataType());
        if (!ptrType)
            continue;

        auto argOp = arg->getOp();

        // A by-ref `Param` and a block-local `Var` are already in the `function` address
        // space, so they can be passed to a `ptr<function, T>` parameter directly.
        if (argOp == kIROp_Param)
            continue;
        if (argOp == kIROp_Var && as<IRBlock>(arg->getParent()))
            continue;

        // A module-scope object (`GlobalVar`/`GlobalParam`, or a `Var` outside a block)
        // lives in a module-scope space such as `private`. Passing it directly to a
        // `ptr<function, T>` (`out`/`inout`) parameter is the address-space mismatch WGSL
        // rejects (#12173), so bridge it through the copy-in/out temp below. A `ref`/borrow
        // parameter instead aliases the caller's real object (e.g. `workgroupUniformLoad`
        // on groupshared memory), so it must keep receiving that object directly.
        if (argOp == kIROp_Var || argOp == kIROp_GlobalVar || argOp == kIROp_GlobalParam)
        {
            if (!paramHasCopyInOutSemantics(call, i))
                continue;
        }
        // Otherwise the argument is a pointer to a sub-part of a composite value (e.g.
        // `s.x`), which WGSL cannot address directly — bridge it unconditionally, as before.

        // Create a local variable to hold the input argument.
        auto var = builder.emitVar(ptrType->getValueType(), AddressSpace::Function);

        // Store the input argument into the local variable.
        builder.emitStore(var, builder.emitLoad(arg));
        builder.replaceOperand(call->getArgs() + i, var);
        pendingWritebacks.add({arg, var});
    }

    // Perform writebacks after the call.
    builder.setInsertAfter(call);
    for (auto& pair : pendingWritebacks)
    {
        builder.emitStore(pair.dest, builder.emitLoad(pair.value));
    }
}

static void legalizeFunc(IRFunc* func)
{
    // Insert casts to convert integer return types
    auto funcReturnType = func->getResultType();
    if (isIntegralType(funcReturnType))
    {
        for (auto block : func->getBlocks())
        {
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                auto returnedValue = returnInst->getOperand(0);
                auto returnedValueType = returnedValue->getDataType();
                if (isIntegralType(returnedValueType))
                {
                    IRBuilder builder(returnInst);
                    builder.setInsertBefore(returnInst);
                    auto newOp = builder.emitCast(funcReturnType, returnedValue);
                    builder.replaceOperand(returnInst->getOperands(), newOp);
                }
            }
        }
    }
}

static void legalizeSwitch(IRSwitch* switchInst)
{
    // WGSL Requires all switch statements to contain a default case.
    // If the switch statement does not contain a default case, we will add one.
    if (switchInst->getDefaultLabel() != switchInst->getBreakLabel())
        return;
    IRBuilder builder(switchInst);
    auto defaultBlock = builder.createBlock();
    builder.setInsertInto(defaultBlock);
    builder.emitBranch(switchInst->getBreakLabel());
    defaultBlock->insertBefore(switchInst->getBreakLabel());
    List<IRInst*> cases;
    for (UInt i = 0; i < switchInst->getCaseCount(); i++)
    {
        cases.add(switchInst->getCaseValue(i));
        cases.add(switchInst->getCaseLabel(i));
    }
    builder.setInsertBefore(switchInst);
    auto newSwitch = builder.emitSwitch(
        switchInst->getCondition(),
        switchInst->getBreakLabel(),
        defaultBlock,
        (UInt)cases.getCount(),
        cases.getBuffer());
    switchInst->transferDecorationsTo(newSwitch);
    switchInst->removeAndDeallocate();
}

static void processInst(IRInst* inst, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_Call:
        legalizeCall(static_cast<IRCall*>(inst));
        break;

    case kIROp_Switch:
        legalizeSwitch(as<IRSwitch>(inst));
        break;

    // For all binary operators, make sure both side of the operator have the same type
    // (vector-ness and matrix-ness).
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_FRem:
    case kIROp_IRem:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_Geq:
    case kIROp_Leq:
        legalizeBinaryOp(inst, sink, targetProgram);
        break;

    case kIROp_Func:
        legalizeFunc(static_cast<IRFunc*>(inst));
        [[fallthrough]];
    default:
        for (auto child : inst->getModifiableChildren())
        {
            processInst(child, targetProgram, sink);
        }
    }
}

struct GlobalInstInliningContext : public GlobalInstInliningContextGeneric
{
    bool isLegalGlobalInstForTarget(IRInst* inst) override
    {
        switch (inst->getOp())
        {
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_MakeVectorFromScalar:
            return true;
        default:
            break;
        }
        // The global instructions that are generically considered legal are fine for
        // WGSL.
        return false;
    }

    bool isInlinableGlobalInstForTarget(IRInst* /* inst */) override
    {
        // The global instructions that are generically considered inlineable are fine
        // for WGSL.
        return false;
    }

    bool shouldBeInlinedForTarget(IRInst* /* user */) override
    {
        // WGSL doesn't do any extra inlining beyond what is generically done by default.
        return false;
    }

    IRInst* getOutsideASM(IRInst* beforeInst) override
    {
        // Not needed for WGSL, check e.g. the SPIR-V case to see why this is used.
        return beforeInst;
    }
};

void legalizeIRForWGSL(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    List<EntryPointInfo> entryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        IRFunc* const func{as<IRFunc>(inst)};
        if (!func)
            continue;
        IREntryPointDecoration* const entryPointDecor =
            func->findDecoration<IREntryPointDecoration>();
        if (!entryPointDecor)
            continue;
        EntryPointInfo info;
        info.entryPointDecor = entryPointDecor;
        info.entryPointFunc = func;
        entryPoints.add(info);
    }

    legalizeEntryPointVaryingParamsForWGSL(module, sink, entryPoints);

    // Go through every instruction in the module and legalize them as needed.
    processInst(module->getModuleInst(), targetProgram, sink);

    // Some global insts are illegal, e.g. function calls.
    // We need to inline and remove those.
    GlobalInstInliningContext().inlineGlobalValuesAndRemoveIfUnused(module);
}

// WGSL's uniformity analysis rejects a workgroup barrier unless it can prove that every
// invocation of the workgroup reaches it, and it treats every read of `var<workgroup>` memory as
// possibly non-uniform unless the read is a `workgroupUniformLoad`. For example, a loop bounded by
// a groupshared count fails even when one invocation writes the count before a barrier:
//
//     for (uint i = 0; i < gCount; ++i) { ...; GroupMemoryBarrierWithGroupSync(); }
//
// HLSL already requires a group-sync barrier to be reached by every thread of the group, so a
// groupshared value that decides whether a barrier runs has to be the same for the whole group.
// This pass finds such reads (through arithmetic, comparisons, SSA phis, local variables and
// call arguments) and emits them as `workgroupUniformLoad`, which states that uniformity to WGSL.
//
// `workgroupUniformLoad` is itself a workgroup barrier followed by the load, so it must also be
// reached in uniform control flow. The pass only rewrites reads that decide whether a barrier
// runs, and it requires the same of the control flow around each rewritten read, so every
// rewritten read is in control flow that the program already requires to be uniform. Reads of
// groupshared memory elsewhere are left alone.
struct WorkgroupUniformLoadContext
{
    CapabilitySet targetCaps;

    // Functions whose body (or a callee's body) contains a workgroup barrier.
    Dictionary<IRInst*, bool> barrierCallees;

    // For each block, the blocks whose branch condition decides whether the block runs.
    Dictionary<IRBlock*, List<IRBlock*>> controlDependences;
    HashSet<IRGlobalValueWithCode*> codeWithControlDependences;

    // Values that must be uniform, and blocks that must be reached in uniform control flow.
    List<IRInst*> workList;
    HashSet<IRInst*> required;

    List<IRLoad*> loadsToReplace;

    static IRInst* getBranchCondition(IRBlock* block)
    {
        auto terminator = block->getTerminator();
        if (auto ifElse = as<IRIfElse>(terminator))
            return ifElse->getCondition();
        if (auto condBranch = as<IRConditionalBranch>(terminator))
            return condBranch->getCondition();
        if (auto switchInst = as<IRSwitch>(terminator))
            return switchInst->getCondition();
        return nullptr;
    }

    bool isBarrierCallee(IRInst* callee)
    {
        if (auto result = barrierCallees.tryGetValue(callee))
            return *result;

        // Treat a function as barrier-free while it is being visited, so that recursion ends.
        barrierCallees[callee] = false;

        bool hasBarrier = false;
        UnownedStringSlice definition;
        IRInst* definitionInst = nullptr;
        if (findTargetIntrinsicDefinition(callee, targetCaps, definition, definitionInst))
        {
            hasBarrier = definition.indexOf(toSlice("workgroupBarrier")) >= 0 ||
                         definition.indexOf(toSlice("storageBarrier")) >= 0 ||
                         definition.indexOf(toSlice("textureBarrier")) >= 0;
        }
        else if (auto code = as<IRGlobalValueWithCode>(callee))
        {
            for (auto block : code->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto call = as<IRCall>(inst);
                    if (call && isBarrierCallee(call->getCallee()))
                    {
                        hasBarrier = true;
                        break;
                    }
                }
                if (hasBarrier)
                    break;
            }
        }
        barrierCallees[callee] = hasBarrier;
        return hasBarrier;
    }

    // Compute control dependences for `code` from its post-dominator tree: a block depends on
    // a branch when it post-dominates one of the branch's successors but not the branch itself.
    void computeControlDependences(IRGlobalValueWithCode* code)
    {
        if (!codeWithControlDependences.add(code))
            return;

        List<IRBlock*> blocks;
        Dictionary<IRBlock*, Index> blockIndices;
        for (auto block : code->getBlocks())
        {
            blockIndices[block] = blocks.getCount();
            blocks.add(block);
        }

        // Blocks that leave the function branch to a virtual exit node.
        const Index exitNode = blocks.getCount();
        const Index nodeCount = exitNode + 1;
        List<List<Index>> successors;
        List<List<Index>> predecessors;
        successors.setCount(nodeCount);
        predecessors.setCount(nodeCount);
        for (Index i = 0; i < exitNode; i++)
        {
            for (auto successor : blocks[i]->getSuccessors())
            {
                Index successorIndex = 0;
                if (!blockIndices.tryGetValue(successor, successorIndex))
                    continue;
                successors[i].add(successorIndex);
                predecessors[successorIndex].add(i);
            }
            if (successors[i].getCount() == 0)
            {
                successors[i].add(exitNode);
                predecessors[exitNode].add(i);
            }
        }

        // Post-order of the reversed CFG, starting at the exit node.
        List<Index> postOrder;
        List<Index> postOrderNumbers;
        postOrderNumbers.setCount(nodeCount);
        for (auto& number : postOrderNumbers)
            number = -1;
        {
            List<bool> visited;
            visited.setCount(nodeCount);
            for (auto& v : visited)
                v = false;
            struct StackEntry
            {
                Index node;
                Index nextEdge;
            };
            List<StackEntry> stack;
            stack.add({exitNode, 0});
            visited[exitNode] = true;
            while (stack.getCount())
            {
                auto& top = stack.getLast();
                if (top.nextEdge < predecessors[top.node].getCount())
                {
                    Index next = predecessors[top.node][top.nextEdge++];
                    if (!visited[next])
                    {
                        visited[next] = true;
                        stack.add({next, 0});
                    }
                    continue;
                }
                postOrderNumbers[top.node] = postOrder.getCount();
                postOrder.add(top.node);
                stack.removeLast();
            }
        }

        // Immediate post-dominators (Cooper, Harvey and Kennedy). Blocks that cannot reach an
        // exit get no post-dominator, and so no control dependences.
        List<Index> postDominators;
        postDominators.setCount(nodeCount);
        for (auto& node : postDominators)
            node = -1;
        postDominators[exitNode] = exitNode;
        auto intersect = [&](Index a, Index b)
        {
            while (a != b)
            {
                while (postOrderNumbers[a] < postOrderNumbers[b])
                    a = postDominators[a];
                while (postOrderNumbers[b] < postOrderNumbers[a])
                    b = postDominators[b];
            }
            return a;
        };
        for (bool changed = true; changed;)
        {
            changed = false;
            for (Index i = postOrder.getCount() - 1; i >= 0; i--)
            {
                Index node = postOrder[i];
                if (node == exitNode)
                    continue;
                Index newPostDominator = -1;
                for (auto successor : successors[node])
                {
                    if (postDominators[successor] == -1)
                        continue;
                    newPostDominator =
                        newPostDominator == -1 ? successor : intersect(successor, newPostDominator);
                }
                if (newPostDominator != postDominators[node])
                {
                    postDominators[node] = newPostDominator;
                    changed = true;
                }
            }
        }

        for (Index branchNode = 0; branchNode < exitNode; branchNode++)
        {
            if (!getBranchCondition(blocks[branchNode]))
                continue;
            Index stopNode = postDominators[branchNode];
            if (stopNode == -1)
                continue;
            for (auto successor : successors[branchNode])
            {
                for (Index node = successor; node != stopNode && node != exitNode && node != -1;
                     node = postDominators[node])
                {
                    auto& dependences = controlDependences[blocks[node]];
                    if (!dependences.contains(blocks[branchNode]))
                        dependences.add(blocks[branchNode]);
                }
            }
        }
    }

    void require(IRInst* inst)
    {
        if (!inst)
            return;
        // Module-scope values (constants, uniform parameters) need nothing from us.
        if (!as<IRBlock>(inst) && !as<IRBlock>(inst->getParent()))
            return;
        if (required.add(inst))
            workList.add(inst);
    }

    template<typename F>
    static void forEachCallOf(IRInst* callee, const F& f)
    {
        for (auto use = callee->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            if (call && call->getCallee() == callee)
                f(call);
        }
    }

    void requireUniformControlFlow(IRBlock* block)
    {
        auto code = as<IRGlobalValueWithCode>(block->getParent());
        if (!code)
            return;
        computeControlDependences(code);
        if (auto dependences = controlDependences.tryGetValue(block))
        {
            for (auto branchBlock : *dependences)
            {
                require(getBranchCondition(branchBlock));
                require(branchBlock);
            }
        }
        // The whole function runs in uniform control flow only if every call to it does.
        if (block == code->getFirstBlock())
        {
            forEachCallOf(code, [&](IRCall* call) { require(call->getParent()); });
        }
    }

    void requireUniformLoad(IRLoad* load)
    {
        IRInst* root = load->getPtr();
        for (;;)
        {
            if (auto elementPtr = as<IRGetElementPtr>(root))
            {
                require(elementPtr->getIndex());
                root = elementPtr->getBase();
            }
            else if (auto fieldAddress = as<IRFieldAddress>(root))
            {
                root = fieldAddress->getBase();
            }
            else
            {
                break;
            }
        }

        auto ptrType = as<IRPtrTypeBase>(load->getPtr()->getDataType());
        bool isGroupShared = as<IRGroupSharedRate>(root->getRate()) ||
                             (ptrType && ptrType->getAddressSpace() == AddressSpace::GroupShared);
        if (as<IRGlobalVar>(root) && isGroupShared)
        {
            loadsToReplace.add(load);
            require(load->getParent());
            return;
        }

        // A local variable is uniform when every value stored to it is, and every store
        // happens in uniform control flow. That includes stores through an element or field
        // address of the variable, which also need the element indices on their path to be
        // uniform. A load of one element is covered the same way, conservatively: a store to
        // any part of the variable counts.
        if (as<IRVar>(root))
        {
            List<IRInst*> addresses;
            addresses.add(root);
            for (Index i = 0; i < addresses.getCount(); i++)
            {
                auto address = addresses[i];
                for (auto use = address->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (auto store = as<IRStore>(user))
                    {
                        if (store->getPtr() != address)
                            continue;
                        require(store->getVal());
                        require(store->getParent());
                        for (auto path = address; path != root;)
                        {
                            if (auto elementPtr = as<IRGetElementPtr>(path))
                            {
                                require(elementPtr->getIndex());
                                path = elementPtr->getBase();
                            }
                            else
                            {
                                path = as<IRFieldAddress>(path)->getBase();
                            }
                        }
                    }
                    else if (auto elementPtr = as<IRGetElementPtr>(user))
                    {
                        if (elementPtr->getBase() == address)
                            addresses.add(elementPtr);
                    }
                    else if (auto fieldAddress = as<IRFieldAddress>(user))
                    {
                        if (fieldAddress->getBase() == address)
                            addresses.add(fieldAddress);
                    }
                }
            }
        }
    }

    // A call's result is uniform when every value the callee returns is, and so is the choice
    // of `return`. This follows the values inside the callee and maps each parameter of the
    // callee to this call's argument, so the other calls of the callee are not affected. Reads
    // of groupshared memory inside the callee go through `require` as usual: rewriting such a
    // read changes it for every call, so every call has to reach it in uniform control flow.
    // Returns false if the callee has no body to follow.
    bool requireUniformCallResult(IRCall* call)
    {
        auto callee = as<IRFunc>(call->getCallee());
        if (!callee || !callee->getFirstBlock())
            return false;
        UnownedStringSlice definition;
        IRInst* definitionInst = nullptr;
        if (findTargetIntrinsicDefinition(callee, targetCaps, definition, definitionInst))
            return false;

        computeControlDependences(callee);
        auto firstBlock = callee->getFirstBlock();

        HashSet<IRInst*> visited;
        List<IRInst*> values;
        List<IRBlock*> blocks;
        auto addValue = [&](IRInst* value)
        {
            if (value && visited.add(value))
                values.add(value);
        };
        auto addBlock = [&](IRBlock* block)
        {
            if (visited.add(block))
                blocks.add(block);
        };

        for (auto block : callee->getBlocks())
        {
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                addValue(returnInst->getVal());
                addBlock(block);
            }
        }

        Index blockIndex = 0;
        Index valueIndex = 0;
        while (blockIndex < blocks.getCount() || valueIndex < values.getCount())
        {
            // The branches that decide which `return` runs must be uniform. Unlike
            // `requireUniformControlFlow`, this stays inside the callee.
            if (blockIndex < blocks.getCount())
            {
                auto block = blocks[blockIndex++];
                if (auto dependences = controlDependences.tryGetValue(block))
                {
                    for (auto branchBlock : *dependences)
                    {
                        addValue(getBranchCondition(branchBlock));
                        addBlock(branchBlock);
                    }
                }
                continue;
            }

            auto value = values[valueIndex++];
            if (auto param = as<IRParam>(value))
            {
                auto block = as<IRBlock>(param->getParent());
                auto paramIndex = (UInt)getParamIndexInBlock(param);
                if (block == firstBlock)
                {
                    if (paramIndex < call->getArgCount())
                        require(call->getArg(paramIndex));
                    continue;
                }
                for (auto predecessor : block->getPredecessors())
                {
                    if (auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator()))
                    {
                        if (paramIndex < branch->getArgCount())
                            addValue(branch->getArg(paramIndex));
                    }
                    addBlock(predecessor);
                }
                continue;
            }

            // Module-scope values (constants, uniform parameters) need nothing from us.
            if (!as<IRBlock>(value->getParent()))
                continue;

            if (as<IRLoad>(value) || as<IRCall>(value))
            {
                require(value);
                continue;
            }

            if (value->mightHaveSideEffects())
                continue;
            for (UInt i = 0; i < value->getOperandCount(); i++)
            {
                auto operand = value->getOperand(i);
                if (!as<IRBlock>(operand))
                    addValue(operand);
            }
        }
        return true;
    }

    void requireUniformValue(IRInst* inst)
    {
        if (auto param = as<IRParam>(inst))
        {
            auto block = as<IRBlock>(param->getParent());
            auto paramIndex = (UInt)getParamIndexInBlock(param);
            auto code = as<IRGlobalValueWithCode>(block->getParent());
            if (code && block == code->getFirstBlock())
            {
                // A function parameter: every argument passed to it must be uniform.
                forEachCallOf(
                    code,
                    [&](IRCall* call)
                    {
                        if (paramIndex < call->getArgCount())
                            require(call->getArg(paramIndex));
                    });
                return;
            }
            // A phi: every incoming value must be uniform, and so must the choice of edge.
            for (auto predecessor : block->getPredecessors())
            {
                if (auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator()))
                {
                    if (paramIndex < branch->getArgCount())
                        require(branch->getArg(paramIndex));
                }
                require(predecessor);
            }
            return;
        }

        if (auto load = as<IRLoad>(inst))
        {
            requireUniformLoad(load);
            return;
        }

        if (auto call = as<IRCall>(inst))
        {
            if (requireUniformCallResult(call))
                return;
        }

        // Anything else is uniform when it is a pure function of uniform operands.
        if (inst->mightHaveSideEffects())
            return;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            if (!as<IRBlock>(operand))
                require(operand);
        }
    }

    void processModule(IRModule* module)
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(globalInst);
            if (!func)
                continue;
            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto call = as<IRCall>(inst);
                    if (call && isBarrierCallee(call->getCallee()))
                    {
                        require(block);
                        break;
                    }
                }
            }
        }

        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto inst = workList[i];
            if (auto block = as<IRBlock>(inst))
                requireUniformControlFlow(block);
            else
                requireUniformValue(inst);
        }

        for (auto load : loadsToReplace)
        {
            IRBuilder builder(load);
            builder.setInsertBefore(load);
            IRInst* ptr = load->getPtr();
            auto uniformLoad =
                builder.emitIntrinsicInst(load->getFullType(), kIROp_WorkgroupUniformLoad, 1, &ptr);
            load->transferDecorationsTo(uniformLoad);
            load->replaceUsesWith(uniformLoad);
            load->removeAndDeallocate();
        }
    }
};

void insertWorkgroupUniformLoadsForWGSL(IRModule* module, CapabilitySet const& targetCaps)
{
    WorkgroupUniformLoadContext context;
    context.targetCaps = targetCaps;
    context.processModule(module);
}

struct WGSLAddressSpaceAssigner : InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst* inst, AddressSpace& outAddressSpace) override
    {
        switch (inst->getOp())
        {
        case kIROp_Var:
            if (as<IRBlock>(inst->getParent()))
                outAddressSpace = AddressSpace::Function;
            else
                outAddressSpace = AddressSpace::ThreadLocal;
            return true;
        case kIROp_RWStructuredBufferGetElementPtr:
            outAddressSpace = AddressSpace::Global;
            return true;
        case kIROp_Load:
            {
                auto addrSpace = getAddressSpaceFromVarType(inst->getDataType());
                if (addrSpace != AddressSpace::Generic)
                {
                    outAddressSpace = addrSpace;
                    return true;
                }
            }
            return false;
        default:
            return false;
        }
    }

    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type) override
    {
        if (as<IRUniformParameterGroupType>(type))
        {
            return AddressSpace::Uniform;
        }
        if (as<IRByteAddressBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRHLSLStructuredBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRGLSLShaderStorageBufferType>(type))
        {
            return AddressSpace::Global;
        }
        if (auto ptrType = as<IRPtrTypeBase>(type))
        {
            if (ptrType->hasAddressSpace())
                return ptrType->getAddressSpace();
            return AddressSpace::Generic;
        }
        return AddressSpace::Generic;
    }

    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst) override
    {
        if (as<IRGroupSharedRate>(inst->getRate()))
            return AddressSpace::GroupShared;
        switch (inst->getOp())
        {
        case kIROp_RWStructuredBufferGetElementPtr:
            return AddressSpace::Global;
        case kIROp_Var:
            if (as<IRBlock>(inst->getParent()))
                return AddressSpace::Function;
            else
                return AddressSpace::ThreadLocal;
            break;
        default:
            break;
        }
        auto type = unwrapAttributedType(inst->getDataType());
        if (!type)
            return AddressSpace::Generic;
        return getAddressSpaceFromVarType(type);
    }
};

void specializeAddressSpaceForWGSL(IRModule* module)
{
    WGSLAddressSpaceAssigner wgslAddressSpaceAssigner;
    specializeAddressSpace(module, &wgslAddressSpaceAssigner);
}

} // namespace Slang
