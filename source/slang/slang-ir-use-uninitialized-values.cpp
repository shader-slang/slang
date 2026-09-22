#include "slang-ir-use-uninitialized-values.h"

#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static bool isUninitializedValue(IRInst* inst)
{
    // Also consider var since it does not
    // automatically mean it will be initialized
    // (at least not as the user may have intended)
    return (as<IRUndefined>(inst) || (inst->m_op == kIROp_Var));
}

static bool isUnmodifying(IRFunc* func)
{
    auto intr = func->findDecoration<IRIntrinsicOpDecoration>();
    return (intr && intr->getIntrinsicOp() == kIROp_Unmodified);
}

enum ParameterCheckType
{
    Never,  // Parameter does NOT to be checked for uninitialization (e.g. is `in` or special type)
    AsOut,  // Parameter DOES need to be checked for usage before initializations
    AsInOut // Parameter DOES need to be checked to see if it is ever written to
};

static ParameterCheckType isPotentiallyUnintended(IRParam* param, Stage stage, int index)
{
    IRType* type = param->getFullType();
    if (auto out = as<IROutParamType>(param->getFullType()))
    {
        // Don't check `out Vertices<T>` or `out Indices<T>` parameters
        // in mesh shaders.
        // TODO: we should find a better way to represent these mesh shader
        // parameters so they conform to the initialize before use convention.
        // For example, we can use a `OutputVetices` and `OutputIndices` type
        // to represent an output, like `OutputPatch` in domain shader.
        // For now, we just skip the check for these parameters.
        switch (out->getValueType()->getOp())
        {
        case kIROp_VerticesType:
        case kIROp_IndicesType:
        case kIROp_PrimitivesType:
            return Never;
        default:
            break;
        }

        return AsOut;
    }
    else if (auto inout = as<IRBorrowInOutParamType>(type))
    {
        // TODO: some way to check if the method
        // is actually used for autodiff
        if (as<IRDifferentialPairType>(inout->getValueType()))
            return Never;

        switch (stage)
        {
        case Stage::AnyHit:
        case Stage::ClosestHit:
            // In HLSL the payload is required to be `inout`
            return (index == 0) ? Never : AsInOut;
        case Stage::Geometry:
            // Second parameter is the triangle stream
            return (index == 1) ? Never : AsInOut;
        default:
            break;
        }

        return AsInOut;
    }

    return Never;
}

static bool isAliasable(IRInst* inst)
{
    switch (inst->getOp())
    {
    // These instructions generate (implicit) references to inst
    case kIROp_FieldExtract:
    case kIROp_FieldAddress:
    case kIROp_GetElement:
    case kIROp_GetElementPtr:
    case kIROp_InOutImplicitCast:
        return true;
    default:
        break;
    }

    return false;
}

// The `upper` field contains the struct that the type is
// is contained in. It is used to check for empty structs.
static bool canIgnoreType(IRType* type, IRType* upper)
{
    // In case specialization returns a function instead
    if (!type)
        return true;

    if (as<IRVoidType>(type))
        return true;

    // For structs, ignore if its empty
    if (auto str = as<IRStructType>(type))
    {
        int count = 0;
        for (auto field : str->getFields())
        {
            IRType* ftype = field->getFieldType();
            count += !canIgnoreType(ftype, type);
        }

        return (count == 0);
    }

    // Nothing to initialize for a pure interface
    if (as<IRInterfaceType>(type))
        return true;

    // We don't know what type it will be yet.
    if (as<IRParam>(type))
        return true;

    // For pointers, check the value type (primarily for globals)
    if (auto ptr = as<IRPtrType>(type))
    {
        // Avoid the recursive step if its a
        // recursive structure like a linked list
        IRType* ptype = ptr->getValueType();
        if (auto resolvedType = as<IRType>(getResolvedInstForDecorations(ptype)))
            ptype = resolvedType;
        return (ptype != upper) && canIgnoreType(ptype, upper);
    }

    // In the case of specializations, check returned type
    if (auto spec = as<IRSpecialize>(type))
    {
        IRInst* inner = getResolvedInstForDecorations(spec);
        IRType* innerType = (IRType*)(inner);
        return canIgnoreType(innerType, upper);
    }

    return false;
}

// If `argUse` is an *argument* operand of an unconditional branch or loop (i.e. a phi
// edge value, not one of the branch's target/break/continue block operands), return the
// target block parameter that receives that argument. Otherwise return null.
//
// Branch arguments map positionally to the target block's parameters, so the value passed
// as the i-th argument becomes the i-th block parameter (the SSA phi) along this edge.
static IRParam* getBranchArgPhiParam(IRUse* argUse)
{
    auto branch = as<IRUnconditionalBranch>(argUse->getUser());
    if (!branch)
        return nullptr;

    // `getArgs()` points into the branch's contiguous operand storage and, by
    // construction, excludes the non-argument operand slots (the target block, plus the
    // break/continue blocks for an `IRLoop`). So the argument index is just the offset of
    // `argUse` within that range; anything outside it (e.g. the target/break/continue use)
    // is not a phi argument.
    auto args = branch->getArgs();
    UInt argCount = branch->getArgCount();
    UInt argIndex = UInt(argUse - args);
    if (argIndex >= argCount)
        return nullptr;

    // The target block parameter at the same index receives this argument. In well-formed
    // IR the target always has at least `argCount` parameters, so `getParamAt` resolves it
    // (and asserts otherwise).
    return getParamAt(branch->getTargetBlock(), argIndex);
}

// Collect all instructions that alias `inst` for the purpose of the uninitialized-use
// analysis. This follows two kinds of aliasing:
//
//  - Address aliasing: instructions like `getElementPtr`/`fieldAddress` produce a
//    derived reference to the same storage (see `isAliasable`).
//  - SSA value flow through phis: when `inst` (a value) is passed as a branch/loop
//    argument, the receiving block parameter is the same value along that edge, so its
//    uses are uses of `inst`. This is what lets a loop-carried use be seen: an
//    uninitialized value passed as a loop's initial phi argument flows to the loop-header
//    parameter, whose first-iteration read is a genuine use of the uninitialized value.
//
// A `visited` set guards against the cycles that phi following introduces (a loop-header
// phi is reachable from its own back-edge argument).
static void getAliasableInstructionsRec(
    IRInst* inst,
    HashSet<IRInst*>& visited,
    List<IRInst*>& addresses)
{
    if (!visited.add(inst))
        return;

    addresses.add(inst);
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();

        // Type queries do not observe the runtime value carried by this operand.
        if (doesInstOnlyDependOnOperandTypes(user))
            continue;

        if (isAliasable(user))
        {
            getAliasableInstructionsRec(user, visited, addresses);
            continue;
        }

        // Follow SSA value flow through a phi: an argument passed to a branch/loop
        // becomes the corresponding block parameter, which is the same value along
        // this edge.
        if (auto phiParam = getBranchArgPhiParam(use))
            getAliasableInstructionsRec(phiParam, visited, addresses);
    }
}

