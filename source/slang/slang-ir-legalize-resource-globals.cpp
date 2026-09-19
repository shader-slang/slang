// slang-ir-legalize-resource-globals.cpp
//
// This pass removes source-level per-invocation resource variables from global IR storage.
// Consider this example:
//
//      Texture2D textures[1];
//      static Texture2D texture;
//
//      float4 loadTexture()
//      {
//          return texture.Load(int3(0));
//      }
//
//      float4 main()
//      {
//          texture = textures[0];
//          return loadTexture();
//      }
//
// Although `texture` is declared at file scope, its value belongs to one invocation of `main`.
// Targets that cannot represent a mutable resource value in global storage need the equivalent
// program to use an entry-point local and explicit data flow through the call graph:
//
//      float4 loadTexture(Texture2D texture)
//      {
//          return texture.Load(int3(0));
//      }
//
//      float4 main()
//      {
//          Texture2D texture;
//          texture = textures[0];
//          return loadTexture(texture);
//      }
//
// The pass implements that conceptual rewrite in five phases:
//
// 1. Build a stable, module-ordered inventory of functions, resource globals, and direct calls.
// 2. Reject storage and call boundaries across which hidden per-invocation state cannot be
//    threaded without changing a preserved ABI or storage identity.
// 3. Analyze every derived address and direct call to determine which functions read or write each
//    resource value, and whether a writer replaces the whole value on every return path.
// 4. Materialize the analysis by introducing entry-point locals and helper parameters, replacing
//    direct global uses, and appending the corresponding hidden arguments at calls.
// 5. Check the now-explicit entry-point locals for reads before initialization, then remove the
//    unused global storage.
//
// The front end admits only the supported resource-value categories, optionally wrapped in arrays.
// Earlier target lowering can place such a value in a compiler-generated aggregate (for example,
// append-buffer lowering), so discovery follows aggregate wrappers without broadening the accepted
// resource leaves. Resource-type legalization runs afterward and performs the final decomposition.
//
// Parameter direction follows the source-level effect proved in phase 3. A read-only helper gets a
// value parameter. A helper that replaces the whole value on every normal return gets an `out`
// parameter only when it does not read the incoming value first. Every other writer gets `inout`,
// because some path can preserve or observe the incoming value. Writer bodies use the canonical
// local-mirror representation expected by the downstream resource-output specializer: copy in
// when required, operate on a local, and copy out at each return.
//
// The initializer-moving pass supplies `resourceDependentState`, a conservative boundary-
// validation set. It contains the resource globals, every initializer target that transitively
// depends on them, and ordinary state that the moved initializer call graph may mutate. This wider
// set is used only to validate storage and invocation boundaries. `resourceGlobals` below is the
// narrower set of resource-typed globals that this pass actually rewrites.
#include "slang-ir-legalize-resource-globals.h"

#include "slang-diagnostics.h"
#include "slang-ir-dominators.h"
#include "slang-ir-explicit-global-init.h"
#include "slang-ir-insts.h"
#include "slang-ir-use-uninitialized-values.h"
#include "slang-ir-util.h"

namespace Slang
{

namespace
{

/// Summarizes whether a function may read or write one resource value.
///
/// These flags describe possible effects and are propagated transitively from callees to callers.
/// Definite whole-value replacement is tracked separately because `Write` alone does not say
/// whether every control-flow path performs a complete assignment.
enum class ResourceStateAccess : UInt
{
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
};

/// Records the module-level facts and cached CFG analysis for one function.
///
/// Functions retain their module order so that hidden parameters are deterministic. The original
/// body position lets phase 4 insert locals after parameters but before any pre-existing code.
struct FunctionContext
{
    /// The function represented by this record.
    IRFunc* func = nullptr;

    /// The stable insertion anchor captured before phase 4 adds any entry code.
    IRInst* originalFirstOrdinaryInst = nullptr;

    /// The lazily computed CFG dominator tree shared by every per-resource proof in this function.
    RefPtr<IRDominatorTree> dominatorTree;

    /// Whether this function creates fresh per-invocation state.
    bool isEntryPoint = false;

    /// Whether at least one invocation has no direct call site at which to pass hidden state.
    bool isIndependentRoot = false;
};

/// Records the analysis and rewrite decisions for one function/resource-global pair.
struct FunctionResourceStateInfo
{
    /// The possible semantic read/write effects, including effects propagated from callees.
    ResourceStateAccess access = ResourceStateAccess::None;

    /// Whether every reachable normal return follows a complete assignment of this value.
    bool replacesWholeValueOnEveryReturn = false;

    /// Whether some execution path observes the value supplied by the caller before replacing it.
    bool semanticallyReadsIncomingValue = false;

    /// Whether a partial or conditional writer must transport an otherwise-unobserved input value.
    bool preservesIncomingValue = false;

    /// Whether phase 4 must redirect an original use in this body, including non-semantic metadata.
    bool hasDirectUse = false;

    /// The local address that replaces direct uses of the global in this function.
    IRInst* replacementAddress = nullptr;

    /// The hidden helper parameter, or null when this function owns a local without a parameter.
    IRParam* parameter = nullptr;

    /// Exact source-level effects retained for the generated local's definite-assignment check.
    List<UninitializedVariableUseEffect> uninitializedUseEffects;

    /// Return whether the helper's local must start with the value supplied by its caller.
    bool mustSeedLocalFromCaller() const
    {
        return semanticallyReadsIncomingValue || preservesIncomingValue;
    }
};

/// Identifies a direct use of the global and the function body that contains it, if any.
struct DirectGlobalUse
{
    IRUse* use = nullptr;
    Index functionIndex = -1;
};

/// Describes a semantic terminal use at the end of a derived-address chain.
struct ResourceAddressUse
{
    /// The operand use through which the terminal instruction accesses the address.
    IRUse* use = nullptr;

    /// The containing function's stable module-order index.
    Index functionIndex = -1;

    /// The possible read/write effect of this terminal instruction.
    ResourceStateAccess access = ResourceStateAccess::None;

    /// Whether this instruction completely replaces the root value, not merely a subobject.
    bool replacesWholeValue = false;
};

/// Collects the analysis and rewrite state for one resource global.
struct ResourceGlobalToRewrite
{
    /// The original storage whose uses phase 4 will replace and whose declaration phase 5 removes.
    IRGlobalVar* globalVar = nullptr;

    /// The value type stored by `globalVar`, cached before rewriting changes its uses.
    IRType* valueType = nullptr;

    /// One analysis/rewrite record per function in the module inventory.
    List<FunctionResourceStateInfo> functions;

    /// Every direct use-list edge from the global, saved before phase 4 mutates the use list.
    List<DirectGlobalUse> rootUses;

    /// Semantic reads and writes reached by following each root through derived addresses.
    List<ResourceAddressUse> terminalUses;

    /// Call-argument address uses retained for alias validation before rewriting.
    List<IRUse*> addressPassingUses;

