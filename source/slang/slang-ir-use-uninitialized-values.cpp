#include "slang-ir-use-uninitialized-values.h"

#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

// This file diagnoses reads that may observe an uninitialized IR value. We obtain read and write
// effects in two ways. The pre-link module-wide check follows aliases and classifies instruction
// opcodes. Resource-global legalization later creates entry-point locals that did not exist during
// that check, so it supplies an exhaustive effect record for every runtime access to each new
// local.
//
// Both modes use the same two-stage analysis. We first find reads that no possible write can reach.
// For the remaining reads, we propagate uninitialized state through the CFG until a propagation
// boundary stops it. For a generated resource local, the producer supplies a boundary only when
// every execution that continues past an instruction has a fully assigned value. The older module-
// wide analysis instead treats every classified possible write as a boundary, and its loop
// heuristic stops propagation at a loop break when any block in the loop body contains such a
// write. The generated-local analysis preserves every path that reaches a read without crossing a
// supplied boundary. The module-wide analysis also retains its existing wave-election exception;
// the generated resource-local analysis does not use that exception.

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

        // Type-only queries do not observe whether the value is initialized.
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

// Does `inst` depend (transitively, through its operands) on any value in `aliasSet`?
// Used to tell a genuinely-defined phi argument from one that is merely the tracked
// uninitialized value carried/derived through computation. For example, with a
// loop-carried accumulator the back-edge argument is `add(total1, a[i])`, which depends on
// the loop-header phi `total1` (an alias) and so is *not* a real definition; whereas a
// conditionally-stored value like `load(candidateProceduralAttrs)` depends on nothing in
// the alias set and *is* a real definition. A `seen` set bounds the operand walk against
// cycles (phis can be mutually recursive).
static bool dependsOnAlias(IRInst* inst, const HashSet<IRInst*>& aliasSet, HashSet<IRInst*>& seen)
{
    if (aliasSet.contains(inst))
        return true;
    if (!seen.add(inst))
        return false;
    // Stop at block boundaries: a block parameter's "operands" are its phi arguments,
    // which we reach via the predecessors, not via getOperand. Following an alias param is
    // already handled by aliasSet membership above; any non-alias param is treated as an
    // independent definition.
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

// We record phi edges that bring a genuinely initialized value into an alias of the tracked
// uninitialized value. Each such edge is a possible write at the merge, so a later read does not
// belong to the diagnostic set for reads with no preceding possible write.
//
// Consider a value that is assigned on only some paths and then read after a loop:
//
//     MyAttrs attrs;                       // uninitialized
//     for (;;) { ...; if (cond) attrs = computed; ... }
//     use(attrs);                          // reads the loop-header phi
//
// The loop-header phi merges the undefined pre-loop value with `computed`. We record the
// `computed` edge as a possible write even though SSA has no `store` instruction for it. The later
// path-sensitive analysis then decides whether uninitialized state can still reach the read. A
// purely loop-carried accumulator such as `total += a[i]` has no independent initialized input: its
// non-undefined phi argument still depends on the phi. `dependsOnAlias` therefore rejects that
// edge, and the accumulator read remains in the set with no preceding possible write.
static void collectPhiMergePossibleWrites(
    const HashSet<IRInst*>& aliasSet,
    const List<IRInst*>& aliases,
    List<IRInst*>& possibleWrites)
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
            // Only an argument that does not derive from the tracked uninitialized value is
            // a genuine definition reaching this merge point.
            HashSet<IRInst*> seen;
            if (dependsOnAlias(arg, aliasSet, seen))
                continue;
            // We record the possible write at the predecessor edge, not at the argument's
            // definition. The variable becomes initialized only on this incoming edge; the value
            // `arg` may have been defined earlier (e.g. `float y = 1; if (cond) x = y;`). Using its
            // definition point would make the definite-assignment walk treat the unassigned edge
            // as initialized too, suppressing the legitimate "may be uninitialized" diagnostic.
            possibleWrites.add(branch);
        }
    }
}

/// A `ModuleWideOperandUseClassification` records how the module-wide uninitialized-value
/// heuristic treats one exact operand use.
///
/// This is not an exhaustive effect set. The legacy analysis assigns each use to one category. If
/// a use may both read and write, or if its effects are opaque, it records only `PossibleWrite`.
/// That choice makes the no-write proof inconclusive and treats the instruction as a propagation
/// boundary, favoring suppression of a warning over diagnosing a possible read. The generated-
/// resource-local analysis uses `GeneratedResourceLocalUseEffect` when it needs independent read,
/// write, and propagation-boundary facts.
enum class ModuleWideOperandUseClassification
{
    Ignored,
    PossibleWrite,
    EnclosingSPIRVAsmPossibleWrite,
    Read,
};

