#include "slang-ir-use-uninitialized-values.h"

#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
static bool isMetaOp(IRInst* inst)
{
    switch (inst->getOp())
    {
    // These instructions only look at the parameter's type,
    // so passing an undefined value to them is permissible
    case kIROp_IsBool:
    case kIROp_IsInt:
    case kIROp_IsUnsignedInt:
    case kIROp_IsSignedInt:
    case kIROp_IsHalf:
    case kIROp_IsFloat:
    case kIROp_IsCoopFloat:
    case kIROp_IsVector:
    case kIROp_GetNaturalStride:
    case kIROp_TypeEquals:
        return true;
    default:
        break;
    }

    return false;
}

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

static List<IRInst*> getAliasableInstructions(IRInst* inst)
{
    List<IRInst*> addresses;

    addresses.add(inst);
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();

        // Meta instructions only use the argument type
        if (isMetaOp(user) || !isAliasable(user))
            continue;

        addresses.addRange(getAliasableInstructions(user));
    }

    return addresses;
}

enum InstructionUsageType
{
    None,        // Instruction neither stores nor loads from the soruce (e.g. meta operations)
    Store,       // Instruction acts as a write to the source
    StoreParent, // Instruction's parent acts as a write to the source
    Load         // Instruciton acts as a load from the source
};

static InstructionUsageType getCallUsageType(IRCall* call, IRInst* inst)
{
    IRInst* callee = call->getCallee();

    // Resolve the actual function
    IRFunc* ftn = nullptr;
    IRFuncType* ftype = nullptr;
    if (auto spec = as<IRSpecialize>(callee))
        ftn = as<IRFunc>(getResolvedInstForDecorations(spec));
    else if (as<IRTranslateBase>(callee))
        return Store;

    else if (auto wit = as<IRLookupWitnessMethod>(callee))
        ftype = as<IRFuncType>(wit->getFullType());
    else
        ftn = as<IRFunc>(callee);

    // Find the argument index so we can fetch the type
    int index = 0;

    auto args = call->getArgsList();
    for (int i = 0; i < args.getCount(); i++)
    {
        if (args[i] == inst)
        {
            index = i;
            break;
        }
    }

    if (ftn)
        ftype = as<IRFuncType>(ftn->getFullType());

    if (!ftype)
        return None;

    // Consider it as a store if its passed
    // as an out/inout/ref parameter
    auto type = unwrapAttributedType(ftype->getParamType(index));
    return (as<IROutParamType>(type) || as<IRBorrowInOutParamType>(type) ||
            as<IRRefParamType>(type))
               ? Store
               : Load;
}

static InstructionUsageType getInstructionUsageType(IRInst* user, IRInst* inst)
{
    // Meta intrinsics (which evaluate on type) do nothing
    if (isMetaOp(user))
        return None;

    // Ignore instructions generating more aliases
    if (isAliasable(user))
        return None;

    switch (user->getOp())
    {
    case kIROp_Loop:
    case kIROp_UnconditionalBranch:
        // TODO: Ignore branches for now
        return None;

    // Debug info instructions should be ignored - they don't constitute
    // actual loads or stores of data, they're just metadata.
    case kIROp_DebugValue:
    case kIROp_DebugVar:
    case kIROp_DebugLine:
    case kIROp_DebugScope:
    case kIROp_DebugInlinedAt:
        return None;

    case kIROp_Call:
        // Function calls can be either
        // stores or loads depending on
        // whether the callee takes it
        // in as a out parameter or not
        return getCallUsageType(as<IRCall>(user), inst);

    // These instructions will store data...
    case kIROp_Store:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
    case kIROp_SPIRVAsm:
    case kIROp_AtomicStore:
        return Store;

    case kIROp_SPIRVAsmOperandInst:
        // For SPIRV asm instructions, need to check out the entire
        // block when doing reachability checks
        return StoreParent;

    case kIROp_MakeExistential:
    case kIROp_MakeExistentialWithRTTI:
        // For specializing generic structs
        return Store;

    // Miscellaenous cases
    case kIROp_ManagedPtrAttach:
    case kIROp_Unmodified:
        return Store;

    default:
        // Default case is that if the instruction is a pointer, it
        // is considered a store, otherwise a load.
        if (as<IRPtrTypeBase>(user->getDataType()))
            return Store;
        return Load;
    }
}