    /// Uses that let the address outlive the call/body contract and therefore cannot be rewritten.
    List<IRUse*> addressEscapes;
};

/// Records one direct call whose caller and callee both belong to the module inventory.
struct DirectCallEdge
{
    IRCall* call = nullptr;
    Index callerIndex = -1;
    Index calleeIndex = -1;
};

// The pass is presented before these low-level IR classifiers; their definitions follow it.
static ResourceStateAccess mergeAccess(ResourceStateAccess left, ResourceStateAccess right);
static bool hasAccess(ResourceStateAccess value, ResourceStateAccess test);
static bool doesInstSemanticallyUseOperandValue(IRInst* user);
static bool doesUseDeriveAddress(IRUse* use);
static IRInst* findCallArgumentParameterType(IRCall* call, IRUse* use);
static ResourceStateAccess classifyCallArgumentAccess(IRCall* call, IRUse* use);
static ResourceStateAccess classifyResourceAddressUse(IRUse* use);
static bool doesUseEscapeAddress(IRUse* use);
static bool hasInvocationWithoutRewritableCallSite(IRFunc* func);
static bool isIndependentlyInvokedFunction(IRFunc* func);
static bool requiresPreservedGlobalStorage(IRGlobalVar* globalVar);

/// Implements the five-phase transformation described at the top of this file.
///
/// The pass keeps analysis records separate from the IR until phase 4. That separation lets the
/// validation phases reject an unsupported program without leaving a partially rewritten module.
struct LegalizeResourceGlobalVarsPass
{
    IRModule* module = nullptr;
    List<FunctionContext> functions;
    Dictionary<IRFunc*, Index> functionIndices;
    List<ResourceGlobalToRewrite> resourceGlobals;
    List<DirectCallEdge> directCallEdges;

    explicit LegalizeResourceGlobalVarsPass(IRModule* inModule)
        : module(inModule)
    {
    }

    void processModule(List<IRGlobalVar*> const& resourceDependentState, DiagnosticSink* sink)
    {
        // Phase 1: Inventory the module. Every later phase refers to functions, globals, and calls
        // by stable module-order indices, so no analysis depends on hash-table or use-list order.
        collectFunctions();
        collectResourceGlobals();
        collectDirectCalls();
        assertResourceGlobalsAreResourceDependent(resourceDependentState);

        // Phase 2: Prove that resource-dependent state can be localized and supplied at every
        // invocation. Check storage identity and invocation roots separately so the reason each
        // boundary is unsupported remains explicit in this top-level decomposition.
        //
        // TODO: Move these semantic boundary checks into a target-independent post-link validation
        // stage if the pipeline gains one. Mandatory per-module checking does not yet have the
        // fully linked call graph needed to identify every independently invoked function.
        bool diagnosedUnsupportedBoundary =
            diagnosePreservedResourceDependentStorage(resourceDependentState, sink);
        diagnosedUnsupportedBoundary |=
            diagnoseUnsupportedInvocationRoots(resourceDependentState, sink);
        if (diagnosedUnsupportedBoundary)
            return;

        if (resourceGlobals.getCount() == 0)
            return;

        // Phase 3: Determine the effect of each function on each resource value. The result tells
        // phase 4 whether a helper needs a value, `out`, or `inout` parameter and tells phase 5
        // which generated operations are possible versus definite initializations.
        analyzeResourceStateFlow();
        recordMayWriteEffectsForDefiniteAssignment();
        if (diagnoseAddressEscapes(sink))
            return;
        if (diagnoseAliasingCallBoundaries(sink))
            return;
        assertAllStatefulFunctionsHaveRewritableUses();

        // Phase 4: Give every function its own representation of the value, redirect the old
        // global uses, and make the previously implicit data flow explicit at direct calls.
        introduceReplacements();
        replaceGlobalUses();
        rewriteCalls();

        // Phase 5: Once entry-point state is explicit, apply the shared definite-assignment solver
        // to it. Successful rewriting leaves the original globals unused and safe to delete.
        diagnoseUninitializedEntryPointReads(sink);
        removeReplacedResourceGlobals();
    }

    // ## Phase 1: Build the stable module inventory

    /// Collect functions and the facts that determine where hidden state may enter the call graph.
    ///
    /// Entry points create fresh state for one invocation. Every other independently invoked
    /// function is a boundary because at least one invocation has no rewritable call site through
    /// which phase 4 can supply that state.
    void collectFunctions()
    {
        for (auto inst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            FunctionContext context;
            context.func = func;
            context.isEntryPoint = func->findDecoration<IREntryPointDecoration>() != nullptr;
            context.isIndependentRoot =
                !context.isEntryPoint && isIndependentlyInvokedFunction(func);
            if (auto firstBlock = func->getFirstBlock())
                context.originalFirstOrdinaryInst = firstBlock->getFirstOrdinaryInst();

            auto index = functions.getCount();
            functions.add(context);
            functionIndices.add(func, index);
        }
    }

    /// Collect exactly the global variables whose resource values phase 4 must localize.
    ///
    /// The initializer mover has already removed each selected initializer body. Keeping these
    /// records in module order makes the order of hidden parameters deterministic.
    void collectResourceGlobals()
    {
        for (auto inst : module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !isPerInvocationResourceStateGlobalVar(globalVar))
                continue;

            auto ptrType = cast<IRPtrTypeBase>(globalVar->getDataType());
            SLANG_RELEASE_ASSERT(!globalVar->getFirstBlock());

            ResourceGlobalToRewrite resourceGlobal;
            resourceGlobal.globalVar = globalVar;
            resourceGlobal.valueType = ptrType->getValueType();
            for (Index i = 0; i < functions.getCount(); ++i)
                resourceGlobal.functions.add(FunctionResourceStateInfo());
            resourceGlobals.add(_Move(resourceGlobal));
        }
    }