/// Classify `argumentUse` for the module-wide uninitialized-value heuristic.
///
/// A call may pass the same value to parameters with different directions. We therefore inspect
/// the parameter that corresponds to this exact argument use instead of searching for the first
/// argument with the same value.
static ModuleWideOperandUseClassification classifyModuleWideCallArgumentUse(
    IRCall* call,
    IRUse* argumentUse)
{
    SLANG_ASSERT(argumentUse != call->getCalleeUse());
    IRInst* callee = call->getCallee();

    // Before automatic-differentiation lowering, an `IRTranslateBase` callee does not expose the
    // translated function's parameter directions. We retain the legacy `PossibleWrite`
    // classification, which prevents an unsupported translated call from causing a speculative
    // uninitialized-read diagnostic.
    if (as<IRTranslateBase>(callee))
        return ModuleWideOperandUseClassification::PossibleWrite;

    // `findCallArgumentParameterType` preserves the identity of `argumentUse`, including when
    // another argument uses the same value. When the call-site type provides no corresponding
    // parameter, we cannot prove that the callee only writes the argument. We therefore treat the
    // argument as a read so that an unknown call contract cannot hide an uninitialized use.
    auto parameterType = findCallArgumentParameterType(call, argumentUse);
    if (!parameterType)
        return ModuleWideOperandUseClassification::Read;

    // `out`, `inout`, and `ref` parameters may write the tracked value, so the module-wide
    // heuristic classifies them as `PossibleWrite`. An `inout` or `ref` parameter may also read the
    // incoming value, but this one-category heuristic deliberately omits that read and treats the
    // call as a propagation boundary. Every other parameter evaluates the argument as an input and
    // is classified as `Read`. These classifications use the call-site signature rather than a
    // specialized callee body.
    if (as<IROutParamType>(parameterType))
        return ModuleWideOperandUseClassification::PossibleWrite;
    if (as<IRBorrowInOutParamType>(parameterType))
        return ModuleWideOperandUseClassification::PossibleWrite;
    if (as<IRRefParamType>(parameterType))
        return ModuleWideOperandUseClassification::PossibleWrite;
    return ModuleWideOperandUseClassification::Read;
}

/// Classify the instruction that owns `use` for the module-wide uninitialized-value heuristic.
///
/// We retain the exact operand use because one instruction can use the same value in roles with
/// different classifications. For a call operand, we use the corresponding parameter direction
/// when one exists. Every other opcode without a more precise rule receives the legacy
/// read-or-possible-write classification described above.
static ModuleWideOperandUseClassification classifyModuleWideOperandUse(IRUse* use)
{
    auto user = use->getUser();

    // Type-only queries do not observe whether the value is initialized.
    if (doesInstOnlyDependOnOperandTypes(user))
        return ModuleWideOperandUseClassification::Ignored;

    // Debug records describe the program without reading or writing its runtime values.
    if (isDebugInfoInst(user))
        return ModuleWideOperandUseClassification::Ignored;

    // `getAliasableInstructions` follows each alias-producing instruction to its eventual users.
    // We therefore defer read/write classification until one of those users consumes the alias.
    if (isAliasable(user))
        return ModuleWideOperandUseClassification::Ignored;

    switch (user->getOp())
    {
    case kIROp_Loop:
    case kIROp_UnconditionalBranch:
        // The alias walk follows each branch argument to the corresponding block parameter. The
        // branch itself adds no read or write beyond that value transfer.
        return ModuleWideOperandUseClassification::Ignored;

    case kIROp_Call:
        {
            auto call = as<IRCall>(user);

            // Invoking a function value reads it. For an argument, the corresponding parameter
            // direction determines whether this particular operand reads or may write the tracked
            // value.
            if (use == call->getCalleeUse())
                return ModuleWideOperandUseClassification::Read;
            return classifyModuleWideCallArgumentUse(call, use);
        }

    case kIROp_Store:
    case kIROp_AtomicStore:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
        // These instructions write through operand zero and read the value in operand one. We use
        // the operand identity rather than the operand value because both operands may refer to the
        // same instruction. When operand one has pointer type, the store copies the address without
        // reading the pointee, so it does not read the tracked value.
        if (use == user->getOperandUse(1) && !as<IRPtrTypeBase>(use->get()->getDataType()))
            return ModuleWideOperandUseClassification::Read;
        return ModuleWideOperandUseClassification::PossibleWrite;

    case kIROp_SPIRVAsm:
        // A SPIR-V assembly instruction is opaque. We retain the legacy `PossibleWrite`
        // classification so that the module-wide heuristic does not diagnose a possible read whose
        // assembly-level effect it cannot inspect.
        return ModuleWideOperandUseClassification::PossibleWrite;

    case kIROp_SPIRVAsmOperandInst:
        // A SPIR-V assembly operand record belongs to an enclosing assembly instruction. The
        // assembly instruction is the executable operation, so we record the possible write there.
        return ModuleWideOperandUseClassification::EnclosingSPIRVAsmPossibleWrite;

    case kIROp_MakeExistential:
    case kIROp_MakeExistentialWithRTTI:
        // Existential construction packages and therefore reads the tracked value. The legacy
        // heuristic nevertheless records `PossibleWrite` for this opcode. We preserve that
        // classification to avoid changing warning results as part of this refactor; because the
        // heuristic stores only one category, it consequently omits the known read.
        return ModuleWideOperandUseClassification::PossibleWrite;

    case kIROp_ManagedPtrAttach:
    case kIROp_Unmodified:
        // These marker instructions preserve the tracked pointer for later uses that this
        // module-wide heuristic does not follow. We record `PossibleWrite` so that the no-write
        // proof becomes inconclusive and suppresses a warning. Because the heuristic stores only
        // one category, this choice may omit a simultaneous read.
        return ModuleWideOperandUseClassification::PossibleWrite;

    default:
        // For every remaining opcode, the legacy policy classifies a pointer-producing instruction
        // as a possible write through its result and a non-pointer result as a read. The
        // possible-write choice intentionally follows the warning-suppression policy above rather
        // than claiming that the instruction cannot also read the pointee.
        if (as<IRPtrTypeBase>(user->getDataType()))
            return ModuleWideOperandUseClassification::PossibleWrite;
        return ModuleWideOperandUseClassification::Read;
    }
}

/// Append each generic-assembly instruction in `block` as a possible write.
static void collectGenericAssemblyPossibleWrites(List<IRInst*>& possibleWrites, IRBlock* block)
{
    // Generic assembly does not expose operand effects, so the module-wide analysis treats the
    // entire instruction as a possible write.
    for (auto inst = block->getFirstInst(); inst; inst = inst->next)
    {
        if (as<IRGenericAsm>(inst))
            possibleWrites.add(inst);
    }
}