static List<IRInst*> getAliasableInstructions(IRInst* inst, HashSet<IRInst*>& aliasSet)
{
    List<IRInst*> addresses;
    getAliasableInstructionsRec(inst, aliasSet, addresses);
    return addresses;
}

static List<IRInst*> getAliasableInstructions(IRInst* inst)
{
    HashSet<IRInst*> aliasSet;
    return getAliasableInstructions(inst, aliasSet);
}

// We determine whether `inst` transitively depends on a value in `aliasSet`. A phi argument that
// depends on the tracked value merely carries that value around a loop; an independent argument is
// a genuine definition reaching the phi. For example, `add(total1, a[i])` depends on the loop-
// header alias `total1`, while `load(candidateProceduralAttrs)` does not. We use `seen` to
// terminate the operand walk when phis form a cycle.
static bool dependsOnAlias(IRInst* inst, const HashSet<IRInst*>& aliasSet, HashSet<IRInst*>& seen)
{
    if (aliasSet.contains(inst))
        return true;
    if (!seen.add(inst))
        return false;
    // We stop at block boundaries because a block parameter's phi arguments arrive through its
    // predecessors rather than through `getOperand`. Membership in `aliasSet` above handles an
    // alias parameter; every other parameter is an independent definition for this operand walk.
    if (as<IRParam>(inst))
        return false;
    for (UInt i = 0, n = inst->getOperandCount(); i < n; i++)
    {
        auto operand = inst->getOperand(i);
        if (operand && dependsOnAlias(operand, aliasSet, seen))
            return true;
    }
    return false;
}

// We treat a defined value entering an alias phi as a write on that predecessor edge. Without that
// write, a later read would appear to have no initialization even when a defined value reaches the
// phi along one path.
//
// This is what keeps the analysis honest once phi following is enabled. Consider a value
// that is assigned only on some paths and then read after a loop:
//
//     MyAttrs attrs;                       // uninitialized
//     for (;;) { ...; if (cond) attrs = computed; ... }
//     use(attrs);                          // reads the loop-header phi
//
// The loop-header phi merges the undefined pre-loop value with `computed`. No IR `store` exists
// for that SSA value, so we record the predecessor edge as a possible write and as a write this
// checker treats as definite. A purely loop-carried accumulator (`total += a[i]`) has no defined
// incoming value: its only non-undefined phi argument, `add(total1, a[i])`, depends on the phi
// itself. `dependsOnAlias` therefore rejects that argument, and we continue to diagnose the
// accumulator read.
static void collectPhiMergeWrites(
    const HashSet<IRInst*>& aliasSet,
    const List<IRInst*>& aliases,
    List<IRInst*>& writes)
{
    for (auto alias : aliases)
    {
        auto param = as<IRParam>(alias);
        if (!param)
            continue;
        auto block = as<IRBlock>(param->getParent());
        if (!block)
            continue;
        Index paramIndex = getParamIndexInBlock(param);
        if (paramIndex < 0)
            continue;

        for (auto pred : block->getPredecessors())
        {
            auto branch = as<IRUnconditionalBranch>(pred->getTerminator());
            if (!branch || UInt(paramIndex) >= branch->getArgCount())
                continue;
            auto arg = branch->getArg(UInt(paramIndex));
            // An argument defines the tracked value only when it does not derive from that value.
            HashSet<IRInst*> seen;
            if (dependsOnAlias(arg, aliasSet, seen))
                continue;
            // We record the write at the predecessor edge rather than at the argument's definition,
            // because the tracked value becomes initialized only on this incoming edge. The value
            // `arg` may have been defined earlier (for example, `float y = 1; if (cond) x = y;`).
            // Treating that definition as the write would incorrectly initialize the unassigned
            // edge and suppress the legitimate "may be uninitialized" diagnostic.
            writes.add(branch);
        }
    }
}

enum InstructionUsageType
{
    None,        // Instruction neither stores nor loads from the source (e.g. type-only queries)
    Store,       // Instruction acts as a write to the source
    StoreParent, // Instruction's parent acts as a write to the source
    Load         // Instruction acts as a load from the source
};

// We normally classify a call argument from the corresponding parameter direction: `out`, `inout`,
// and `ref` arguments are writes, while every other resolved argument is a read. Calls through
// `IRTranslateBase` are an IR-internal write that has no ordinary parameter signature, so we handle
// them directly. When the signature cannot be resolved, we leave the use unclassified rather than
// inventing an effect. `collectInstructionByUsage` can replace this baseline for an exact `IRUse`
// when a transformation supplies a more precise effect.
static InstructionUsageType getCallUsageType(IRCall* call, IRUse* argumentUse)
{
    if (as<IRTranslateBase>(call->getCallee()))
        return Store;

    auto paramType = findCallArgumentParameterType(call, argumentUse);
    if (!paramType)
        return None;

    return (as<IROutParamType>(paramType) || as<IRBorrowInOutParamType>(paramType) ||
            as<IRRefParamType>(paramType))
               ? Store
               : Load;
}