    /// Collect direct calls whose caller and callee both belong to the module inventory.
    ///
    /// These edges are the only invocation boundaries phase 4 can rewrite. They also provide the
    /// graph over which phases 2 and 3 propagate state requirements back toward entry points.
    void collectDirectCalls()
    {
        for (Index callerIndex = 0; callerIndex < functions.getCount(); ++callerIndex)
        {
            auto func = functions[callerIndex].func;
            if (!func->getFirstBlock())
                continue;

            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    auto call = as<IRCall>(inst);
                    if (!call)
                        continue;

                    auto callee = as<IRFunc>(call->getCallee());
                    auto calleeIndex = callee ? functionIndices.tryGetValue(callee) : nullptr;
                    if (!calleeIndex)
                        continue;

                    directCallEdges.add(DirectCallEdge{call, callerIndex, *calleeIndex});
                }
            }
        }
    }

    /// Assert the contract between initializer movement and resource-state legalization.
    ///
    /// Every resource global rewritten here must be present in the mover's wider state set. If it
    /// is absent, phase 2 could miss a preserved boundary that observes the localized value.
    void assertResourceGlobalsAreResourceDependent(List<IRGlobalVar*> const& resourceDependentState)
    {
        HashSet<IRGlobalVar*> resourceDependentStateSet;
        for (auto stateGlobal : resourceDependentState)
            resourceDependentStateSet.add(stateGlobal);
        for (auto const& resourceGlobal : resourceGlobals)
            SLANG_RELEASE_ASSERT(resourceDependentStateSet.contains(resourceGlobal.globalVar));
    }

    // ## Phase 2: Validate storage and invocation boundaries

    /// Diagnose resource-dependent state whose storage must remain externally observable.
    ///
    /// Localizing such a variable independently in each entry point would change its identity and
    /// lifetime. Diagnose the storage declaration itself; other boundary checks skip this state
    /// because there is no valid localization to thread through the call graph.
    bool diagnosePreservedResourceDependentStorage(
        List<IRGlobalVar*> const& resourceDependentState,
        DiagnosticSink* sink)
    {
        bool diagnosed = false;
        for (auto stateGlobal : resourceDependentState)
        {
            if (!requiresPreservedGlobalStorage(stateGlobal))
                continue;
            sink->diagnose(Diagnostics::ResourceDependentStateHasPreservedStorage{
                .variable = stateGlobal,
                .location = stateGlobal->sourceLoc});
            diagnosed = true;
        }
        return diagnosed;
    }

    /// Find every function that directly or transitively observes localizable dependent state.
    ///
    /// Direct global uses seed the set. Repeatedly adding callers computes the reverse-call-graph
    /// closure, including recursion, so phase 2 also checks roots that reach state through helpers.
    List<bool> findFunctionsAccessingResourceDependentState(
        List<IRGlobalVar*> const& resourceDependentState)
    {
        List<bool> accessesState;
        for (Index i = 0; i < functions.getCount(); ++i)
            accessesState.add(false);

        for (auto stateGlobal : resourceDependentState)
        {
            if (requiresPreservedGlobalStorage(stateGlobal))
                continue;
            for (auto use = stateGlobal->firstUse; use; use = use->nextUse)
            {
                if (!doesInstSemanticallyUseOperandValue(use->getUser()))
                    continue;
                auto parentFunc = getParentFunc(use->getUser());
                auto functionIndex = parentFunc ? functionIndices.tryGetValue(parentFunc) : nullptr;
                if (functionIndex)
                    accessesState[*functionIndex] = true;
            }
        }

        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                if (!accessesState[edge.calleeIndex] || accessesState[edge.callerIndex])
                    continue;
                accessesState[edge.callerIndex] = true;
                changed = true;
            }
        } while (changed);
        return accessesState;
    }

    /// Diagnose call roots that cannot receive per-invocation state from an in-module caller.
    ///
    /// A callable entry point would need both a fresh-state entry wrapper and an ordinary helper
    /// that accepts its caller's state. Other independent roots would need an analogous wrapper to
    /// preserve their external signature. The current IR has neither representation at this point.
    bool diagnoseUnsupportedInvocationRoots(
        List<IRGlobalVar*> const& resourceDependentState,
        DiagnosticSink* sink)
    {
        bool diagnosed = false;
        auto accessesState = findFunctionsAccessingResourceDependentState(resourceDependentState);

        List<bool> hasOrdinaryCallSite;
        for (Index i = 0; i < functions.getCount(); ++i)
            hasOrdinaryCallSite.add(false);
        for (auto const& edge : directCallEdges)
            hasOrdinaryCallSite[edge.calleeIndex] = true;

        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            if (!accessesState[functionIndex])
                continue;

            auto const& function = functions[functionIndex];
            if (function.isEntryPoint && hasOrdinaryCallSite[functionIndex])
            {
                sink->diagnose(Diagnostics::ResourceDependentStaticUsedByCallableEntryPoint{
                    .function = function.func,
                    .location = function.func->sourceLoc});
                diagnosed = true;
            }
            else if (function.isIndependentRoot)
            {
                sink->diagnose(Diagnostics::ResourceDependentStaticUsedByPreservedFunction{
                    .function = function.func,
                    .location = function.func->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    // ## Phase 3: Analyze resource-state flow and validate its representation

    /// Analyze how every function reads, writes, and replaces every resource global.
    ///
    /// For each global, first classify direct and derived-address uses, then propagate possible
    /// effects through callers. Finally prove which writers replace the whole value on all returns
    /// and which reads or partial writers therefore need the caller's incoming value.
    void analyzeResourceStateFlow()
    {
        for (auto& resourceGlobal : resourceGlobals)
        {
            collectRootUses(resourceGlobal);

            HashSet<IRInst*> visitedAddresses;
            analyzeDerivedAddressUses(
                resourceGlobal,
                resourceGlobal.globalVar,
                visitedAddresses,
                true);

            propagateAccessToCallers(resourceGlobal);
            findWholeValueReplacementsOnEveryReturn(resourceGlobal);
            findIncomingValueRequirements(resourceGlobal);
        }
    }

    /// Save every direct use of the global before phase 4 mutates any use lists.
    ///
    /// Function-local uses establish where replacement storage is required. Module-scope metadata
    /// is retained only so `replaceGlobalUses` can remove it safely with the obsolete global.
    void collectRootUses(ResourceGlobalToRewrite& global)
    {
        for (auto use = global.globalVar->firstUse; use; use = use->nextUse)
        {
            DirectGlobalUse rootUse;
            rootUse.use = use;

            if (auto parentFunc = getParentFunc(use->getUser()))
            {
                if (auto index = functionIndices.tryGetValue(parentFunc))
                {
                    rootUse.functionIndex = *index;
                    global.functions[*index].hasDirectUse = true;
                }
            }

            global.rootUses.add(rootUse);
        }
    }

    /// Walk the address graph rooted at one resource global and classify its terminal uses.
    ///
    /// Address derivations recurse while carrying whether the address still denotes the whole
    /// value. Terminal loads, stores, and calls contribute effects to their containing function;
    /// escaping and explicitly passed addresses are saved for validation later in phase 3.
    void analyzeDerivedAddressUses(
        ResourceGlobalToRewrite& global,
        IRInst* address,
        HashSet<IRInst*>& visitedAddresses,
        bool representsWholeValue)
    {
        if (!visitedAddresses.add(address))
            return;

        for (auto use = address->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (!doesInstSemanticallyUseOperandValue(user))
                continue;

            if (doesUseDeriveAddress(use))
            {
                analyzeDerivedAddressUses(global, user, visitedAddresses, false);
                continue;
            }

            if (as<IRCall>(user))
                global.addressPassingUses.add(use);
            if (doesUseEscapeAddress(use))
                global.addressEscapes.add(use);

            auto parentFunc = getParentFunc(user);
            SLANG_RELEASE_ASSERT(parentFunc);
            auto functionIndex = functionIndices.tryGetValue(parentFunc);
            SLANG_RELEASE_ASSERT(functionIndex);

            auto terminalAccess = classifyResourceAddressUse(use);
            auto& access = global.functions[*functionIndex].access;
            access = mergeAccess(access, terminalAccess);

            bool replacesWholeValue = false;
            if (representsWholeValue)
            {
                if (auto store = as<IRStore>(user))
                    replacesWholeValue = store->ptr.get() == address;
                else if (as<IRCall>(user))
                    replacesWholeValue = terminalAccess == ResourceStateAccess::Write;
            }
            global.terminalUses.add(
                ResourceAddressUse{use, *functionIndex, terminalAccess, replacesWholeValue});
        }
    }

    /// Propagate possible read/write effects from callees to callers.
    ///
    /// Iterating direct calls to a fixed point handles ordinary chains and recursive strongly
    /// connected components without relying on a particular function order.
    void propagateAccessToCallers(ResourceGlobalToRewrite& global)
    {
        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                auto calleeAccess = global.functions[edge.calleeIndex].access;
                auto& callerAccess = global.functions[edge.callerIndex].access;
                auto merged = mergeAccess(callerAccess, calleeAccess);
                if (merged != callerAccess)
                {
                    callerAccess = merged;
                    changed = true;
                }
            }
        } while (changed);
    }

    /// Compute which writers satisfy an `out`-parameter contract.
    ///
    /// Callee proofs create barriers in callers, so repeat the per-function proof to a fixed point.
    /// Starting from false keeps a recursive cycle conservative unless some member has its own
    /// writes that cover every reachable return path.
    void findWholeValueReplacementsOnEveryReturn(ResourceGlobalToRewrite& global)
    {
        bool changed = false;
        do
        {
            changed = false;
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto& function = global.functions[functionIndex];
                if (!hasAccess(function.access, ResourceStateAccess::Write) ||
                    function.replacesWholeValueOnEveryReturn)
                    continue;

                if (doesFunctionReplaceWholeValueOnEveryReturn(global, functionIndex))
                {
                    function.replacesWholeValueOnEveryReturn = true;
                    changed = true;
                }
            }
        } while (changed);
    }

    /// Prove that every reachable normal return follows a complete replacement of the value.
    ///
    /// A function with no replacement, or no reachable normal return, does not establish an `out`
    /// contract. Otherwise a return disproves the contract if it is reachable before a barrier.
    bool doesFunctionReplaceWholeValueOnEveryReturn(
        ResourceGlobalToRewrite& global,
        Index functionIndex)
    {
        HashSet<IRInst*> replacements;
        collectWholeValueReplacementBarriers(global, functionIndex, replacements);
        if (replacements.getCount() == 0)
            return false;

        bool foundReachableReturn = false;
        auto dominatorTree = getDominatorTree(functionIndex);
        for (auto block : functions[functionIndex].func->getBlocks())
        {
            if (dominatorTree->isUnreachable(block))
                continue;

            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            foundReachableReturn = true;
            if (canReachInstructionBeforeWholeValueReplacement(
                    functionIndex,
                    returnInst,
                    replacements))
                return false;
        }

        return foundReachableReturn;
    }

    /// Collect instructions after which the current function has a complete replacement value.
    ///
    /// Direct whole-value stores are barriers immediately. Calls become barriers only after the
    /// fixed-point analysis proves that their callee replaces the value on every normal return.
    void collectWholeValueReplacementBarriers(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        HashSet<IRInst*>& replacements)
    {
        for (auto const& terminalUse : global.terminalUses)
        {
            if (terminalUse.functionIndex == functionIndex && terminalUse.replacesWholeValue)
                replacements.add(terminalUse.use->getUser());
        }

        for (auto const& edge : directCallEdges)
        {
            if (edge.callerIndex == functionIndex &&
                global.functions[edge.calleeIndex].replacesWholeValueOnEveryReturn)
            {
                replacements.add(edge.call);
            }
        }
    }

    /// Return the dominator tree used to exclude unreachable returns from all-path proofs.
    ///
    /// Each resource value runs the same CFG queries. Compute the tree lazily on the first query
    /// and retain it in the function record so later resources reuse the same analysis.
    IRDominatorTree* getDominatorTree(Index functionIndex)
    {
        auto& function = functions[functionIndex];
        if (!function.dominatorTree)
            function.dominatorTree = computeDominatorTree(function.func);
        return function.dominatorTree;
    }

    /// Return whether control can reach `target` before any complete replacement barrier.
    ///
    /// The worklist carries exactly one state: the resource still has its incoming value. A
    /// replacement ends that path, while reaching the target first proves the incoming value is
    /// observable there. Each block therefore needs to be visited only once.
    bool canReachInstructionBeforeWholeValueReplacement(
        Index functionIndex,
        IRInst* target,
        HashSet<IRInst*> const& replacements)
    {
        HashSet<IRBlock*> visited;
        List<IRBlock*> workList;
        auto entryBlock = functions[functionIndex].func->getFirstBlock();
        if (!entryBlock)
            return false;
        visited.add(entryBlock);
        workList.add(entryBlock);

        while (workList.getCount())
        {
            auto block = workList.getLast();
            workList.removeLast();

            bool pathWasReplaced = false;
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                // A call can both read the incoming value and replace it before returning. Test
                // the target first so its own replacement does not hide that read.
                if (inst == target)
                    return true;
                if (replacements.contains(inst))
                {
                    pathWasReplaced = true;
                    break;
                }
            }
            if (pathWasReplaced)
                continue;

            for (auto successor : block->getSuccessors())
            {
                if (visited.add(successor))
                    workList.add(successor);
            }
        }

        return false;
    }

    /// Compute why each function needs the caller's incoming resource value.
    ///
    /// First mark partial or conditional writers that must preserve an incoming value even if they
    /// never read it semantically. Next classify direct reads by whether a replacement dominates
    /// them. Finally propagate callee read requirements to callers, stopping when an earlier
    /// replacement supplies the value instead.
    void findIncomingValueRequirements(ResourceGlobalToRewrite& global)
    {
        for (auto& function : global.functions)
        {
            if (hasAccess(function.access, ResourceStateAccess::Write) &&
                !function.replacesWholeValueOnEveryReturn)
            {
                function.preservesIncomingValue = true;
            }
        }

        for (auto const& terminalUse : global.terminalUses)
        {
            if (!hasAccess(terminalUse.access, ResourceStateAccess::Read))
                continue;
            if (!hasPriorWholeValueReplacement(
                    global,
                    terminalUse.functionIndex,
                    terminalUse.use->getUser()))
            {
                global.functions[terminalUse.functionIndex].semanticallyReadsIncomingValue = true;
            }
        }

        bool changed = false;
        do
        {
            changed = false;
            for (auto const& edge : directCallEdges)
            {
                if (!global.functions[edge.calleeIndex].semanticallyReadsIncomingValue)
                    continue;
                auto& caller = global.functions[edge.callerIndex];
                if (caller.semanticallyReadsIncomingValue ||
                    hasPriorWholeValueReplacement(global, edge.callerIndex, edge.call))
                {
                    continue;
                }
                caller.semanticallyReadsIncomingValue = true;
                changed = true;
            }
        } while (changed);
    }

    /// Return whether every path to `read` has already replaced the incoming value.
    ///
    /// Collect the current direct and callee-proven barriers, then negate the complementary
    /// reachability question: whether any path can reach the read before crossing one of them.
    bool hasPriorWholeValueReplacement(
        ResourceGlobalToRewrite& global,
        Index functionIndex,
        IRInst* read)
    {
        HashSet<IRInst*> replacements;
        collectWholeValueReplacementBarriers(global, functionIndex, replacements);
        return replacements.getCount() != 0 &&
               !canReachInstructionBeforeWholeValueReplacement(functionIndex, read, replacements);
    }

    /// Record writes whose generated `out`/`inout` type would overstate definite assignment.
    ///
    /// A subobject store or an `inout` call may write the value but does not initialize the whole
    /// value on every path. Preserve that semantic distinction for the phase-5 CFG solver.
    void recordMayWriteEffectsForDefiniteAssignment()
    {
        for (auto& global : resourceGlobals)
        {
            for (auto const& terminalUse : global.terminalUses)
            {
                if (!hasAccess(terminalUse.access, ResourceStateAccess::Write) ||
                    terminalUse.replacesWholeValue)
                {
                    continue;
                }

                auto& stateInfo = global.functions[terminalUse.functionIndex];
                stateInfo.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                    .use = terminalUse.use,
                    .readsValue = hasAccess(terminalUse.access, ResourceStateAccess::Read),
                    .mayWriteValue = true,
                    .definitelyWritesValue = false,
                });
            }
        }
    }

    /// Diagnose uses that let a resource global's address escape the analyzable address graph.
    ///
    /// Phase 4 creates separate locals and parameters. An escaped address could retain or compare
    /// the old global identity, so rewriting it without an explicit storage model would be unsound.
    bool diagnoseAddressEscapes(DiagnosticSink* sink)
    {
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            for (auto use : resourceGlobal.addressEscapes)
            {
                sink->diagnose(Diagnostics::ResourceStaticAddressEscapes{
                    .variable = resourceGlobal.globalVar,
                    .location = use->getUser()->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Diagnose an explicit argument that aliases state also threaded implicitly to the callee.
    ///
    /// After localization, the explicit argument and hidden argument could name different local
    /// copies of what was one global. Reject that call until the transform can preserve one shared
    /// storage identity.
    bool diagnoseAliasingCallBoundaries(DiagnosticSink* sink)
    {
        bool diagnosed = false;
        for (auto const& resourceGlobal : resourceGlobals)
        {
            HashSet<IRCall*> diagnosedCalls;
            for (auto use : resourceGlobal.addressPassingUses)
            {
                auto call = cast<IRCall>(use->getUser());
                auto callee = as<IRFunc>(call->getCallee());
                auto calleeIndex = callee ? functionIndices.tryGetValue(callee) : nullptr;
                if (!calleeIndex)
                    continue;

                auto implicitAccess = resourceGlobal.functions[*calleeIndex].access;
                if (implicitAccess == ResourceStateAccess::None || !diagnosedCalls.add(call))
                    continue;

                sink->diagnose(Diagnostics::ResourceStaticAliasesThreadedState{
                    .variable = resourceGlobal.globalVar,
                    .function = callee,
                    .location = call->sourceLoc});
                diagnosed = true;
            }
        }
        return diagnosed;
    }

    /// Assert that every function receiving hidden state has only rewritable invocation uses.
    ///
    /// Phase 2 diagnoses preserved roots. This assertion protects the remaining pipeline contract:
    /// no witness table, specialization, or function-pointer use may be silently left with the old
    /// signature after phase 4 adds parameters.
    void assertAllStatefulFunctionsHaveRewritableUses()
    {
        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            auto func = functions[functionIndex].func;
            if (functions[functionIndex].isEntryPoint)
                continue;

            bool needsParameter = false;
            for (auto const& global : resourceGlobals)
            {
                needsParameter |=
                    global.functions[functionIndex].access != ResourceStateAccess::None;
            }
            if (!needsParameter)
                continue;

            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (!doesInstSemanticallyUseOperandValue(user))
                    continue;

                auto call = as<IRCall>(user);
                SLANG_RELEASE_ASSERT(call && call->getCalleeUse() == use);
                SLANG_RELEASE_ASSERT(getParentFunc(call));
            }
        }
    }

    // ## Phase 4: Materialize locals, parameters, uses, and calls

    /// Materialize the per-function representations selected by phase-3 analysis.
    ///
    /// Entry points and metadata-only users receive locals. Helpers with semantic access receive a
    /// hidden parameter plus a local mirror. After all parameters are present, rebuild affected
    /// function and debug signatures once per function.
    void introduceReplacements()
    {
        IRBuilder builder(module);

        for (auto& global : resourceGlobals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                auto const& function = functions[functionIndex];
                auto& stateInfo = global.functions[functionIndex];
                if (stateInfo.access == ResourceStateAccess::None && !stateInfo.hasDirectUse)
                    continue;

                if (function.isEntryPoint || stateInfo.access == ResourceStateAccess::None)
                    createLocalReplacement(builder, functionIndex, global, stateInfo);
                else
                    createThreadedHelperReplacement(builder, functionIndex, global, stateInfo);
            }
        }

        for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
        {
            bool changed = false;
            for (auto const& global : resourceGlobals)
                changed |= global.functions[functionIndex].parameter != nullptr;
            if (changed)
            {
                fixUpFuncType(functions[functionIndex].func);
                fixUpDebugFuncType(functions[functionIndex].func);
            }
        }
    }

    /// Choose the hidden parameter type that preserves the analyzed source-level effect.
    ///
    /// Read-only state travels by value. A writer proven to replace the whole value on every
    /// return can use `out`, unless it reads the incoming value first. Every remaining writer uses
    /// `inout` because some execution path must retain the caller's value.
    IRType* chooseThreadedParameterType(
        IRBuilder& builder,
        ResourceGlobalToRewrite const& global,
        FunctionResourceStateInfo const& stateInfo)
    {
        bool writesValue = hasAccess(stateInfo.access, ResourceStateAccess::Write);
        if (writesValue && !stateInfo.semanticallyReadsIncomingValue &&
            stateInfo.replacesWholeValueOnEveryReturn)
        {
            return builder.getOutParamType(global.valueType);
        }
        if (writesValue)
            return builder.getBorrowInOutParamType(global.valueType);
        return global.valueType;
    }

    /// Insert replacement storage at the start of a function's original body.
    ///
    /// Entry points own fresh state for each invocation. A non-semantic direct use, such as debug
    /// metadata, also needs a local address but does not justify adding a parameter.
    void createLocalReplacement(
        IRBuilder& builder,
        Index functionIndex,
        ResourceGlobalToRewrite const& global,
        FunctionResourceStateInfo& stateInfo)
    {
        setInsertAtOriginalBodyStart(builder, functionIndex);
        stateInfo.replacementAddress = builder.emitVar(global.valueType);
        stateInfo.replacementAddress->sourceLoc = global.globalVar->sourceLoc;
        copyNameHint(builder, global.globalVar, stateInfo.replacementAddress);
    }

    /// Add a hidden parameter and canonical local mirror for one helper's resource state.
    ///
    /// Downstream resource-output specialization expects bodies to operate on locals and normal
    /// returns to copy writer locals back to `out`/`inout` parameters. Seed the local only when the
    /// helper reads or preserves the incoming value; a true `out` helper starts uninitialized.
    void createThreadedHelperReplacement(
        IRBuilder& builder,
        Index functionIndex,
        ResourceGlobalToRewrite const& global,
        FunctionResourceStateInfo& stateInfo)
    {
        auto const& function = functions[functionIndex];
        auto firstBlock = function.func->getFirstBlock();
        SLANG_RELEASE_ASSERT(firstBlock);

        auto paramType = chooseThreadedParameterType(builder, global, stateInfo);
        stateInfo.parameter = builder.createParam(paramType);
        stateInfo.parameter->sourceLoc = global.globalVar->sourceLoc;
        firstBlock->addParam(stateInfo.parameter);
        copyNameHint(builder, global.globalVar, stateInfo.parameter);

        createLocalReplacement(builder, functionIndex, global, stateInfo);

        bool writesValue = hasAccess(stateInfo.access, ResourceStateAccess::Write);
        if (!writesValue || stateInfo.mustSeedLocalFromCaller())
        {
            auto inputValue = !writesValue
                                  ? static_cast<IRInst*>(stateInfo.parameter)
                                  : builder.emitLoad(global.valueType, stateInfo.parameter);
            builder.emitStore(stateInfo.replacementAddress, inputValue);
        }

        if (!writesValue)
            return;
        for (auto block : function.func->getBlocks())
        {
            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            builder.setInsertBefore(returnInst);
            auto result = builder.emitLoad(global.valueType, stateInfo.replacementAddress);
            builder.emitStore(stateInfo.parameter, result);
        }
    }

    /// Position generated entry code before the first instruction captured during phase 1.
    ///
    /// Phase 4 inserts several locals and copy-ins into the same block. Reusing the pre-mutation
    /// anchor keeps all generated entry code ahead of the original body as those insertions build
    /// up.
    void setInsertAtOriginalBodyStart(IRBuilder& builder, Index functionIndex)
    {
        auto const& function = functions[functionIndex];
        if (function.originalFirstOrdinaryInst)
            builder.setInsertBefore(function.originalFirstOrdinaryInst);
        else
            builder.setInsertInto(function.func->getFirstBlock());
    }

    /// Preserve a source variable's user-facing name on its generated local or parameter.
    void copyNameHint(IRBuilder& builder, IRInst* source, IRInst* target)
    {
        if (auto nameHint = source->findDecoration<IRNameHintDecoration>())
            builder.addNameHintDecoration(target, nameHint->getName());
    }

    /// Redirect saved function-local uses to their replacement address and remove stale metadata.
    ///
    /// Metadata deletion is deferred because one instruction may mention multiple globals; deleting
    /// it while another global still holds an `IRUse*` into it would invalidate that saved use.
    void replaceGlobalUses()
    {
        List<IRInst*> moduleMetadataUsers;
        HashSet<IRInst*> seenModuleMetadataUsers;

        for (auto& global : resourceGlobals)
        {
            for (auto const& rootUse : global.rootUses)
            {
                if (rootUse.functionIndex < 0)
                {
                    // Decorations and other module metadata disappear with the storage they name.
                    auto user = rootUse.use->getUser();
                    SLANG_RELEASE_ASSERT(!doesInstSemanticallyUseOperandValue(user));
                    if (user->getParent() != global.globalVar && seenModuleMetadataUsers.add(user))
                    {
                        moduleMetadataUsers.add(user);
                    }
                    continue;
                }

                auto replacement = global.functions[rootUse.functionIndex].replacementAddress;
                SLANG_RELEASE_ASSERT(replacement);
                rootUse.use->set(replacement);
            }
        }

        for (auto user : moduleMetadataUsers)
            user->removeAndDeallocate();
    }

    /// Replace direct calls whose callees gained hidden resource-state parameters.
    ///
    /// Copy explicit arguments first, append hidden arguments in deterministic order, and preserve
    /// source locations, decorations, and saved semantic effects before deleting the old call.
    void rewriteCalls()
    {
        IRBuilder builder(module);

        for (auto const& edge : directCallEdges)
        {
            if (!callNeedsHiddenStateArguments(edge))
                continue;

            auto oldCall = edge.call;
            List<IRInst*> args;
            for (UInt i = 0; i < oldCall->getArgCount(); ++i)
                args.add(oldCall->getArg(i));

            builder.setInsertBefore(oldCall);
            appendHiddenStateArguments(builder, edge, args);

            auto newCall = builder.emitCallInst(oldCall->getFullType(), oldCall->getCallee(), args);
            newCall->sourceLoc = oldCall->sourceLoc;
            oldCall->transferDecorationsTo(newCall);

            remapExplicitArgumentUseEffects(oldCall, newCall, edge.callerIndex);
            recordHiddenArgumentUseEffects(oldCall, newCall, edge);

            oldCall->replaceUsesWith(newCall);
            oldCall->removeAndDeallocate();
        }
    }

    /// Return whether rewriting one call must append at least one hidden state argument.
    ///
    /// A non-null generated parameter is the phase-4 source of truth. Scan the callee's per-global
    /// records and stop as soon as one such parameter requires this call to be rebuilt.
    bool callNeedsHiddenStateArguments(DirectCallEdge const& edge)
    {
        for (auto const& global : resourceGlobals)
        {
            if (global.functions[edge.calleeIndex].parameter)
                return true;
        }
        return false;
    }

    /// Append hidden arguments in the same global order used to create callee parameters.
    ///
    /// Read-only state is loaded and passed by value. Writers receive the caller's local address,
    /// which matches the callee's generated `out` or `inout` parameter.
    void appendHiddenStateArguments(
        IRBuilder& builder,
        DirectCallEdge const& edge,
        List<IRInst*>& args)
    {
        for (auto const& global : resourceGlobals)
        {
            auto const& calleeState = global.functions[edge.calleeIndex];
            if (!calleeState.parameter)
                continue;

            auto const& callerState = global.functions[edge.callerIndex];
            SLANG_RELEASE_ASSERT(callerState.replacementAddress);
            if (calleeState.access == ResourceStateAccess::Read)
                args.add(builder.emitLoad(global.valueType, callerState.replacementAddress));
            else
                args.add(callerState.replacementAddress);
        }
    }

    /// Retarget saved explicit-argument effects when phase 4 replaces a call instruction.
    ///
    /// The semantic override belongs to an `IRUse`, and replacing the call creates new operand
    /// uses. Match arguments by index so phase 5 still sees the source-level `ref`/`inout` effect.
    void remapExplicitArgumentUseEffects(IRCall* oldCall, IRCall* newCall, Index callerIndex)
    {
        for (auto& global : resourceGlobals)
        {
            auto& effects = global.functions[callerIndex].uninitializedUseEffects;
            for (auto& effect : effects)
            {
                for (UInt argIndex = 0; argIndex < oldCall->getArgCount(); ++argIndex)
                {
                    if (effect.use == oldCall->getOperandUse(argIndex + 1))
                    {
                        effect.use = newCall->getOperandUse(argIndex + 1);
                        break;
                    }
                }
            }
        }
    }

    /// Record the exact definite-assignment effect of each newly appended hidden argument.
    ///
    /// Canonical `out`/`inout` types cannot express a conditional write that preserves an incoming
    /// value without semantically reading it. Associate phase-3 facts with the new operand use so
    /// phase 5 does not infer a stronger effect from the generated ABI type.
    void recordHiddenArgumentUseEffects(
        IRCall* oldCall,
        IRCall* newCall,
        DirectCallEdge const& edge)
    {
        UInt hiddenArgIndex = oldCall->getArgCount();
        for (auto& global : resourceGlobals)
        {
            auto const& calleeState = global.functions[edge.calleeIndex];
            if (!calleeState.parameter)
                continue;

            auto& callerState = global.functions[edge.callerIndex];
            if (hasAccess(calleeState.access, ResourceStateAccess::Write))
            {
                callerState.uninitializedUseEffects.add(UninitializedVariableUseEffect{
                    .use = newCall->getOperandUse(hiddenArgIndex + 1),
                    .readsValue = calleeState.semanticallyReadsIncomingValue,
                    .mayWriteValue = true,
                    .definitelyWritesValue = calleeState.replacesWholeValueOnEveryReturn,
                });
            }
            hiddenArgIndex++;
        }
    }

    // ## Phase 5: Diagnose generated locals and remove obsolete storage

    /// Diagnose entry-point paths that read generated local state before any complete assignment.
    ///
    /// These locals do not exist during mandatory front-end checking. Reuse its intraprocedural
    /// solver here with the exact possible/definite effects retained through phase 3 and rewriting.
    void diagnoseUninitializedEntryPointReads(DiagnosticSink* sink)
    {
        for (auto const& global : resourceGlobals)
        {
            for (Index functionIndex = 0; functionIndex < functions.getCount(); ++functionIndex)
            {
                if (!functions[functionIndex].isEntryPoint)
                    continue;
                auto replacement = global.functions[functionIndex].replacementAddress;
                if (!replacement)
                    continue;
                checkForUsingUninitializedVariable(
                    functions[functionIndex].func,
                    replacement,
                    global.functions[functionIndex].uninitializedUseEffects.getArrayView(),
                    sink);
            }
        }
    }

    /// Remove the obsolete globals after all function and metadata uses have been redirected.
    ///
    /// Phase 4 must leave no uses of the old storage. Assert that invariant at the destruction
    /// boundary so a future unhandled use fails here instead of becoming a dangling reference.
    void removeReplacedResourceGlobals()
    {
        for (auto& global : resourceGlobals)
        {
            SLANG_ASSERT(!global.globalVar->hasUses());
            global.globalVar->removeAndDeallocate();
        }
    }
};