/// Append the instruction that owns `use` to the list selected by the module-wide classification.
///
/// We pass the exact `IRUse` to `classifyModuleWideOperandUse` so two occurrences of the same value
/// in one instruction can receive different classifications.
static void appendModuleWideOperandUseClassification(
    List<IRInst*>& possibleWrites,
    List<IRInst*>& reads,
    IRUse* use)
{
    // We classify the exact operand and append the instruction where the heuristic applies that
    // classification. A SPIR-V assembly operand reports its parent assembly instruction because
    // reachability is defined for that instruction rather than for the operand record.
    auto user = use->getUser();
    auto classification = classifyModuleWideOperandUse(use);
    switch (classification)
    {
    case ModuleWideOperandUseClassification::Ignored:
        return;
    case ModuleWideOperandUseClassification::Read:
        return reads.add(user);
    case ModuleWideOperandUseClassification::PossibleWrite:
        return possibleWrites.add(user);
    case ModuleWideOperandUseClassification::EnclosingSPIRVAsmPossibleWrite:
        return possibleWrites.add(user->getParent());
    }
}

/// Retain reads that no possible write can reach.
static void retainReadsUnreachableFromPossibleWrites(
    ReachabilityContext& reachability,
    const List<IRInst*>& possibleWrites,
    List<IRInst*>& reads)
{
    // We report a read as uninitialized only when no possible write can execute before it. We
    // remove reads reachable from possible writes and retain the rest.
    for (auto write : possibleWrites)
    {
        for (Index i = 0; i < reads.getCount();)
        {
            if (reachability.isInstReachable(write, reads[i]))
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

// A `WaveElectionGuard` records an `if (WaveIsFirstLane()) { ... }` statement in the function
// being analyzed. We collect these guards once per function rather than once per tracked value.
//
// `trueBlock` is the guard's true-branch entry block, and `mergeBlock` is the `IRIfElse`
// reconvergence block. `removeReadsAcceptedByWaveElectionHeuristic` uses both blocks to apply the
// wave-broadcast exception described at that function.
struct WaveElectionGuard
{
    IRBlock* trueBlock;
    IRBlock* mergeBlock;
};

// We collect every `if (WaveIsFirstLane())` guard in `func`.
//
// We require the condition to be a direct call to the known builtin; we do not look through
// boolean negation. `!WaveIsFirstLane()` lowers to an explicit `not` operand rather than swapped
// true and false blocks. In that form the elected lane skips the guarded write, so the legacy
// wave-election exception does not apply.
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

// We collect the `WaveIsFirstLane()` guards and build the dominator tree once per function because
// both are independent of the tracked value. The dominator tree lets the wave-election heuristic
// require the guard's merge block to dominate the block containing a candidate read.
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

/// Return whether any control-flow path reaches `to` from `from`.
static bool isBlockReachableFrom(IRBlock* from, IRBlock* to)
{
    // We walk forward from `from` and stop as soon as we reach `to`.
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

/// Return whether every path from `from` to `to` crosses a propagation boundary.
///
/// We restrict the walk to paths that start at `from`. Whole-function dominance would include the
/// sibling branch of the surrounding `if` and could never establish that a boundary after `from`
/// is mandatory. We require `to` to be reachable, then stop each path at its first boundary.
/// Reaching `to` without crossing a boundary disproves the claim.
static bool doesEveryPathFromToCrossPropagationBoundary(
    IRBlock* from,
    IRBlock* to,
    const HashSet<IRBlock*>& blocksWithPropagationBoundary)
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
            return false;
        if (blocksWithPropagationBoundary.contains(block))
            continue;
        for (auto succ : block->getSuccessors())
        {
            if (visited.add(succ))
                worklist.add(succ);
        }
    }
    return true;
}

/// Return whether `readingInst` has the form accepted by the wave-election exception.
//
// We account for two IR shapes for a call written as `WaveReadLaneFirst(x)`. When the call
// takes `x` directly as an input argument, the usage classifier treats the call itself as the read;
// no separate load exists at this point in the pipeline. Therefore:
//
//  - `readingInst` may be the call to `WaveReadLaneFirst`, with the tracked value passed directly;
//    or
//  - `readingInst` may be a load whose result is consumed only by such a call, apart from type-only
//    queries.
//
// For the load form, we require the builtin call to be the load result's only runtime consumer.
// A load with another consumer remains subject to the normal path-sensitive proof. We also require
// a direct call to the known builtin rather than looking through a user-defined wrapper. A wrapper
// therefore retains the existing conservative warning instead of suppressing a valid diagnostic.
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

/// A `LoopUninitializedPathPolicy` states how the analysis treats a loop exit when the loop body
/// contains a propagation boundary.
enum class LoopUninitializedPathPolicy
{
    /// Preserve every path that reaches the loop's break block without crossing a boundary.
    PreservePathsWithoutPropagationBoundary,

    /// Stop propagation at the break block after any boundary in the loop body.
    StopAtBreakAfterAnyBodyBoundary,
};

/// An `UninitializedStatePropagationBoundaryLocations` value maps propagation boundaries to the
/// block-level facts used by the control-flow analysis.
struct UninitializedStatePropagationBoundaryLocations
{
    /// Blocks that contain a propagation boundary.
    HashSet<IRBlock*> blocksWithPropagationBoundary;

    /// Boundary instructions whose order relative to reads can be inspected.
    HashSet<IRInst*> propagationBoundaryInstructions;
};

/// Map each propagation boundary to its containing block and ordering representation.
///
/// For a generated resource local, the caller supplies only instructions after which execution can
/// continue with a fully assigned value. The older module-wide analysis supplies every classified
/// possible write because that analysis uses a less precise heuristic.
static UninitializedStatePropagationBoundaryLocations collectPropagationBoundaryLocations(
    const List<IRInst*>& propagationBoundaries)
{
    // Every boundary identifies an executable instruction. We record its block for inter-block
    // reachability and retain the instruction itself so that we can compare its order with reads in
    // the same block.
    UninitializedStatePropagationBoundaryLocations result;
    for (auto boundary : propagationBoundaries)
    {
        if (auto block = as<IRBlock>(boundary->getParent()))
        {
            result.blocksWithPropagationBoundary.add(block);
            result.propagationBoundaryInstructions.add(boundary);
        }
    }
    return result;
}

/// Remove reads accepted by the existing wave-election heuristic.
static void removeReadsAcceptedByWaveElectionHeuristic(
    const UninitializedStatePropagationBoundaryLocations& boundaries,
    const WaveElectionContext& waveElection,
    List<IRInst*>& reads)
{
    // We preserve the module-wide exception for the wave-broadcast pattern in issue #12545. The
    // module-wide analysis treats each classified possible write as a propagation boundary. A
    // read used only by `WaveReadLaneFirst()` is removed when every path through the elected branch
    // crosses such a boundary before reconvergence and the merge block dominates the read's block.
    //
    // This is the legacy heuristic used by the module-wide analysis. It examines one invocation's
    // CFG and does not establish that the active-lane mask remains unchanged between the election
    // and the broadcast. The generated-resource-local analysis does not use this exception.
    for (auto& guard : waveElection.guards)
    {
        if (!doesEveryPathFromToCrossPropagationBoundary(
                guard.trueBlock,
                guard.mergeBlock,
                boundaries.blocksWithPropagationBoundary))
            continue;
        for (Index i = 0; i < reads.getCount();)
        {
            auto read = reads[i];
            auto readBlock = as<IRBlock>(read->getParent());
            if (!readBlock || !waveElection.dominatorTree->dominates(guard.mergeBlock, readBlock))
            {
                i++;
                continue;
            }
            if (!isWaveReadLaneFirstUse(read))
            {
                i++;
                continue;
            }
            reads.fastRemoveAt(i);
        }
    }
}

/// Return the reads preceded by a propagation boundary in the same block.
static HashSet<IRInst*> findReadsWithPriorPropagationBoundaryInBlock(
    const UninitializedStatePropagationBoundaryLocations& boundaries,
    const List<IRInst*>& reads)
{
    // Block reachability cannot distinguish the order of a read and boundary in one block. For an
    // instruction-level effect, we therefore scan from the start of the block until the read.
    HashSet<IRInst*> result;
    for (auto read : reads)
    {
        auto block = as<IRBlock>(read->getParent());
        if (!block)
            continue;
        if (!boundaries.blocksWithPropagationBoundary.contains(block))
            continue;
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == read)
                break;
            if (boundaries.propagationBoundaryInstructions.contains(inst))
            {
                result.add(read);
                break;
            }
        }
    }
    return result;
}