// We infer the effect from the user's opcode and the tracked operand's role. We ignore type queries
// and alias propagation, use precise rules for known opcodes, and preserve the module-wide
// checker's fallback for unknown users: pointer-producing instructions count as writes and all
// others as reads.
static InstructionUsageType getInstructionUsageType(IRUse* use, IRInst* inst)
{
    auto user = use->getUser();

    // Type-only instructions do not observe the runtime value of their operands.
    if (doesInstOnlyDependOnOperandTypes(user))
        return None;

    // Alias-producing instructions are traversed separately and do not themselves read or write the
    // tracked value.
    if (isAliasable(user))
        return None;

    switch (user->getOp())
    {
    case kIROp_Loop:
    case kIROp_UnconditionalBranch:
        // TODO: Ignore branches for now
        return None;

    // Debug instructions describe the value but do not observe or modify it at runtime.
    case kIROp_DebugValue:
    case kIROp_DebugVar:
    case kIROp_DebugLine:
    case kIROp_DebugScope:
    case kIROp_DebugInlinedAt:
        return None;

    case kIROp_Call:
        return getCallUsageType(as<IRCall>(user), use);

    case kIROp_Store:
    case kIROp_AtomicStore:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
        // Each of these writes to its destination pointer (operand 0) but
        // *reads* the value/source being stored (operand 1). When the tracked
        // instruction is that value -- rather than the destination -- the store
        // reads it, so classify it as a `Load`. This lets a direct copy of an
        // uninitialized value (e.g. `x = uninit;` or `v.x = uninit;`) be
        // detected just like feeding it to an expression (`x = uninit + 1.0;`).
        //
        // A pointer-typed operand is excluded: a store whose value is an
        // address (e.g. a variable's own address in `self.self = &self;`, which
        // lowers to `store(getFieldAddr(self), self)` with the `self` pointer as
        // the value) stores that address without reading the pointed-to memory,
        // so it is not a use of the location. This applies the same
        // pointer-vs-value rule as the `default` case below, but on a different
        // subject: here it tests the stored value's type
        // (`inst->getDataType()`), whereas the `default` case tests the using
        // instruction's type (`user->getDataType()`).
        if (use == user->getOperandUse(1) && !as<IRPtrTypeBase>(inst->getDataType()))
            return Load;
        return Store;

    // A SPIR-V asm block is opaque -- its operands have no fixed read/write
    // role -- so conservatively treat any use by one as a store (a write).
    case kIROp_SPIRVAsm:
        return Store;

    case kIROp_SPIRVAsmOperandInst:
        // For SPIRV asm instructions, need to check out the entire
        // block when doing reachability checks
        return StoreParent;

    case kIROp_MakeExistential:
    case kIROp_MakeExistentialWithRTTI:
        // For specializing generic structs
        return Store;

    case kIROp_ManagedPtrAttach:
        // We retain the legacy approximation that either address operand represents a write. The
        // generic classifier does not distinguish the destination from the attached native pointer;
        // a caller that needs operand-specific semantics must provide a use-specific effect.
        return Store;

    case kIROp_Unmodified:
        // This intrinsic explicitly satisfies an output write contract without changing the value.
        return Store;

    default:
        // For an unfamiliar instruction, we preserve the established conservative rule: a
        // pointer-producing user counts as a write and every other user counts as a read.
        if (as<IRPtrTypeBase>(user->getDataType()))
            return Store;
        return Load;
    }
}

// Generic assembly does not expose operand effects, so output-parameter checking treats the whole
// instruction as a possible write barrier.
static void collectGenericAsmPossibleWrites(List<IRInst*>& possibleWrites, IRBlock* block)
{
    for (auto inst = block->getFirstInst(); inst; inst = inst->next)
    {
        if (as<IRGenericAsm>(inst))
            possibleWrites.add(inst);
    }
}

// We separate each tracked variable's uses into the facts needed by the two analyses. The generic
// classifier predates use-specific effects and treats every inferred write as both possible and
// definite. That is an established approximation, not proof that every instruction writes the
// complete value on every path. Use-specific effects preserve the distinction for synthesized
// operations whose writes can be partial or conditional.
struct TrackedVariableUses
{
    List<IRInst*> reads;
    List<IRInst*> possibleWrites;
    List<IRInst*> writesTreatedAsDefinite;
};

// We record one use in the read/write sets shared by the reachability and definite-assignment
// analyses. The generic classifier adds each inferred write to both write sets to preserve the
// checker's historical behavior. When a transformation supplies a use-specific effect, we use it
// instead: an `inout` ABI parameter need not semantically read its incoming value, and the effect
// can distinguish a partial or conditional write from a definite whole-value write.
static void collectInstructionByUsage(
    TrackedVariableUses& result,
    IRUse* use,
    IRInst* inst,
    ConstArrayView<UninitializedVariableUseEffect> useEffects = {})
{
    auto user = use->getUser();
    for (auto const& effect : useEffects)
    {
        if (effect.use != use)
            continue;

        if (effect.readsValue)
            result.reads.add(user);
        if (effect.mayWriteValue || effect.definitelyWritesValue)
            result.possibleWrites.add(user);
        if (effect.definitelyWritesValue)
            result.writesTreatedAsDefinite.add(user);
        return;
    }

    InstructionUsageType usage = getInstructionUsageType(use, inst);
    switch (usage)
    {
    case Load:
        return result.reads.add(user);
    case Store:
        result.possibleWrites.add(user);
        result.writesTreatedAsDefinite.add(user);
        return;
    case StoreParent:
        result.possibleWrites.add(user->getParent());
        result.writesTreatedAsDefinite.add(user->getParent());
        return;
    }
}

// We remove reads that an earlier possible write can reach. Any read left has no possible
// initialization reaching it. When one instruction both reads and writes, its outgoing write cannot
// initialize its incoming read.
static void removeReadsReachedByPossibleWrite(
    ReachabilityContext& reachability,
    const List<IRInst*>& possibleWrites,
    List<IRInst*>& reads)
{
    for (auto write : possibleWrites)
    {
        for (Index i = 0; i < reads.getCount();)
        {
            if (write != reads[i] && reachability.isInstReachable(write, reads[i]))
                reads.fastRemoveAt(i);
            else
                i++;
        }
    }
}

// If `block` ends in an `ifElse` whose condition is one of `block`'s own parameters
// (a phi), and the value that parameter receives along the edge from `fromPred` is a
// boolean constant, then only one of the two branches is feasible when arriving from
// `fromPred`. Returns the block that is *not* taken (the infeasible successor), or
// null if both successors remain feasible.
//
// This captures the short-circuit `&&`/`||` lowering: the merge block of `a && b`
// carries a phi that is the literal `false` along the "a is false" edge, so a later
// branch on that phi cannot take its true-side from that edge. Without this, the
// flat CFG admits an infeasible store-free path through the merge, producing a false
// "possibly uninitialized" warning for patterns like `f(out x) && use(x)`.
static IRBlock* getInfeasibleBranchFromPredecessor(IRBlock* block, IRBlock* fromPred)
{
    auto ifElse = as<IRIfElse>(block->getTerminator());
    if (!ifElse)
        return nullptr;

    auto cond = ifElse->getCondition();
    auto condParam = as<IRParam>(cond);
    if (!condParam || condParam->getParent() != block)
        return nullptr;

    // Find the index of this parameter among the block's parameters.
    UInt paramIndex = 0;
    bool found = false;
    for (auto p : block->getParams())
    {
        if (p == condParam)
        {
            found = true;
            break;
        }
        paramIndex++;
    }
    if (!found)
        return nullptr;

    // Get the branch argument supplied for that parameter along the edge from
    // `fromPred`.
    auto branch = as<IRUnconditionalBranch>(fromPred->getTerminator());
    if (!branch || paramIndex >= branch->getArgCount())
        return nullptr;

    auto argVal = as<IRBoolLit>(branch->getArg(paramIndex));
    if (!argVal)
        return nullptr;

    // A constant condition makes the opposite branch infeasible from this edge.
    return argVal->getValue() ? ifElse->getFalseBlock() : ifElse->getTrueBlock();
}