// ## Low-level IR classification helpers

static ResourceStateAccess mergeAccess(ResourceStateAccess left, ResourceStateAccess right)
{
    return ResourceStateAccess(UInt(left) | UInt(right));
}

static bool hasAccess(ResourceStateAccess value, ResourceStateAccess test)
{
    return (UInt(value) & UInt(test)) != 0;
}

/// Return whether an instruction semantically observes or changes an operand's runtime value.
///
/// Address-use analysis must ignore decorations, debug records, types, and queries whose result
/// depends only on an operand's type. Every other use is conservatively treated as a value use so
/// that an unfamiliar instruction cannot silently lose resource state during rewriting.
static bool doesInstSemanticallyUseOperandValue(IRInst* user)
{
    if (as<IRDecoration>(user) || as<IRAnnotation>(user) || as<IRAttr>(user) || as<IRType>(user))
        return false;

    if (doesInstOnlyDependOnOperandTypes(user))
        return false;

    switch (user->getOp())
    {
    case kIROp_DebugValue:
    case kIROp_DebugVar:
    case kIROp_DebugLine:
    case kIROp_DebugInlinedAt:
    case kIROp_DebugScope:
    case kIROp_DebugNoScope:
    case kIROp_DebugInlinedVariable:
    case kIROp_DebugFunction:
    case kIROp_DebugFuncDecoration:
    case kIROp_DebugSource:
    case kIROp_DebugBuildIdentifier:
    case kIROp_DebugCompilationUnit:
        return false;

    default:
        return true;
    }
}