static void collectSpecialCaseInstructions(List<IRInst*>& stores, IRBlock* block)
{
    for (auto inst = block->getFirstInst(); inst; inst = inst->next)
    {
        if (as<IRGenericAsm>(inst))
            stores.add(inst);
    }
}

static void collectInstructionByUsage(
    List<IRInst*>& stores,
    List<IRInst*>& loads,
    IRInst* user,
    IRInst* inst)
{
    InstructionUsageType usage = getInstructionUsageType(user, inst);
    switch (usage)
    {
    case Load:
        return loads.add(user);
    case Store:
        return stores.add(user);
    case StoreParent:
        return stores.add(user->getParent());
    }
}

static void cancelLoads(
    ReachabilityContext& reachability,
    const List<IRInst*>& stores,
    List<IRInst*>& loads)
{
    // Remove all loads which are reachable from stores
    for (auto store : stores)
    {
        for (Index i = 0; i < loads.getCount();)
        {
            if (reachability.isInstReachable(store, loads[i]))
                loads.fastRemoveAt(i);
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

// Remove all loads that are "definitely assigned": every control-flow path from
// the function entry to the load passes through at least one store.
//
// A load is kept (a must-init violation) only when there exists a path from entry
// to the load that does not pass through any store first — i.e. the variable can
// be read while still uninitialized on at least one path.
//
// This is the standard definite-assignment property. We compute it directly with a
// forward CFG walk from entry that is blocked by store-containing blocks, rather
// than using simple dominance. Dominance is too strict: "the load is dominated by
// some single store" misses the common-and-safe case where different paths are
// guarded by different stores, producing false positives on patterns like
// short-circuit `&&` chains (`f(out x) && use(x)`), where the only way to reach the
// use is through the store, but no individual store-block dominates the use-block
// because the join block has a store-free predecessor whose path never reaches the
// use.
static void cancelLoadsByDefiniteAssignment(
    IRGlobalValueWithCode* func,
    const List<IRInst*>& stores,
    List<IRInst*>& loads)
{
    if (loads.getCount() == 0)
        return;

    // Map each store to the block that contains it.
    //
    // `collectInstructionByUsage` records most stores as the storing instruction, but
    // for the `StoreParent` case (e.g. inline SPIRV-asm operands) it records the
    // *containing block* itself as the "store". Such a block is treated as initializing
    // the variable somewhere within it, matching the block-reachability granularity the
    // may-init analysis already uses. We track those separately as `wholeBlockStores`
    // so that every load in them counts as definitely assigned.
    HashSet<IRBlock*> blocksWithStore;
    HashSet<IRBlock*> wholeBlockStores;
    HashSet<IRInst*> storeSet;
    for (auto store : stores)
    {
        if (auto storeBlock = as<IRBlock>(store))
        {
            blocksWithStore.add(storeBlock);
            wholeBlockStores.add(storeBlock);
        }
        else if (auto block = as<IRBlock>(store->getParent()))
        {
            blocksWithStore.add(block);
            storeSet.add(store);
        }
    }

    // For blocks that contain both a store and a load, the relative order matters:
    // a load that appears before any store in the same block is reachable while
    // uninitialized (along the store-free entry path into the block), whereas a load
    // after a store in the same block is definitely assigned by that store.
    //
    // Precompute, for each load, whether a store precedes it within its own block.
    HashSet<IRInst*> loadHasPriorStoreInBlock;
    for (auto load : loads)
    {
        auto block = as<IRBlock>(load->getParent());
        if (!block)
            continue;
        // A whole-block (StoreParent) store covers the entire block, so any load in it
        // is considered definitely assigned.
        if (wholeBlockStores.contains(block))
        {
            loadHasPriorStoreInBlock.add(load);
            continue;
        }
        if (!blocksWithStore.contains(block))
            continue;
        for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (inst == load)
                break;
            if (storeSet.contains(inst))
            {
                loadHasPriorStoreInBlock.add(load);
                break;
            }
        }
    }

    // Loop relaxation: element-wise initialization inside a loop is extremely common
    // (e.g. `[ForceUnroll] for (i) result[i] = ...;` filling an array/vector before
    // use). Such a store does not strictly dominate the post-loop use — the loop could
    // run zero times — but in practice these loops have constant trip counts >= 1, so
    // treating the zero-trip path as leaving the variable uninitialized produces noisy
    // false positives (this is what previously forced large parts of the core module
    // and many tests to disable the warning).
    //
    // To suppress those while still catching genuine first-iteration reads (the #10658
    // motivating bug, where the use appears *inside* the loop before any store), we
    // treat a loop whose body contains a store as initializing the variable by the time
    // control reaches the loop-exit (break) block: that break block is never marked
    // clean-reachable. The break block is the loop's reconvergence point, so every path
    // that reaches it first goes through the loop; excluding it does not hide store-free
    // paths that bypass the loop entirely. Clean state still flows into the loop body, so
    // uses that precede the store inside the body are still reported.
    HashSet<IRBlock*> suppressedBreakBlocks;
    for (auto block : func->getBlocks())
    {
        auto loop = as<IRLoop>(block->getTerminator());
        if (!loop)
            continue;
        auto breakBlock = loop->getBreakBlock();

        // Collect the loop body: blocks reachable from the loop target without passing
        // through the break block. If any of them contains a store, treat the loop as
        // initializing the variable by the time control reaches the break block.
        //
        // This is deliberately conservative toward *fewer* false positives. Loops that
        // fill a variable element-by-element (e.g. `[ForceUnroll] for (i) result[i] =
        // ...;`, including nested loops over compile-time-constant bounds) are extremely
        // common and safe in practice, but the store is not guaranteed to dominate the
        // post-loop use under a purely structural analysis (the loop "might" run zero
        // times, or an inner loop might). Distinguishing those from a genuine
        // conditionally-initialized-in-a-loop bug would require trip-count reasoning, so
        // we accept the rare missed in-loop conditional-store case rather than warn on
        // the pervasive element-wise pattern. Uses that occur *inside* the loop before
        // the store are still reported, since clean state still flows into the body.
        HashSet<IRBlock*> bodyVisited;
        List<IRBlock*> bodyWork;
        bodyVisited.add(breakBlock); // sentinel: never traverse past the break block
        if (auto target = loop->getTargetBlock())
        {
            if (bodyVisited.add(target))
                bodyWork.add(target);
        }
        bool bodyHasStore = false;
        while (bodyWork.getCount())
        {
            auto b = bodyWork.getLast();
            bodyWork.removeLast();
            if (blocksWithStore.contains(b))
            {
                bodyHasStore = true;
                break;
            }
            for (auto succ : b->getSuccessors())
            {
                if (bodyVisited.add(succ))
                    bodyWork.add(succ);
            }
        }
        if (bodyHasStore)
            suppressedBreakBlocks.add(breakBlock);
    }

    // Forward CFG reachability from entry, treating any block that contains a store
    // as a barrier: we can enter such a block "clean" (still uninitialized) but its
    // successors are reached only after the store has executed, so they are not
    // propagated as clean.
    //
    // The set of "clean-reachable" blocks is exactly the set of blocks reachable
    // from entry along a path with no preceding store (subject to the loop relaxation
    // above, and pruning of short-circuit-infeasible edges below).
    //
    // We propagate over CFG edges rather than blocks so that, when entering a block,
    // we know which predecessor we came from and can prune outgoing edges that are
    // statically infeasible due to short-circuit `&&`/`||` constant-phi conditions
    // (see getInfeasibleBranchFromPredecessor). A block is clean-reachable if some
    // clean, feasible edge enters it (the entry block is clean by definition).
    //
    // The worklist holds (predecessor, block) edges. `predecessor` is null for the
    // synthetic entry edge.
    HashSet<IRBlock*> cleanReachable;
    HashSet<KeyValuePair<IRBlock*, IRBlock*>> visitedEdges;
    List<KeyValuePair<IRBlock*, IRBlock*>> worklist;

    auto enqueueEdge = [&](IRBlock* pred, IRBlock* succ)
    {
        // Don't mark a loop's break block clean when the loop body initializes the
        // variable: by the time control reconverges at the break block, the loop has
        // run at least once and performed the store.
        if (suppressedBreakBlocks.contains(succ))
            return;
        KeyValuePair<IRBlock*, IRBlock*> edge(pred, succ);
        if (visitedEdges.add(edge))
        {
            cleanReachable.add(succ);
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

        // A store in this block blocks propagation to its successors.
        if (blocksWithStore.contains(block))
            continue;

        // Prune the outgoing branch that is infeasible given the predecessor we
        // arrived from (short-circuit constant-phi correlation).
        IRBlock* infeasibleSucc = pred ? getInfeasibleBranchFromPredecessor(block, pred) : nullptr;

        for (auto succ : block->getSuccessors())
        {
            if (succ == infeasibleSucc)
                continue;
            enqueueEdge(block, succ);
        }
    }

    // A load is a violation iff its block is clean-reachable and no store precedes
    // it within that block.
    for (Index i = 0; i < loads.getCount();)
    {
        auto block = as<IRBlock>(loads[i]->getParent());
        bool definitelyAssigned = !block || !cleanReachable.contains(block) ||
                                  loadHasPriorStoreInBlock.contains(loads[i]);
        if (definitelyAssigned)
            loads.fastRemoveAt(i);
        else
            i++;
    }
}

static void collectAliasableLoadStores(IRInst* inst, List<IRInst*>& stores, List<IRInst*>& loads)
{
    auto addresses = getAliasableInstructions(inst);

    for (auto alias : addresses)
    {
        // TODO: Mark specific parts assigned to for partial initialization checks
        for (auto use = alias->firstUse; use; use = use->nextUse)
            collectInstructionByUsage(stores, loads, use->getUser(), alias);
    }
}

static List<IRInst*> getUnresolvedParamLoads(
    ReachabilityContext& reachability,
    IRFunc* func,
    IRInst* inst)
{
    // Partition instructions
    List<IRInst*> stores;
    List<IRInst*> loads;

    collectAliasableLoadStores(inst, stores, loads);

    // Special cases for parameters
    for (const auto& b : func->getBlocks())
    {
        collectSpecialCaseInstructions(stores, b);

        auto t = b->getTerminator();
        if (as<IRReturn>(t))
            loads.add(t);
    }

    cancelLoads(reachability, stores, loads);

    return loads;
}

// The two disjoint classes of uninitialized-use violations for a single variable,
// computed from one shared collection pass over its aliasable loads/stores.
struct UninitializedUseLoads
{
    // Loads with NO store reaching them at all (the may-init violations, 41016/41033).
    List<IRInst*> mayInit;

    // Loads that some store reaches (so not may-init) but for which a store-free path
    // from the function entry can still reach the load — i.e. the variable is only
    // conditionally initialized (the must-init / definite-assignment violations,
    // 41035/41036).
    List<IRInst*> mustInit;
};

static UninitializedUseLoads getUninitializedUseLoads(
    ReachabilityContext& reachability,
    IRGlobalValueWithCode* func,
    IRInst* inst)
{
    // Collect the aliasable loads/stores once and derive both violation sets from it.
    List<IRInst*> stores;
    List<IRInst*> allLoads;
    collectAliasableLoadStores(inst, stores, allLoads);

    UninitializedUseLoads result;

    // May-init violations: loads not reachable from any store.
    result.mayInit = allLoads;
    cancelLoads(reachability, stores, result.mayInit);

    // Must-init only adds information when there is at least one store (otherwise every
    // load is already a may-init violation) and at least one load.
    if (stores.getCount() == 0 || allLoads.getCount() == 0)
        return result;

    HashSet<IRInst*> mayInitSet;
    for (auto load : result.mayInit)
        mayInitSet.add(load);

    result.mustInit = allLoads;
    cancelLoadsByDefiniteAssignment(func, stores, result.mustInit);

    // Keep the two sets disjoint: drop loads already reported as may-init violations.
    for (Index i = 0; i < result.mustInit.getCount();)
    {
        if (mayInitSet.contains(result.mustInit[i]))
            result.mustInit.fastRemoveAt(i);
        else
            i++;
    }

    return result;
}

// Emit an uninitialized-use diagnostic at each load location. The named-variable form
// (`TVarDiag`, carrying `varName`) is used when `inst` has a user-visible name; the
// typed-value form (`TValDiag`, carrying `typeName`) is used otherwise (e.g. poison ops
// and other compiler-synthesized intermediates). This is shared by the may-init
// (41016/41033) and must-init (41035/41036) paths, which differ only in the diagnostic
// pair they pass.
template<typename TVarDiag, typename TValDiag>
static void diagnoseUninitializedUses(
    DiagnosticSink* sink,
    IRInst* inst,
    IRType* type,
    const List<IRInst*>& loads)
{
    bool hasName = inst->findDecoration<IRNameHintDecoration>() != nullptr ||
                   inst->findDecoration<IRLinkageDecoration>() != nullptr;

    for (auto load : loads)
    {
        if (hasName)
        {
            StringBuilder varNameSb;
            printDiagnosticArg(varNameSb, inst);
            sink->diagnose(TVarDiag{
                .varName = varNameSb.produceString(),
                .location = load->sourceLoc,
            });
        }
        else
        {
            StringBuilder typeNameSb;
            printDiagnosticArg(typeNameSb, type);
            sink->diagnose(TValDiag{
                .typeName = typeNameSb.produceString(),
                .location = load->sourceLoc,
            });
        }
    }
}

static bool isInstStoredInto(ReachabilityContext& reachability, IRInst* reference, IRInst* inst)
{
    List<IRInst*> stores;
    List<IRInst*> loads;

    for (auto alias : getAliasableInstructions(inst))
    {
        for (auto use = alias->firstUse; use; use = use->nextUse)
            collectInstructionByUsage(stores, loads, use->getUser(), alias);
    }

    for (auto store : stores)
    {
        if (reachability.isInstReachable(store, reference))
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
        InstructionUsageType usage = getInstructionUsageType(use->getUser(), inst);
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
    auto loads = getUnresolvedParamLoads(reachability, func, param);
    for (auto load : loads)
    {
        StringBuilder paramNameSb;
        printDiagnosticArg(paramNameSb, param);
        if (as<IRTerminatorInst>(load))
        {
            sink->diagnose(Diagnostics::ReturningWithUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = load->sourceLoc,
            });
        }
        else
        {
            sink->diagnose(Diagnostics::UsingUninitializedOut{
                .paramName = paramNameSb.produceString(),
                .location = load->sourceLoc,
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

            // Collect both may-init and must-init violations from a single shared
            // load/store collection pass.
            auto useLoads = getUninitializedUseLoads(reachability, func, inst);

            // May-init: the variable is read on a path where no store reaches it at all.
            diagnoseUninitializedUses<
                Diagnostics::UsingUninitializedVariable,
                Diagnostics::UsingUninitializedValue>(sink, inst, type, useLoads.mayInit);

            // Must-init: some store reaches the use, but a store-free path from entry can
            // still reach it — the variable is only conditionally initialized.
            diagnoseUninitializedUses<
                Diagnostics::PossiblyUsingUninitializedVariable,
                Diagnostics::PossiblyUsingUninitializedValue>(sink, inst, type, useLoads.mustInit);
        }
    }

    // Separate analysis for constructors
    checkConstructor(func, reachability, sink);
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

    // Check for initialization blocks
    for (auto inst : variable->getChildren())
    {
        if (as<IRBlock>(inst))
            return;
    }

    auto addresses = getAliasableInstructions(variable);

    List<IRInst*> loads;
    for (auto alias : addresses)
    {
        for (auto use = alias->firstUse; use; use = use->nextUse)
        {
            InstructionUsageType usage = getInstructionUsageType(use->getUser(), alias);
            if (usage == Store || usage == StoreParent)
                return;

            if (usage == Load)
                loads.add(use->getUser());
        }
    }

    for (auto load : loads)
    {
        StringBuilder varNameSb;
        printDiagnosticArg(varNameSb, variable);
        sink->diagnose(Diagnostics::UsingUninitializedGlobalVariable{
            .varName = varNameSb.produceString(),
            .location = load->sourceLoc,
        });
    }
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