// We record each `if (WaveIsFirstLane()) { ... }` guard once per function rather than once per
// tracked variable (see `WaveElectionContext` below).
//
// `trueBlock` is the guard's true-branch entry block (the "elected lane" region); `mergeBlock`
// is the `ifElse`'s reconvergence block (`IRIfElse::getAfterBlock()`). Together with
// `isEveryPathBlockedByDefiniteWrite`, these blocks let
// `removeReadsWithDefiniteWriteOnEveryPath` recognize the pattern reported in
// https://github.com/shader-slang/slang/issues/12545. A value written inside the guard and read via
// `WaveReadLaneFirst()` cannot be observed before that write: the intrinsic broadcasts from the
// first active lane, which is precisely the lane that entered `trueBlock`.
struct WaveElectionGuard
{
    IRBlock* trueBlock;
    IRBlock* mergeBlock;
};

// We collect every direct `if (WaveIsFirstLane())` guard in `func`.
//
// We deliberately do not unwrap `!` or other boolean operations around the known builtin.
// `!WaveIsFirstLane()` lowers to an explicit `not(...)` operand feeding the `ifElse`, rather than
// to swapped true and false blocks. Requiring a direct call therefore excludes
// `if (!WaveIsFirstLane()) { write }`, where the first active lane never executes the write and a
// later `WaveReadLaneFirst()` can still observe an uninitialized value.
static List<WaveElectionGuard> collectWaveElectionGuards(IRGlobalValueWithCode* func)
{
    List<WaveElectionGuard> guards;
    for (auto block : func->getBlocks())
    {
        auto ifElse = as<IRIfElse>(block->getTerminator());
        if (!ifElse)
            continue;
        auto call = as<IRCall>(ifElse->getCondition());
        if (!call)
            continue;
        if (getBuiltinFuncEnum(call->getCallee()) != KnownBuiltinDeclName::WaveIsFirstLane)
            continue;
        guards.add(WaveElectionGuard{ifElse->getTrueBlock(), ifElse->getAfterBlock()});
    }
    return guards;
}

// We keep the `WaveIsFirstLane()`/`WaveReadLaneFirst()` relaxation in a function-wide context,
// mirroring the lifetime of `ReachabilityContext` in `checkUninitializedValues`. Both `guards` and
// `dominatorTree` are independent of the tracked variable, so rebuilding them per variable would
// waste work. We build the dominator tree only when a guard exists, since most functions never
// need this relaxation.
struct WaveElectionContext
{
    List<WaveElectionGuard> guards;
    RefPtr<IRDominatorTree> dominatorTree;
};

static WaveElectionContext collectWaveElectionContext(IRGlobalValueWithCode* func)
{
    WaveElectionContext context;
    context.guards = collectWaveElectionGuards(func);
    if (context.guards.getCount() != 0)
        context.dominatorTree = computeDominatorTree(func);
    return context;
}

// We first ask whether `to` is reachable from `from` without treating definite writes as barriers.
// `isEveryPathBlockedByDefiniteWrite` uses this fact to distinguish a write on every path from a
// region that never reaches `to`. An early `return`, `break`, or `discard` is the latter and
// provides no initialization guarantee at `to`.
static bool isBlockReachableFrom(IRBlock* from, IRBlock* to)
{
    HashSet<IRBlock*> visited;
    List<IRBlock*> worklist;
    visited.add(from);
    worklist.add(from);
    while (worklist.getCount())
    {
        auto block = worklist.getLast();
        worklist.removeLast();
        if (block == to)
            return true;
        for (auto succ : block->getSuccessors())
        {
            if (visited.add(succ))
                worklist.add(succ);
        }
    }
    return false;
}

// We determine whether every control-flow path from `from` to `to` passes through a block with a
// write this checker treats as definite. This tells the wave relaxation whether the elected lane is
// guaranteed to execute a write before reaching the guard's merge block.
//
// We walk the sub-CFG rooted at `from` instead of asking whether one write dominates `to` in the
// whole function. The false branch of the surrounding `if` always reaches `to` without entering
// this region, so whole-function dominance could never establish the property. The regional walk
// also rejects a write nested inside another conditional:
//
//     if (WaveIsFirstLane())
//     {
//         if (rareCondition)
//             nBaseIndex = ...;   // This write is conditional within the elected lane.
//     }
//     uint nIndex = WaveReadLaneFirst(nBaseIndex) + ...;
//
// When `rareCondition` is false, the elected lane reaches the merge without the inner write, so we
// retain the diagnostic.
//
// A traversal that never reaches `to` is not sufficient by itself: the region may diverge through
// an early `return`, `break`, or `discard` instead of encountering a write. We therefore establish
// ordinary reachability first. Once `to` is known to be reachable, failure to reach it while writes
// act as barriers proves that every path crosses such a write.
static bool isEveryPathBlockedByDefiniteWrite(
    IRBlock* from,
    IRBlock* to,
    const HashSet<IRBlock*>& blocksWithDefiniteWrite)
{
    if (!isBlockReachableFrom(from, to))
        return false;

    HashSet<IRBlock*> visited;
    List<IRBlock*> worklist;
    visited.add(from);
    worklist.add(from);
    while (worklist.getCount())
    {
        auto block = worklist.getLast();
        worklist.removeLast();
        if (block == to)
            return false; // This path reached `to` without a preceding definite write.
        if (blocksWithDefiniteWrite.contains(block))
            continue; // The definite write prevents this path from remaining uninitialized.
        for (auto succ : block->getSuccessors())
        {
            if (visited.add(succ))
                worklist.add(succ);
        }
    }
    return true;
}

// We determine whether `readingInst`, which `getInstructionUsageType` has already classified as a
// read of the tracked value, is actually a `WaveReadLaneFirst()` broadcast eligible for the wave
// relaxation.
//
// This has to account for two different IR shapes for the same source-level
// `WaveReadLaneFirst(x)`, because `getInstructionUsageType`/`getCallUsageType` classify a call
// that takes `x` directly as an "in" argument as itself being the read (no separate load
// instruction exists for it at this point in the pipeline -- that only appears once a later
// pass, e.g. SSA construction, hoists the argument into its own temporary). So:
//
//  - `readingInst` may itself be the call to `WaveReadLaneFirst`, with the tracked value
//    passed straight in as its argument; or
//  - `readingInst` may be a plain load (or other Load-classified instruction) whose result is
//    consumed *only* (ignoring type-only instructions) as the argument to such a call.
//
// In either shape, we require `WaveReadLaneFirst()` to be the tracked value's only runtime
// consumer, because that broadcast is what makes the relaxation sound. A read with another
// consumer remains diagnostic. We also require the call to resolve directly to the known builtin;
// a user-defined wrapper such as `T MyBroadcast(T x) { return WaveReadLaneFirst(x); }` does not
// qualify. This narrow scope can retain an existing over-warning, but cannot suppress a valid
// diagnostic.
static bool isWaveReadLaneFirstUse(IRInst* readingInst)
{
    if (auto call = as<IRCall>(readingInst))
    {
        if (getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::WaveReadLaneFirst)
            return true;
    }

    IRInst* realUse = nullptr;
    int numRealUses = 0;
    for (auto use = readingInst->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (doesInstOnlyDependOnOperandTypes(user))
            continue;
        numRealUses++;
        realUse = user;
    }
    if (numRealUses != 1)
        return false;
    auto call = as<IRCall>(realUse);
    if (!call)
        return false;
    return getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::WaveReadLaneFirst;
}