/// Return whether `use` derives another address whose uses must be analyzed recursively.
///
/// Slang address instructions and pointer-preserving casts take their base address as operand zero.
/// Uses in any other operand position are terminal uses of the address value, not derivations.
static bool doesUseDeriveAddress(IRUse* use)
{
    auto user = use->getUser();
    if (user->getOperandCount() == 0 || user->getOperandUse(0) != use)
        return false;

    if (isAddressInst(user))
        return true;

    switch (user->getOp())
    {
    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
    case kIROp_InOutImplicitCast:
        return as<IRPtrTypeBase>(user->getDataType()) != nullptr;
    default:
        return false;
    }
}

/// Return the formal parameter type corresponding to one call argument use.
///
/// Operand zero is the callee, followed by the arguments. Match the exact `IRUse`, then consult the
/// callee's function type and remove attributes so effect and lifetime classifiers see the
/// directional parameter wrapper itself. Return null when any part of that contract is unavailable.
static IRInst* findCallArgumentParameterType(IRCall* call, IRUse* use)
{
    if (use == call->getCalleeUse())
        return nullptr;

    Index argIndex = -1;
    for (UInt i = 0; i < call->getArgCount(); ++i)
    {
        if (call->getOperandUse(i + 1) == use)
        {
            argIndex = Index(i);
            break;
        }
    }
    if (argIndex < 0)
        return nullptr;

    auto funcType = as<IRFuncType>(call->getCallee()->getDataType());
    if (!funcType || UInt(argIndex) >= funcType->getParamCount())
        return nullptr;
    return unwrapAttributedType(funcType->getParamType(UInt(argIndex)));
}