/// Return whether the body of `loop` contains a propagation boundary.
static bool doesLoopBodyContainPropagationBoundary(
    IRLoop* loop,
    const HashSet<IRBlock*>& blocksWithPropagationBoundary)
{
    // We walk from the loop target without crossing the break block. In the module-wide mode, every
    // supplied boundary is a classified possible write, so a boundary in any visited block enables
    // the loop heuristic.
    HashSet<IRBlock*> visited;
    List<IRBlock*> workList;
    visited.add(loop->getBreakBlock());
    if (auto target = loop->getTargetBlock())
    {
        if (visited.add(target))
            workList.add(target);
    }
    while (workList.getCount())
    {
        auto block = workList.getLast();
        workList.removeLast();
        if (blocksWithPropagationBoundary.contains(block))
            return true;
        for (auto successor : block->getSuccessors())
        {
            if (visited.add(successor))
                workList.add(successor);
        }
    }
    return false;
}

/// Return the loop break blocks at which the selected policy stops propagation.
static HashSet<IRBlock*> collectLoopBreakBlocksThatStopPropagation(
    IRGlobalValueWithCode* func,
    const HashSet<IRBlock*>& blocksWithPropagationBoundary,
    LoopUninitializedPathPolicy policy)
{
    // The pre-link module-wide analysis stops propagation at a loop break when any block in the
    // loop body contains a classified possible write. This heuristic supports aggregates that are
    // initialized element by element, but it can also suppress a warning for a zero-trip loop or a
    // conditional write. Distinguishing those cases requires path and trip-count analysis that this
    // check does not perform.
    HashSet<IRBlock*> result;
    if (policy == LoopUninitializedPathPolicy::PreservePathsWithoutPropagationBoundary)
        return result;

    for (auto block : func->getBlocks())
    {
        auto loop = as<IRLoop>(block->getTerminator());
        if (!loop)
            continue;
        if (doesLoopBodyContainPropagationBoundary(loop, blocksWithPropagationBoundary))
            result.add(loop->getBreakBlock());
    }
    return result;
}

/// An `UninitializedPathTraversal` finds blocks reachable before any propagation boundary.
struct UninitializedPathTraversal
{
    const HashSet<IRBlock*>& blocksWithPropagationBoundary;
    const HashSet<IRBlock*>& loopBreakBlocksThatStopPropagation;
    HashSet<IRBlock*> reachableBlocks;
    HashSet<KeyValuePair<IRBlock*, IRBlock*>> visitedEdges;
    List<KeyValuePair<IRBlock*, IRBlock*>> workList;