// The caller supplies the instructions this checker treats as definite writes. We remove each read
// for which every feasible control-flow path from the function entry passes through one of those
// writes.
//
// A read remains when at least one path can reach it without a preceding definite write, because
// that path can observe the variable while it is still uninitialized.
//
// We compute this definite-assignment property with a forward CFG walk that stops at blocks with a
// definite write. Simple dominance would be too strict: different paths can initialize the value
// with different writes. For example, in `f(out x) && use(x)`, every feasible path to `use(x)`
// passes through the write in `f`, even though no individual write block dominates the use block.
static void removeReadsWithDefiniteWriteOnEveryPath(
    IRGlobalValueWithCode* func,
    const List<IRInst*>& writesTreatedAsDefinite,
    List<IRInst*>& readsWithoutDefiniteWrite,
    const WaveElectionContext& waveElection)
{
    if (readsWithoutDefiniteWrite.getCount() == 0)
        return;

    // We map each definite write to the block that contains it.
    //
    // `collectInstructionByUsage` usually records the writing instruction. For `StoreParent`
    // (for example, an inline SPIR-V assembly operand), it records the containing block instead.
    // We preserve that block-level approximation in `wholeBlockDefiniteWrites`, so every read in
    // such a block counts as following the write.
    HashSet<IRBlock*> blocksWithDefiniteWrite;
    HashSet<IRBlock*> wholeBlockDefiniteWrites;
    HashSet<IRInst*> definiteWriteSet;
    for (auto definiteWrite : writesTreatedAsDefinite)
    {
        if (auto definiteWriteBlock = as<IRBlock>(definiteWrite))
        {
            blocksWithDefiniteWrite.add(definiteWriteBlock);
            wholeBlockDefiniteWrites.add(definiteWriteBlock);
        }
        else if (auto block = as<IRBlock>(definiteWrite->getParent()))
        {
            blocksWithDefiniteWrite.add(block);
            definiteWriteSet.add(definiteWrite);
        }
    }

    // We apply the wave-broadcast relaxation from
    // https://github.com/shader-slang/slang/issues/12545 when every path out of a
    // `WaveIsFirstLane()` true branch reaches a definite write before reconvergence, and the read
    // occurs only after that reconvergence through `WaveReadLaneFirst()`. That read cannot observe
    // the unwritten value: the intrinsic broadcasts from the first active lane, which is exactly
    // the lane that took the guard and performed the write.
    //
    // Known, accepted scope limit: this is a single-thread CFG proof (see
    // `isEveryPathBlockedByDefiniteWrite` and `isWaveReadLaneFirstUse` for the two halves of it).
    // It does not model whether the wave's active-lane mask could shift between the guard and
    // the read -- e.g. an intervening `discard`/`return` taken by only some lanes -- since
    // Slang has no dynamic-uniformity/wave-reconvergence analysis anywhere, and this relaxation
    // does not add one.
    for (auto& guard : waveElection.guards)
    {
        if (!isEveryPathBlockedByDefiniteWrite(
                guard.trueBlock,
                guard.mergeBlock,
                blocksWithDefiniteWrite))
            continue;
        for (Index i = 0; i < readsWithoutDefiniteWrite.getCount();)
        {
            auto read = readsWithoutDefiniteWrite[i];
            auto block = as<IRBlock>(read->getParent());
            if (block && waveElection.dominatorTree->dominates(guard.mergeBlock, block) &&
                isWaveReadLaneFirstUse(read))
            {
                readsWithoutDefiniteWrite.fastRemoveAt(i);
            }
            else
            {
                i++;
            }
        }
    }

    if (readsWithoutDefiniteWrite.getCount() == 0)
        return;

    // When one block contains both a definite write and a read, their order matters. A read before
    // every write can still observe the uninitialized value, while a preceding write initializes
    // the value for the remainder of the block.
    //
    // We record which reads have a preceding definite write in their own block.
    HashSet<IRInst*> readHasPriorDefiniteWriteInBlock;
    for (auto read : readsWithoutDefiniteWrite)
    {
        auto block = as<IRBlock>(read->getParent());
        if (!block)
            continue;
        // A whole-block `StoreParent` write covers every read in that block.
        if (wholeBlockDefiniteWrites.contains(block))
        {
            readHasPriorDefiniteWriteInBlock.add(read);
            continue;
        }
        if (!blocksWithDefiniteWrite.contains(block))
            continue;
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == read)
                break;
            if (definiteWriteSet.contains(inst))
            {
                readHasPriorDefiniteWriteInBlock.add(read);
                break;
            }
        }
    }

    // We relax the structural proof for element-wise initialization inside a loop, such as
    // `[ForceUnroll] for (i) result[i] = ...;`. A write in the body does not dominate a post-loop
    // read because the CFG admits a zero-trip path. In practice these loops commonly have constant,
    // positive trip counts, and diagnosing that structural path produces pervasive false positives.
    //
    // We therefore treat a loop whose body contains a definite write as initialized at its break
    // block. The break block is the loop's reconvergence point, so this does not hide a path that
    // bypasses the loop. We still propagate the unwritten state into the body and diagnose a read
    // that precedes the write there, including the pattern from issue #10658.
    HashSet<IRBlock*> suppressedBreakBlocks;
    for (auto block : func->getBlocks())
    {
        auto loop = as<IRLoop>(block->getTerminator());
        if (!loop)
            continue;
        auto breakBlock = loop->getBreakBlock();

        // We collect the body blocks reachable from the loop target without crossing the break
        // block. A definite write in that region initializes the value at reconvergence.
        //
        // This choice favors fewer false positives. Distinguishing fixed-trip element-wise writes
        // from a genuinely conditional write would require trip-count reasoning, so we accept a
        // rare missed conditional case instead of warning on the common array/vector pattern.
        HashSet<IRBlock*> bodyVisited;
        List<IRBlock*> bodyWork;
        bodyVisited.add(breakBlock); // This sentinel keeps the traversal within the loop body.
        if (auto target = loop->getTargetBlock())
        {
            if (bodyVisited.add(target))
                bodyWork.add(target);
        }
        bool bodyHasDefiniteWrite = false;
        while (bodyWork.getCount())
        {
            auto b = bodyWork.getLast();
            bodyWork.removeLast();
            if (blocksWithDefiniteWrite.contains(b))
            {
                bodyHasDefiniteWrite = true;
                break;
            }
            for (auto succ : b->getSuccessors())
            {
                if (bodyVisited.add(succ))
                    bodyWork.add(succ);
            }
        }
        if (bodyHasDefiniteWrite)
            suppressedBreakBlocks.add(breakBlock);
    }

    // We now traverse the CFG from entry while treating a block with a definite write as a barrier.
    // An unwritten value can enter such a block, but the write executes before control reaches any
    // successor, so the unwritten state does not propagate beyond it.
    //
    // `reachableWithoutDefiniteWrite` therefore contains exactly the blocks reached along a path
    // with no preceding definite write, subject to the loop relaxation above and the
    // infeasible-edge pruning below.
    //
    // We propagate over CFG edges rather than blocks so that the predecessor remains available for
    // pruning short-circuit `&&`/`||` edges with constant phi conditions (see
    // `getInfeasibleBranchFromPredecessor`). A block belongs to the set when any feasible incoming
    // edge carries the unwritten state; the entry block has that state by definition.
    //
    // The worklist holds `(predecessor, block)` edges, with a null predecessor for the synthetic
    // entry edge.
    HashSet<IRBlock*> reachableWithoutDefiniteWrite;
    HashSet<KeyValuePair<IRBlock*, IRBlock*>> visitedEdges;
    List<KeyValuePair<IRBlock*, IRBlock*>> worklist;

    auto enqueueEdge = [&](IRBlock* pred, IRBlock* succ)
    {
        // When the loop relaxation applies, the body has performed its definite write before
        // control reconverges at the break block.
        if (suppressedBreakBlocks.contains(succ))
            return;
        KeyValuePair<IRBlock*, IRBlock*> edge(pred, succ);
        if (visitedEdges.add(edge))
        {
            reachableWithoutDefiniteWrite.add(succ);
            worklist.add(edge);
        }
    };

    if (auto entry = func->getFirstBlock())
        enqueueEdge(nullptr, entry);

    while (worklist.getCount())
    {
        auto edge = worklist.getLast();
        worklist.removeLast();
        IRBlock* pred = edge.key;
        IRBlock* block = edge.value;

        // A definite write in this block prevents the unwritten state from reaching successors.
        if (blocksWithDefiniteWrite.contains(block))
            continue;

        // We prune the outgoing branch made infeasible by the predecessor's constant phi argument.
        IRBlock* infeasibleSucc = pred ? getInfeasibleBranchFromPredecessor(block, pred) : nullptr;

        for (auto succ : block->getSuccessors())
        {
            if (succ == infeasibleSucc)
                continue;
            enqueueEdge(block, succ);
        }
    }

    // A remaining read can observe an uninitialized value exactly when its block is reachable
    // without a definite write and no such write precedes it within that block.
    for (Index i = 0; i < readsWithoutDefiniteWrite.getCount();)
    {
        auto read = readsWithoutDefiniteWrite[i];
        auto block = as<IRBlock>(read->getParent());
        bool hasDefiniteWriteOnEveryPath = !block ||
                                           !reachableWithoutDefiniteWrite.contains(block) ||
                                           readHasPriorDefiniteWriteInBlock.contains(read);
        if (hasDefiniteWriteOnEveryPath)
            readsWithoutDefiniteWrite.fastRemoveAt(i);
        else
            i++;
    }
}