/// Classify how a call may access the resource address passed by `use`.
///
/// Directional parameter types provide the precise read/write contract. If the use cannot be
/// matched to such a parameter, both effects are retained: under-classifying an unknown call would
/// permit the rewrite to omit required incoming state or copy-out behavior.
static ResourceStateAccess classifyCallArgumentAccess(IRCall* call, IRUse* use)
{
    auto paramType = findCallArgumentParameterType(call, use);
    if (!paramType)
        return mergeAccess(ResourceStateAccess::Read, ResourceStateAccess::Write);

    if (as<IROutParamType>(paramType))
        return ResourceStateAccess::Write;
    if (as<IRBorrowInParamType>(paramType))
        return ResourceStateAccess::Read;
    if (as<IRBorrowInOutParamType>(paramType) || as<IRRefParamType>(paramType))
        return mergeAccess(ResourceStateAccess::Read, ResourceStateAccess::Write);
    if (as<IRPtrTypeBase>(paramType))
    {
        // A raw pointer parameter carries no directional contract. Its callee may observe or
        // replace the pointee, so preserve both directions rather than assuming an input use.
        return mergeAccess(ResourceStateAccess::Read, ResourceStateAccess::Write);
    }
    return ResourceStateAccess::Read;
}

/// Classify a non-deriving use reached while walking a resource global's address graph.
///
/// Loads are reads, stores through the address operand are writes, and calls inherit the formal
/// parameter's direction. Uses with no precise contract conservatively preserve every effect they
/// might have. Whether a write replaces the *whole* value is decided by the caller because it also
/// depends on whether this address denotes the root or only a subobject.
static ResourceStateAccess classifyResourceAddressUse(IRUse* use)
{
    auto user = use->getUser();

    if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
        return ResourceStateAccess::Read;

    switch (user->getOp())
    {
    case kIROp_Store:
    case kIROp_AtomicStore:
    case kIROp_SwizzledStore:
    case kIROp_MatrixSwizzleStore:
        return user->getOperandUse(0) == use
                   ? ResourceStateAccess::Write
                   : mergeAccess(ResourceStateAccess::Read, ResourceStateAccess::Write);
    default:
        break;
    }

    if (auto call = as<IRCall>(user))
        return classifyCallArgumentAccess(call, use);

    // A pointer-producing instruction that is not a recognized address derivation may let the
    // address escape. The same is true when the address is routed through control flow or returned.
    // We cannot safely infer how those users access the variable, so preserve both directions.
    if (as<IRPtrTypeBase>(user->getDataType()))
        return mergeAccess(ResourceStateAccess::Read, ResourceStateAccess::Write);

    switch (user->getOp())
    {
    case kIROp_Return:
    case kIROp_UnconditionalBranch:
    case kIROp_Loop:
    case kIROp_IfElse:
    case kIROp_Switch:
        return mergeAccess(ResourceStateAccess::Read, ResourceStateAccess::Write);
    default:
        // Ordinary value consumers observe the resource stored at this address.
        return ResourceStateAccess::Read;
    }
}