    UninitializedPathTraversal(
        const HashSet<IRBlock*>& inBlocksWithPropagationBoundary,
        const HashSet<IRBlock*>& inLoopBreakBlocksThatStopPropagation)
        : blocksWithPropagationBoundary(inBlocksWithPropagationBoundary)
        , loopBreakBlocksThatStopPropagation(inLoopBreakBlocksThatStopPropagation)
    {
    }

    /// Add one CFG edge unless the loop policy suppresses it or the traversal has seen it.
    void addEdge(IRBlock* predecessor, IRBlock* block)
    {
        // A selected break block ends propagation of uninitialized state. Tracking edges instead
        // of only blocks preserves predecessor-specific facts for short-circuit control flow.
        if (loopBreakBlocksThatStopPropagation.contains(block))
            return;
        KeyValuePair<IRBlock*, IRBlock*> edge(predecessor, block);
        if (!visitedEdges.add(edge))
            return;
        reachableBlocks.add(block);
        workList.add(edge);
    }

    /// Walk the CFG from `func`'s entry without crossing a propagation boundary.
    void collect(IRGlobalValueWithCode* func)
    {
        // A boundary blocks propagation to successors, but its block remains reachable because a
        // read can occur before an instruction-level boundary. The predecessor on each work item
        // also lets us omit a branch made infeasible when short-circuit lowering passes a constant
        // block argument.
        if (auto entry = func->getFirstBlock())
            addEdge(nullptr, entry);

        while (workList.getCount())
        {
            auto edge = workList.getLast();
            workList.removeLast();
            auto predecessor = edge.key;
            auto block = edge.value;
            if (blocksWithPropagationBoundary.contains(block))
                continue;

            IRBlock* infeasibleSuccessor = nullptr;
            if (predecessor)
                infeasibleSuccessor = getInfeasibleBranchFromPredecessor(block, predecessor);
            for (auto successor : block->getSuccessors())
            {
                if (successor == infeasibleSuccessor)
                    continue;
                addEdge(block, successor);
            }
        }
    }
};

/// Retain reads reached before any propagation boundary.
static void retainReadsReachedBeforePropagationBoundary(
    const HashSet<IRBlock*>& blocksReachableWithoutPropagationBoundary,
    const HashSet<IRInst*>& readsWithPriorPropagationBoundaryInBlock,
    List<IRInst*>& reads)
{
    // The CFG traversal answers whether uninitialized state can enter the read's block. The
    // block-local ordering set answers whether a boundary then precedes the read.
    for (Index i = 0; i < reads.getCount();)
    {
        auto read = reads[i];
        auto block = as<IRBlock>(read->getParent());
        if (!block)
        {
            reads.fastRemoveAt(i);
            continue;
        }
        if (!blocksReachableWithoutPropagationBoundary.contains(block))
        {
            reads.fastRemoveAt(i);
            continue;
        }
        if (readsWithPriorPropagationBoundaryInBlock.contains(read))
        {
            reads.fastRemoveAt(i);
            continue;
        }
        i++;
    }
}

/// Retain reads reachable along at least one path without a preceding propagation boundary.
static void retainReadsReachableWithoutPropagationBoundary(
    IRGlobalValueWithCode* func,
    const List<IRInst*>& propagationBoundaries,
    List<IRInst*>& reads,
    const WaveElectionContext& waveElection,
    LoopUninitializedPathPolicy loopPolicy)
{
    // We answer the path-sensitive question in four steps. We first map the supplied boundaries to
    // their containing blocks. We then apply the legacy wave-election exception when the caller has
    // supplied its context. Next, we record same-block ordering and the loop breaks that stop
    // propagation under the selected policy. Finally, we walk the CFG until each boundary and
    // retain reads reached by the remaining uninitialized paths.
    if (reads.getCount() == 0)
        return;

    auto boundaryLocations = collectPropagationBoundaryLocations(propagationBoundaries);
    removeReadsAcceptedByWaveElectionHeuristic(boundaryLocations, waveElection, reads);
    if (reads.getCount() == 0)
        return;

    auto readsWithPriorBoundary =
        findReadsWithPriorPropagationBoundaryInBlock(boundaryLocations, reads);
    auto loopBreakBlocksThatStopPropagation = collectLoopBreakBlocksThatStopPropagation(
        func,
        boundaryLocations.blocksWithPropagationBoundary,
        loopPolicy);

    UninitializedPathTraversal traversal(
        boundaryLocations.blocksWithPropagationBoundary,
        loopBreakBlocksThatStopPropagation);
    traversal.collect(func);
    retainReadsReachedBeforePropagationBoundary(
        traversal.reachableBlocks,
        readsWithPriorBoundary,
        reads);
}

static void collectAliasableReadsAndPossibleWrites(
    IRInst* inst,
    List<IRInst*>& possibleWrites,
    List<IRInst*>& reads)
{
    // We follow address-preserving instructions and collect the read or possible-write category
    // that the module-wide heuristic assigns to each exact use. We also treat a defined value
    // entering an aliasing phi as an initialized value at that merge.
    HashSet<IRInst*> aliasSet;
    auto addresses = getAliasableInstructions(inst, aliasSet);

    for (auto alias : addresses)
    {
        // TODO: We should record which parts are written so partial-initialization checks can use
        // that information.
        for (auto use = alias->firstUse; use; use = use->nextUse)
            appendModuleWideOperandUseClassification(possibleWrites, reads, use);
    }

    // A defined value flowing into a phi alias is a possible write at that merge. We record it so
    // later reads are not classified as having no preceding possible write.
    collectPhiMergePossibleWrites(aliasSet, addresses, possibleWrites);
}