// We collect every read and write of `inst`, including uses through derived addresses and SSA phis.
// Each use is classified once, with use-specific effects taking precedence over generic IR rules.
// For generic uses, we preserve the legacy classifier's approximation that every inferred write is
// definite. A supplied use-specific effect can instead describe a partial or conditional write. We
// treat a defined value entering an alias phi as both a possible write and a write that is definite
// on that incoming edge.
static TrackedVariableUses collectTrackedVariableUses(
    IRInst* inst,
    ConstArrayView<UninitializedVariableUseEffect> useEffects = {})
{
    TrackedVariableUses result;
    HashSet<IRInst*> aliasSet;
    auto addresses = getAliasableInstructions(inst, aliasSet);

    for (auto alias : addresses)
    {
        // TODO: Partial-initialization checking requires tracking the specific parts assigned here.
        for (auto use = alias->firstUse; use; use = use->nextUse)
            collectInstructionByUsage(result, use, alias, useEffects);
    }

    // A defined value flowing into a phi alias initializes the value at that merge point. We record
    // it in both write sets so later reads are not mistaken for uninitialized uses.
    collectPhiMergeWrites(aliasSet, addresses, result.possibleWrites);
    collectPhiMergeWrites(aliasSet, addresses, result.writesTreatedAsDefinite);
    return result;
}

// We find each output-parameter read or return that no possible write can reach. Generic assembly
// counts as a possible write because its operand effects are opaque to this analysis.
static List<IRInst*> findParameterReadsWithoutReachingWrite(
    ReachabilityContext& reachability,
    IRFunc* func,
    IRInst* inst)
{
    auto uses = collectTrackedVariableUses(inst);

    for (const auto& block : func->getBlocks())
    {
        collectGenericAsmPossibleWrites(uses.possibleWrites, block);

        auto terminator = block->getTerminator();
        if (as<IRReturn>(terminator))
            uses.reads.add(terminator);
    }

    removeReadsReachedByPossibleWrite(reachability, uses.possibleWrites, uses.reads);

    return _Move(uses.reads);
}

// We partition reads that can observe an uninitialized value so callers can select the established
// diagnostics without running two independent use-collection walks.
struct UninitializedReads
{
    /// Reads to which no possible write can reach; these receive `UsingUninitialized...`.
    List<IRInst*> readsWithNoReachingWrite;

    /// Reads reached by some write but also by a path with no write treated as definite; these
    /// receive `PossiblyUsingUninitialized...`.
    List<IRInst*> readsWithoutDefiniteWrite;
};