/// Return whether replacing the global with independent per-function locals would lose identity.
///
/// Storing or returning the address, routing it through control flow, or applying an address-
/// identity operation lets code observe or retain the original storage. Only known pointee
/// operations and explicitly borrowed call parameters preserve the local-threading model.
static bool doesUseEscapeAddress(IRUse* use)
{
    auto user = use->getUser();
    if (as<IRLoad>(user) || as<IRAtomicLoad>(user))
        return false;
    if (auto store = as<IRStore>(user))
        return store->getOperandUse(0) != use;
    if (as<IRAtomicOperation>(user) || as<IRSwizzledStore>(user) || as<IRMatrixSwizzleStore>(user))
        return false;
    if (auto call = as<IRCall>(user))
    {
        // Directional source parameters borrow their argument storage for the duration of the call.
        // A raw pointer or unknown formal has no non-retention contract, so passing the localized
        // address through it would let the callee retain storage owned by this invocation.
        auto paramType = findCallArgumentParameterType(call, use);
        return !paramType ||
               !(as<IRBorrowInParamType>(paramType) || as<IROutParamType>(paramType) ||
                 as<IRBorrowInOutParamType>(paramType) || as<IRRefParamType>(paramType));
    }

    // Every other terminal consumer either retains the address or observes its identity (for
    // example, pointer-to-integer conversion or pointer comparison). Independent per-function
    // locals cannot preserve one global address across those operations.
    return true;
}