/// Return reads and normal returns that may observe an uninitialized `out` parameter.
static List<IRInst*> getUninitializedOutParameterReadOrReturnSites(
    ReachabilityContext& reachability,
    IRFunc* func,
    IRInst* inst)
{
    // An `out` parameter must be written before the function reads it or returns. We collect reads
    // and possible writes through every alias, add the conservative treatment of generic assembly
    // and returns required by the `out` contract, and then retain only reads that no possible write
    // can reach.
    List<IRInst*> possibleWrites;
    List<IRInst*> sites;

    // We first classify ordinary reads and writes through the parameter and its aliases.
    collectAliasableReadsAndPossibleWrites(inst, possibleWrites, sites);

    // We conservatively treat generic assembly as a possible write because its effects are opaque.
    // Each return is a checkpoint at which the `out` contract requires an initialized value.
    for (const auto& b : func->getBlocks())
    {
        collectGenericAssemblyPossibleWrites(possibleWrites, b);

        auto t = b->getTerminator();
        if (as<IRReturn>(t))
            sites.add(t);
    }

    // We retain the reads and returns for which no possible write can execute first.
    retainReadsUnreachableFromPossibleWrites(reachability, possibleWrites, sites);

    return sites;
}

/// An `UninitializedReadSets` value separates the two diagnostic classes for one variable.
struct UninitializedReadSets
{
    /// Reads reported as uninitialized because no possible write can reach them.
    List<IRInst*> uninitializedReads;

    /// Other reads reported as possibly uninitialized because uninitialized state can reach them.
    List<IRInst*> possiblyUninitializedReads;
};

/// Find the two disjoint diagnostic sets from the supplied reads, possible writes, and propagation
/// boundaries.
static UninitializedReadSets getUninitializedReadsFromCollectedEffects(
    ReachabilityContext& reachability,
    IRGlobalValueWithCode* func,
    const List<IRInst*>& possibleWrites,
    const List<IRInst*>& propagationBoundaries,
    const List<IRInst*>& allReads,
    const WaveElectionContext& waveElection,
    LoopUninitializedPathPolicy loopPolicy)
{
    // We first find reads that no possible write can reach. For the remaining reads, we propagate
    // uninitialized state until the supplied boundaries. We remove the first set from the second
    // so that each read receives only one diagnostic.
    UninitializedReadSets result;
    result.uninitializedReads = allReads;
    retainReadsUnreachableFromPossibleWrites(
        reachability,
        possibleWrites,
        result.uninitializedReads);

    // When there is no possible write, every read already belongs to the first diagnostic class.
    // When there is no read, both diagnostic classes are empty.
    if (possibleWrites.getCount() == 0 || allReads.getCount() == 0)
        return result;

    HashSet<IRInst*> uninitializedReadSet;
    for (auto read : result.uninitializedReads)
        uninitializedReadSet.add(read);

    result.possiblyUninitializedReads = allReads;
    retainReadsReachableWithoutPropagationBoundary(
        func,
        propagationBoundaries,
        result.possiblyUninitializedReads,
        waveElection,
        loopPolicy);

    for (Index i = 0; i < result.possiblyUninitializedReads.getCount();)
    {
        if (uninitializedReadSet.contains(result.possiblyUninitializedReads[i]))
            result.possiblyUninitializedReads.fastRemoveAt(i);
        else
            i++;
    }

    return result;
}

/// Return uninitialized reads found by the module-wide alias and instruction classifier.
static UninitializedReadSets getModuleWideUninitializedReads(
    ReachabilityContext& reachability,
    IRGlobalValueWithCode* func,
    IRInst* inst,
    const WaveElectionContext& waveElection)
{
    // The module-wide analysis infers effects from aliases and instruction opcodes. It treats every
    // classified possible write as both a reachability source and a boundary that stops propagation
    // of uninitialized state. The legacy wave-election and loop heuristics use the same possible-
    // write set. In particular, the loop heuristic stops propagation at a loop break when any block
    // in the loop body contains a classified write.
    List<IRInst*> possibleWrites;
    List<IRInst*> reads;
    collectAliasableReadsAndPossibleWrites(inst, possibleWrites, reads);
    return getUninitializedReadsFromCollectedEffects(
        reachability,
        func,
        possibleWrites,
        possibleWrites,
        reads,
        waveElection,
        LoopUninitializedPathPolicy::StopAtBreakAfterAnyBodyBoundary);
}