// We find every read of `inst` that can observe an uninitialized value. Reads with no reaching
// possible write receive `UsingUninitialized...`. Among the rest, reads reachable along a path with
// no write this checker treats as definite receive `PossiblyUsingUninitialized...`. We remove the
// first set from the second so each source location receives one diagnostic.
static UninitializedReads findUninitializedReads(
    ReachabilityContext& reachability,
    IRGlobalValueWithCode* func,
    IRInst* inst,
    const WaveElectionContext& waveElection,
    ConstArrayView<UninitializedVariableUseEffect> useEffects = {})
{
    auto uses = collectTrackedVariableUses(inst, useEffects);

    UninitializedReads result;

    result.readsWithNoReachingWrite = uses.reads;
    removeReadsReachedByPossibleWrite(
        reachability,
        uses.possibleWrites,
        result.readsWithNoReachingWrite);

    // Definite assignment adds information only when some write and some read exist; otherwise all
    // reads are already in the first diagnostic class.
    if (uses.possibleWrites.getCount() == 0 || uses.reads.getCount() == 0)
        return result;

    HashSet<IRInst*> readsWithNoReachingWriteSet;
    for (auto read : result.readsWithNoReachingWrite)
        readsWithNoReachingWriteSet.add(read);

    result.readsWithoutDefiniteWrite = uses.reads;
    removeReadsWithDefiniteWriteOnEveryPath(
        func,
        uses.writesTreatedAsDefinite,
        result.readsWithoutDefiniteWrite,
        waveElection);

    // We keep the sets disjoint so each read receives only one diagnostic.
    for (Index i = 0; i < result.readsWithoutDefiniteWrite.getCount();)
    {
        if (readsWithNoReachingWriteSet.contains(result.readsWithoutDefiniteWrite[i]))
            result.readsWithoutDefiniteWrite.fastRemoveAt(i);
        else
            i++;
    }

    return result;
}