/// Return whether `func` has a use that can invoke it without a rewritable direct call site.
///
/// A direct `IRCall` can receive hidden state arguments. Decorations used by a runtime (for
/// example, a patch-constant function reference) and other first-class function uses cannot.
static bool hasInvocationWithoutRewritableCallSite(IRFunc* func)
{
    for (auto use = func->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (auto call = as<IRCall>(user))
        {
            if (call->getCalleeUse() == use)
                continue;
        }

        // A decoration that names a function can represent a runtime-managed invocation, such as
        // a hull shader's patch-constant function. Other non-call value uses likewise provide no
        // call site where this pass can append a hidden argument. Treat both as independent roots;
        // type and debug metadata do not establish a call boundary.
        if (as<IRDecoration>(user))
            return true;
        if (!doesInstSemanticallyUseOperandValue(user))
            continue;
        return true;
    }
    return false;
}

/// Return whether `func` can be invoked without an ordinary call in this module.
///
/// Phase 4 can add state parameters only when every invocation has a direct `IRCall` to rewrite.
/// Export and keep-alive decorations promise another caller, while a non-call function reference
/// gives the runtime or generated code an invocation path that is not represented as an `IRCall`.
static bool isIndependentlyInvokedFunction(IRFunc* func)
{
    return func->findDecoration<IRKeepAliveDecoration>() ||
           func->findDecoration<IRPublicDecoration>() ||
           func->findDecoration<IRHLSLExportDecoration>() ||
           func->findDecoration<IRDllExportDecoration>() ||
           func->findDecoration<IRExternCDecoration>() ||
           func->findDecoration<IRExternCppDecoration>() ||
           func->findDecoration<IRCudaDeviceExportDecoration>() ||
           func->findDecoration<IRDownstreamModuleExportDecoration>() ||
           func->findDecoration<IRDownstreamModuleImportDecoration>() ||
           hasInvocationWithoutRewritableCallSite(func);
}

/// Return whether `globalVar` must retain one externally meaningful storage location.
///
/// Such storage cannot be replaced by a distinct local in every entry point. A rate or an explicit
/// ABI/storage decoration establishes this requirement; ordinary linkage alone does not.
static bool requiresPreservedGlobalStorage(IRGlobalVar* globalVar)
{
    // An actual-global (or any other rated global) denotes a distinct externally meaningful
    // location and therefore cannot receive resource-dependent initialization inside each entry
    // point. Lack of linkage alone does not preserve storage: function-scope statics are lowered
    // to unlinked, rate-less globals and still have ordinary per-invocation semantics.
    if (globalVar->getRate())
        return true;

    // `IRExportDecoration` is intentionally absent. Ordinary linked source definitions receive
    // that decoration so other IR modules can resolve them; it does not promise externally
    // addressable storage. The decorations below represent call/storage boundaries that must
    // survive final linking and therefore cannot be localized.
    static const IROp kPreservedStorageDecorations[] = {
        kIROp_ImportDecoration,
        kIROp_UserExternDecoration,
        kIROp_PublicDecoration,
        kIROp_KeepAliveDecoration,
        kIROp_DllImportDecoration,
        kIROp_CudaDeviceExportDecoration,
        kIROp_DllExportDecoration,
        kIROp_HLSLExportDecoration,
        kIROp_ExternCDecoration,
        kIROp_ExternCppDecoration,
        kIROp_DownstreamModuleExportDecoration,
        kIROp_DownstreamModuleImportDecoration,
    };
    for (auto op : kPreservedStorageDecorations)
    {
        if (globalVar->findDecorationImpl(op))
            return true;
    }
    return false;
}

} // namespace

void legalizeResourceGlobalVars(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink)
{
    // The task is to make resource-bearing file-scope `static` state explicit per entry-point
    // invocation. Perform it in two ordered steps while keeping their private hand-off scoped here:
    //
    // 1. Move only the initializer dependency closure that requires resource state into entry
    //    points, and collect the wider state whose initialization semantics now depend on those
    //    entry points.
    // 2. Validate storage and invocation boundaries, then localize each resource global and thread
    //    its value through the direct call graph.
    //
    // The ordinary target-policy initializer pass remains later in the pipeline. Moving it here
    // wholesale would broaden this transformation for programs that contain no resource globals.
    List<IRGlobalVar*> resourceDependentState;
    moveResourceDependentGlobalVarInitializationToEntryPoints(
        module,
        targetProgram,
        resourceDependentState);

    // Consume the complete boundary set produced by step 1; clients cannot accidentally invoke
    // the localization pass with a partial view of resource-dependent state.
    LegalizeResourceGlobalVarsPass pass(module);
    pass.processModule(resourceDependentState, sink);
}

} // namespace Slang