/// Collect the exhaustive effects supplied for one generated entry-point resource local.
static void collectGeneratedResourceLocalEffects(
    IRFunc* func,
    IRVar* variable,
    ConstArrayView<GeneratedResourceLocalUseEffect> effects,
    List<IRInst*>& possibleWrites,
    List<IRInst*>& propagationBoundaries,
    List<IRInst*>& reads)
{
    // The resource-global rewrite has already followed every supported operation that transfers
    // storage access and supplied one record for every runtime use reached. We verify that every
    // record names a unique pointer operand in `func`, then translate the records into the three
    // instruction lists consumed by the shared CFG analysis. Separate sets combine multiple
    // operand uses of the same instruction without losing read, possible-write, or propagation-
    // boundary facts.
    HashSet<IRUse*> recordedUses;
    HashSet<IRInst*> recordedPossibleWrites;
    HashSet<IRInst*> recordedPropagationBoundaries;
    HashSet<IRInst*> recordedReads;

    for (auto const& effect : effects)
    {
        // Each record must describe one distinct operand use retained by the resource-global pass.
        SLANG_RELEASE_ASSERT(effect.use);
        SLANG_RELEASE_ASSERT(recordedUses.add(effect.use));

        // The operand must be a pointer in `func`. The producer may have followed an implicit
        // l-value cast whose temporary transfers access to or from `variable`, so this checker does
        // not require pointer provenance from the variable. It can still reject an effect that
        // names another function or a non-address value.
        auto user = effect.use->getUser();
        auto accessedAddress = effect.use->get();
        SLANG_RELEASE_ASSERT(user);
        SLANG_RELEASE_ASSERT(getParentFunc(user) == func);
        SLANG_RELEASE_ASSERT(as<IRBlock>(user->getParent()));
        SLANG_RELEASE_ASSERT(accessedAddress);
        if (accessedAddress != variable)
            SLANG_RELEASE_ASSERT(getParentFunc(accessedAddress) == func);
        SLANG_RELEASE_ASSERT(
            as<IRPtrTypeBase>(unwrapAttributedType(accessedAddress->getDataType())));

        // The caller must state at least one runtime effect for every supplied operand use.
        bool hasRuntimeEffect = effect.readsValue;
        if (effect.mayWriteValue)
            hasRuntimeEffect = true;
        if (effect.stopsUninitializedStatePropagation)
            hasRuntimeEffect = true;
        SLANG_RELEASE_ASSERT(hasRuntimeEffect);

        // We combine effects by executable instruction because separate operands of one call can
        // access the same local. We preserve separate read, possible-write, and
        // propagation-boundary facts. A boundary also counts as a possible write.
        if (effect.readsValue)
        {
            if (recordedReads.add(user))
                reads.add(user);
        }

        bool mayWriteValue = effect.mayWriteValue;
        if (effect.stopsUninitializedStatePropagation)
            mayWriteValue = true;
        if (mayWriteValue)
        {
            if (recordedPossibleWrites.add(user))
                possibleWrites.add(user);
        }

        if (effect.stopsUninitializedStatePropagation)
        {
            if (recordedPropagationBoundaries.add(user))
                propagationBoundaries.add(user);
        }
    }
}

// We emit one diagnostic at each read in the supplied set. We use `TVarDiag` with the value's name
// when `inst` has a user-visible name; otherwise, we use `TValDiag` with its type. Callers select
// either the uninitialized diagnostic pair for reads with no preceding possible write or the
// possibly-uninitialized pair for reads reached while uninitialized state can still propagate.
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

static bool isInstStoredInto(ReachabilityContext& reachability, IRInst* reference, IRInst* inst)
{
    // We ask whether a possible write through `inst` or one of its aliases can reach `reference`.
    // We classify every exact use of those aliases, then query reachability for each resulting
    // write instruction.
    List<IRInst*> possibleWrites;
    List<IRInst*> ignoredReads;

    for (auto alias : getAliasableInstructions(inst))
    {
        for (auto use = alias->firstUse; use; use = use->nextUse)
            appendModuleWideOperandUseClassification(possibleWrites, ignoredReads, use);
    }

    for (auto possibleWrite : possibleWrites)
    {
        if (reachability.isInstReachable(possibleWrite, reference))
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
    // We ask whether the module-wide heuristic classifies an immediate use as a possible write. A
    // SPIR-V assembly operand records that classification on its enclosing instruction, so both
    // possible-write categories answer the question.
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        auto classification = classifyModuleWideOperandUse(use);
        if (classification == ModuleWideOperandUseClassification::PossibleWrite ||
            classification == ModuleWideOperandUseClassification::EnclosingSPIRVAsmPossibleWrite)
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
        if (!fieldAddress || !isInstStoredInto(reachability, ret, user))
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
    auto sites = getUninitializedOutParameterReadOrReturnSites(reachability, func, param);
    for (auto site : sites)
    {
        StringBuilder paramNameSb;
        printDiagnosticArg(paramNameSb, param);
        if (as<IRTerminatorInst>(site))
        {
            sink->diagnose(Diagnostics::ReturningWithUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = site->sourceLoc,
            });
        }
        else
        {
            sink->diagnose(Diagnostics::UsingUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = site->sourceLoc,
            });
        }
    }
}