// We use this emitter for both the `UsingUninitialized...` and
// `PossiblyUsingUninitialized...` diagnostic families. Within either family, we choose the
// named-variable form (`TVarDiag`) when `inst` has a user-visible name, and the typed-value form
// (`TValDiag`) for poison values and other compiler-synthesized intermediates.
template<typename TVarDiag, typename TValDiag>
static void diagnoseUninitializedUses(
    DiagnosticSink* sink,
    IRInst* inst,
    IRType* type,
    const List<IRInst*>& reads)
{
    bool hasName = inst->findDecoration<IRNameHintDecoration>() != nullptr ||
                   inst->findDecoration<IRLinkageDecoration>() != nullptr;

    for (auto read : reads)
    {
        if (hasName)
        {
            StringBuilder varNameSb;
            printDiagnosticArg(varNameSb, inst);
            sink->diagnose(TVarDiag{
                .varName = varNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
        else
        {
            StringBuilder typeNameSb;
            printDiagnosticArg(typeNameSb, type);
            sink->diagnose(TValDiag{
                .typeName = typeNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
    }
}

// We determine whether a possible write through `inst` or one of its aliases can reach `reference`.
// Constructor checking uses this query to identify fields initialized before a returned value.
static bool hasWriteReachingInstruction(
    ReachabilityContext& reachability,
    IRInst* reference,
    IRInst* inst)
{
    TrackedVariableUses uses;

    for (auto alias : getAliasableInstructions(inst))
    {
        for (auto use = alias->firstUse; use; use = use->nextUse)
            collectInstructionByUsage(uses, use, alias);
    }

    for (auto write : uses.possibleWrites)
    {
        if (reachability.isInstReachable(write, reference))
            return true;
    }

    return false;
}

static IRInst* traceInstOrigin(IRInst* inst)
{
    if (auto load = as<IRLoad>(inst))
        return traceInstOrigin(load->getPtr());

    return inst;
}

static bool isReturnedValue(IRInst* inst)
{
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();
        if (as<IRReturn>(user))
            return true;

        // Loading from a Ptr type should be
        // treated as an aliased path to any return
        IRLoad* load = as<IRLoad>(user);
        if (load && isReturnedValue(load))
            return true;
    }
    return false;
}

static bool isDirectlyWrittenTo(IRInst* inst)
{
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        InstructionUsageType usage = getInstructionUsageType(use, inst);
        if (usage == Store || usage == StoreParent)
            return true;
    }

    return false;
}

static List<IRStructField*> checkFieldsFromExit(
    ReachabilityContext& reachability,
    IRReturn* ret,
    IRStructType* type)
{
    IRInst* origin = traceInstOrigin(ret->getVal());

    // We don't want to warn on delegated construction
    if (!isUninitializedValue(origin))
        return {};

    // Check if the origin instruction is ever written to
    if (isDirectlyWrittenTo(origin))
        return {};

    // Now we can look for all references to fields
    HashSet<IRStructKey*> usedKeys;
    for (auto use = origin->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();

        auto fieldAddress = as<IRFieldAddress>(user);
        if (!fieldAddress || !hasWriteReachingInstruction(reachability, ret, user))
            continue;

        IRInst* field = fieldAddress->getField();
        usedKeys.add(as<IRStructKey>(field));
    }

    List<IRStructField*> uninitializedFields;

    auto fields = type->getFields();
    for (auto field : fields)
    {
        if (canIgnoreType(field->getFieldType(), nullptr))
            continue;

        if (!usedKeys.contains(field->getKey()))
            uninitializedFields.add(field);
    }

    return uninitializedFields;
}

static void checkConstructor(IRFunc* func, ReachabilityContext& reachability, DiagnosticSink* sink)
{
    auto constructor = func->findDecoration<IRConstructorDecoration>();
    if (!constructor)
        return;

    IRStructType* stype = as<IRStructType>(func->getResultType());
    if (!stype)
        return;

    // Don't bother giving warnings if its not being used
    bool synthesized = constructor->getSynthesizedStatus();
    if (synthesized && !func->firstUse)
        return;

    auto printWarnings = [&](const List<IRStructField*>& fields, IRReturn* ret)
    {
        for (auto field : fields)
        {
            StringBuilder typeNameSb;
            printDiagnosticArg(typeNameSb, stype);
            StringBuilder fieldNameSb;
            printDiagnosticArg(fieldNameSb, field->getKey());
            if (synthesized)
            {
                // The field key's source location can be empty (e.g. when the
                // struct definition comes from a linked module). Fall back to
                // the struct type's location and then the constructor function
                // so the warning always points somewhere meaningful.
                SourceLoc loc = field->getKey()->sourceLoc;
                if (!loc.isValid())
                    loc = stype->sourceLoc;
                if (!loc.isValid())
                    loc = func->sourceLoc;
                sink->diagnose(Diagnostics::FieldNotDefaultInitialized{
                    .typeName = typeNameSb.produceString(),
                    .fieldName = fieldNameSb.produceString(),
                    .location = loc,
                });
            }
            else
            {
                sink->diagnose(Diagnostics::ConstructorUninitializedField{
                    .fieldName = fieldNameSb.produceString(),
                    .location = ret->sourceLoc,
                });
            }
        }
    };

    // Work backwards, get exit points and find sources
    for (auto block : func->getBlocks())
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->next)
        {
            auto ret = as<IRReturn>(inst);
            if (!ret)
                continue;

            auto fields = checkFieldsFromExit(reachability, ret, stype);
            printWarnings(fields, ret);
        }
    }
}

static void checkParameterAsOut(
    ReachabilityContext& reachability,
    IRFunc* func,
    IRParam* param,
    DiagnosticSink* sink)
{
    auto reads = findParameterReadsWithoutReachingWrite(reachability, func, param);
    for (auto read : reads)
    {
        StringBuilder paramNameSb;
        printDiagnosticArg(paramNameSb, param);
        if (as<IRTerminatorInst>(read))
        {
            sink->diagnose(Diagnostics::ReturningWithUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
        else
        {
            sink->diagnose(Diagnostics::UsingUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = read->sourceLoc,
            });
        }
    }
}

static void checkUninitializedValues(IRFunc* func, DiagnosticSink* sink)
{
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;

    ReachabilityContext reachability(func);

    // Computed once per function (not once per tracked variable) -- see the comment on
    // `WaveElectionContext` for why.
    auto waveElection = collectWaveElectionContext(func);

    // Used for a further analysis and to skip usual return checks
    auto constructor = func->findDecoration<IRConstructorDecoration>();

    // Special checks for stages e.g. raytracing shader
    Stage stage = Stage::Unknown;
    if (auto entry = func->findDecoration<IREntryPointDecoration>())
        stage = entry->getProfile().getStage();

    // Check out parameters
    if (!isUnmodifying(func))
    {
        int index = 0;
        for (auto param : firstBlock->getParams())
        {
            ParameterCheckType checkType = isPotentiallyUnintended(param, stage, index);
            if (checkType == AsOut)
                checkParameterAsOut(reachability, func, param, sink);
            index++;
        }
    }

    // Check ordinary instructions
    for (auto block : func->getBlocks())
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (!isUninitializedValue(inst))
                continue;

            // This will be looked into later
            if (constructor && isReturnedValue(inst))
                continue;

            IRType* type = inst->getFullType();
            if (canIgnoreType(type, nullptr))
                continue;

            auto uninitializedReads =
                findUninitializedReads(reachability, func, inst, waveElection);
            diagnoseUninitializedUses<
                Diagnostics::UsingUninitializedVariable,
                Diagnostics::UsingUninitializedValue>(
                sink,
                inst,
                type,
                uninitializedReads.readsWithNoReachingWrite);

            diagnoseUninitializedUses<
                Diagnostics::PossiblyUsingUninitializedVariable,
                Diagnostics::PossiblyUsingUninitializedValue>(
                sink,
                inst,
                type,
                uninitializedReads.readsWithoutDefiniteWrite);
        }
    }

    // Separate analysis for constructors
    checkConstructor(func, reachability, sink);
}

// Returns true if this global's value is supplied by host code rather than by an in-module
// initializer -- the documented `__global public __extern_cpp int myGlobal;` pattern for CPU
// targets (docs/cpu-target.md), where the host locates the unmangled symbol by name and sets it
// directly. Such a global has no initializer by design and so, like the externally-supplied globals
// exempted in checkUninitializedGlobals, must not warn E41017. The predicate keys off
// `__extern_cpp` (unmangled name) together with external linkage (`export`/`public`).
static bool isHostProvidedGlobal(IRGlobalVar* variable)
{
    if (!variable->findDecoration<IRExternCppDecoration>())
        return false;

    return variable->findDecoration<IRExportDecoration>() ||
           variable->findDecoration<IRHLSLExportDecoration>() ||
           variable->findDecoration<IRPublicDecoration>();
}

static void checkUninitializedGlobals(IRGlobalVar* variable, DiagnosticSink* sink)
{
    IRType* type = variable->getFullType();
    if (canIgnoreType(type, nullptr))
        return;

    // Check for semantic decorations
    // (e.g. globals like gl_GlobalInvocationID)
    if (variable->findDecoration<IRSemanticDecoration>())
        return;

    if (variable->findDecoration<IRGlobalInputDecoration>())
        return;

    if (variable->findDecoration<IRVulkanHitAttributesDecoration>())
        return;

    if (isHostProvidedGlobal(variable))
        return;

    // Check for initialization blocks
    for (auto inst : variable->getChildren())
    {
        if (as<IRBlock>(inst))
            return;
    }

    auto addresses = getAliasableInstructions(variable);

    List<IRInst*> reads;
    for (auto alias : addresses)
    {
        for (auto use = alias->firstUse; use; use = use->nextUse)
        {
            InstructionUsageType usage = getInstructionUsageType(use, alias);
            if (usage == Store || usage == StoreParent)
                return;

            if (usage == Load)
                reads.add(use->getUser());
        }
    }

    for (auto read : reads)
    {
        StringBuilder varNameSb;
        printDiagnosticArg(varNameSb, variable);
        sink->diagnose(Diagnostics::UsingUninitializedGlobalVariable{
            .varName = varNameSb.produceString(),
            .location = read->sourceLoc,
        });
    }
}

// Some transformations introduce a local after the module-wide uninitialized-value pass has run.
// We rerun the same reachability and definite-assignment analyses for that local only, using
// use-specific effects when a synthesized ABI's parameter directions do not express the semantic
// reads and writes.
void checkForUsingUninitializedVariable(
    IRGlobalValueWithCode* code,
    IRInst* variable,
    ConstArrayView<UninitializedVariableUseEffect> useEffects,
    DiagnosticSink* sink)
{
    ReachabilityContext reachability(code);
    auto waveElection = collectWaveElectionContext(code);
    auto uninitializedReads =
        findUninitializedReads(reachability, code, variable, waveElection, useEffects);
    auto type = variable->getFullType();

    diagnoseUninitializedUses<
        Diagnostics::UsingUninitializedVariable,
        Diagnostics::UsingUninitializedValue>(
        sink,
        variable,
        type,
        uninitializedReads.readsWithNoReachingWrite);
    diagnoseUninitializedUses<
        Diagnostics::PossiblyUsingUninitializedVariable,
        Diagnostics::PossiblyUsingUninitializedValue>(
        sink,
        variable,
        type,
        uninitializedReads.readsWithoutDefiniteWrite);
}

void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink)
{
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            checkUninitializedValues(func, sink);
        }
        else if (auto generic = as<IRGeneric>(inst))
        {
            auto retVal = findGenericReturnVal(generic);
            if (auto funcVal = as<IRFunc>(retVal))
                checkUninitializedValues(funcVal, sink);
        }
        else if (auto global = as<IRGlobalVar>(inst))
        {
            checkUninitializedGlobals(global, sink);
        }
    }
}
} // namespace Slang