static void checkUninitializedValues(IRFunc* func, DiagnosticSink* sink)
{
    // A function can expose an uninitialized value through an `out` parameter, an ordinary
    // undefined value or local variable, or an incompletely initialized constructor result. We
    // build the function-wide control-flow information once and check those three categories in
    // that order.
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;

    ReachabilityContext reachability(func);

    // Wave-election structure depends only on the function, so each value analysis shares it.
    auto waveElection = collectWaveElectionContext(func);

    // We diagnose a constructor's returned value field by field instead of reporting one warning
    // for the whole value.
    auto constructor = func->findDecoration<IRConstructorDecoration>();

    // We use the entry-point stage and parameter position to recognize parameters whose language
    // contract differs from an ordinary `out` parameter.
    Stage stage = Stage::Unknown;
    if (auto entry = func->findDecoration<IREntryPointDecoration>())
        stage = entry->getProfile().getStage();

    // We first check each `out` parameter unless the function suppresses modification checks.
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

    // We next inspect each instruction that introduces an undefined value or uninitialized local.
    for (auto block : func->getBlocks())
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (!isUninitializedValue(inst))
                continue;

            // The constructor check below diagnoses a returned value field by field.
            if (constructor && isReturnedValue(inst))
                continue;

            IRType* type = inst->getFullType();
            if (canIgnoreType(type, nullptr))
                continue;

            // We compute both diagnostic classes from one collection of reads and writes. The
            // first class contains reads that no possible write reaches. The second contains other
            // reads that uninitialized state can reach.
            auto readSets = getModuleWideUninitializedReads(reachability, func, inst, waveElection);

            diagnoseUninitializedUses<
                Diagnostics::UsingUninitializedVariable,
                Diagnostics::UsingUninitializedValue>(
                sink,
                inst,
                type,
                readSets.uninitializedReads);

            diagnoseUninitializedUses<
                Diagnostics::PossiblyUsingUninitializedVariable,
                Diagnostics::PossiblyUsingUninitializedValue>(
                sink,
                inst,
                type,
                readSets.possiblyUninitializedReads);
        }
    }

    // We finish by checking each constructor return for fields with no possible reaching write.
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
    // The pre-link module-wide uninitialized-global check diagnoses the case it can prove without
    // the linked call graph: a global has runtime state, no initializer or external provider
    // supplies a value, and no use may write it. We first exclude types with no runtime state and
    // globals whose decorations, exported `__extern_cpp` contract, or initializer block identify a
    // value supplied elsewhere. We then follow aliases to collect reads and look for any possible
    // write. When no write exists anywhere in the module, each read is unambiguously uninitialized.
    //
    // Because this check runs before linking, it cannot prove path-sensitive initialization through
    // the linked call graph. For globals marked for per-invocation replacement, the later resource-
    // global pass uses that graph to diagnose paths that remain uninitialized despite other paths
    // writing the global. Retaining the simpler pre-link no-write proof also diagnoses marked
    // globals when a compilation performs semantic checking without target emission.
    IRType* type = variable->getFullType();
    if (canIgnoreType(type, nullptr))
        return;

    // Semantic, global-input, and hit-attribute decorations identify values supplied by the shader
    // execution environment. An exported `__extern_cpp` global receives its value from host code.
    if (variable->findDecoration<IRSemanticDecoration>())
        return;

    if (variable->findDecoration<IRGlobalInputDecoration>())
        return;

    if (variable->findDecoration<IRVulkanHitAttributesDecoration>())
        return;

    if (isHostProvidedGlobal(variable))
        return;

    // A child block computes an in-module initializer for the global.
    for (auto inst : variable->getChildren())
    {
        if (as<IRBlock>(inst))
            return;
    }

    // A possible write makes this pre-link module-wide test inconclusive. When lowering has marked
    // the global as file- or namespace-scope mutable storage, it may later be selected for per-
    // invocation resource replacement. We record the inconclusive result because entry-point
    // linking can remove the writer before that later path-sensitive analysis runs. The later
    // analysis can then distinguish this case from a global whose reads were already diagnosed by
    // the no-write proof below.
    auto addresses = getAliasableInstructions(variable);
    List<IRInst*> reads;
    for (auto alias : addresses)
    {
        for (auto use = alias->firstUse; use; use = use->nextUse)
        {
            auto classification = classifyModuleWideOperandUse(use);
            if (classification == ModuleWideOperandUseClassification::PossibleWrite ||
                classification ==
                    ModuleWideOperandUseClassification::EnclosingSPIRVAsmPossibleWrite)
            {
                if (variable->findDecoration<IRFileOrNamespaceScopeMutableVarDecoration>())
                {
                    IRBuilder builder(variable->getModule());
                    builder.addDecoration(
                        variable,
                        kIROp_UninitializedGlobalCheckFoundPossibleWriteDecoration);
                }
                return;
            }

            if (classification == ModuleWideOperandUseClassification::Read)
                reads.add(use->getUser());
        }
    }

    // When the classifier finds no possible write, every collected read observes the uninitialized
    // value.
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

void checkForUsingUninitializedGeneratedResourceLocal(
    IRFunc* func,
    IRVar* variable,
    ConstArrayView<GeneratedResourceLocalUseEffect> effects,
    DiagnosticSink* sink)
{
    // The pre-link uninitialized-value pass performs the ordinary-local CFG analysis before the
    // resource-global pass creates `variable`. We apply that CFG analysis to the generated local,
    // but the resource-global pass supplies an exhaustive record for every runtime access instead
    // of asking this function to infer effects from instruction opcodes. We distinguish reads with
    // no preceding possible write from reads reached while uninitialized state can still propagate.
    // We preserve the CFG path that skips each loop body because the generated local must be fully
    // assigned on every path that reaches a read.
    //
    // We first enforce the ownership contract required by the function's CFG.
    SLANG_RELEASE_ASSERT(func);
    SLANG_RELEASE_ASSERT(variable);
    SLANG_RELEASE_ASSERT(isChildInstOf(variable, func));
    SLANG_RELEASE_ASSERT(getParentFunc(variable) == func);

    // We then translate the caller's exhaustive per-use records into instruction effects.
    List<IRInst*> possibleWrites;
    List<IRInst*> propagationBoundaries;
    List<IRInst*> reads;
    collectGeneratedResourceLocalEffects(
        func,
        variable,
        effects,
        possibleWrites,
        propagationBoundaries,
        reads);

    // The shared reduction first finds reads with no preceding possible write. It then finds other
    // reads reached while uninitialized state can still propagate. The `WaveReadLaneFirst`
    // overloads accept only builtin scalar, vector, and matrix types. None of the resource types
    // selected by this pass implement `__BuiltinType`, and a resource array matches none of those
    // overloads. We therefore omit the module-wide wave-election exception.
    ReachabilityContext reachability(func);
    WaveElectionContext waveElection;
    auto uninitializedUses = getUninitializedReadsFromCollectedEffects(
        reachability,
        func,
        possibleWrites,
        propagationBoundaries,
        reads,
        waveElection,
        LoopUninitializedPathPolicy::PreservePathsWithoutPropagationBoundary);

    // Finally, we issue the same two diagnostic classes as the module-wide analysis.
    auto valueType = variable->getDataType()->getValueType();
    diagnoseUninitializedUses<
        Diagnostics::UsingUninitializedVariable,
        Diagnostics::UsingUninitializedValue>(
        sink,
        variable,
        valueType,
        uninitializedUses.uninitializedReads);
    diagnoseUninitializedUses<
        Diagnostics::PossiblyUsingUninitializedVariable,
        Diagnostics::PossiblyUsingUninitializedValue>(
        sink,
        variable,
        valueType,
        uninitializedUses.possiblyUninitializedReads);
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
