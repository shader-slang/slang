// slang-ir-explicit-global-init.cpp
#include "slang-ir-explicit-global-init.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{

// This pass takes code in a form like:
//
//      static int gCounter = 1;
//
//      void computeMain()
//      {
//          ...
//          int tmp = gCounter++;
//      }
//
// and makes the initialization explicit at the start of each entry point:
//
//      static int gCounter;
//
//      void computeMain()
//      {
//          gCounter = 1;
//          ...
//          int tmp = gCounter++;
//      }
//
// We use the same three-step transformation for both selection policies. We first select the
// affected globals whose storage is eligible for per-entry-point initialization. We then split each
// selected global into storage and a zero-argument function that computes its initial value.
// Finally, we call those functions and store their results at the start of every entry point.
//
// Under the target policy, we move every initializer the target cannot represent at global scope.
// Under the resource-dependency policy, we start from linked per-invocation resource-state globals
// and move only resource-dependent initializer globals: initializer targets whose values
// transitively depend on those seeds. We also collect ordinary globals that the moved initializers
// may mutate. The union is resource-dependent state, which later validation uses to reject
// functions that can execute without first passing through an entry point and could therefore
// observe state initialized only there.
//
// We leave unrelated initializers for the later target-policy pass. This preserves the established
// behavior without imposing an eager-versus-lazy initialization order that Slang does not specify.

/// Selects the policy that decides which globals use the shared extraction and injection mechanism.
enum class GlobalInitSelectionMode
{
    TargetPolicy,
    ResourceDependencies,
};

// We present the pass before these low-level classifiers so that the file reads in algorithm order.
// Their definitions follow the transformation that uses them.
static bool isLinkedPerInvocationGlobal(IRGlobalVar* globalVar);
static bool canCallWriteThroughArgument(IRCall* call, IRUse* use);
static bool mayWriteThroughAddressUse(IRUse* use, HashSet<IRInst*>& visitedAddresses);

struct MoveGlobalVarInitializationToEntryPointsPass
{
    /// Records the dependencies and possible mutations of one function or global initializer body.
    struct CodeBodySummary
    {
        /// The body described by this summary.
        IRGlobalValueWithCode* code = nullptr;

        /// The callees and initialized globals through which resource dependence can propagate.
        List<Index> dependencies;

        /// The dependency edges that execute another body and therefore propagate its mutations.
        ///
        /// Merely reading an initialized global does not execute its initializer here, so we keep
        /// call edges as a separate subset.
        List<Index> callees;

        /// The globals this body may mutate directly or by allowing their addresses to escape.
        List<IRGlobalVar*> mutatedGlobals;

        /// Whether this body directly or transitively depends on a resource-state global.
        bool dependsOnResourceState = false;
    };

    IRModule* m_module;
    TargetProgram* m_targetProgram;
    GlobalInitSelectionMode m_selectionMode;

    // The resource-dependency policy maintains related sets with distinct roles:
    //
    // * `m_resourceStateGlobals` contains the linked per-invocation globals whose values contain
    //   resources. They seed the dependency analysis.
    // * `m_resourceDependentInitializerGlobals` contains every initializer target that depends on
    //   those seeds, while `m_selectedResourceDependentInitializerGlobals` is the movable subset
    //   that establishes where explicit initialization must begin.
    // * `m_globalsMutatedByResourceDependentInitializers` contains additional state affected by
    //   the selected resource-dependent initializer bodies and their callees.
    //
    // The output is deliberately wider than the transformation set. Later validation must reject
    // any function that can execute without first passing through an entry point and can observe
    // state whose initialization happens only there.
    HashSet<IRGlobalVar*> m_resourceStateGlobals;
    HashSet<IRGlobalVar*> m_resourceDependentInitializerGlobals;
    HashSet<IRGlobalVar*> m_selectedResourceDependentInitializerGlobals;
    HashSet<IRGlobalVar*> m_globalsMutatedByResourceDependentInitializers;
    List<IRGlobalVar*>* m_outResourceDependentState;
    List<CodeBodySummary> m_codeBodySummaries;
    Dictionary<IRGlobalValueWithCode*, Index> m_codeBodyIndices;

    // An `IRGlobalVar` represents a pointer to storage and may also own the code that computes its
    // initial value. This works because `IRGlobalVar` and `IRFunc` both derive from
    // `IRGlobalValueWithCode`.
    //
    // Extraction separates those roles into a global with no initializer body and an ordinary
    // function that computes the initial value. We retain each pair so the injection phase can emit
    // the corresponding call and store.
    struct ExtractedInitializer
    {
        IRGlobalVar* globalVar = nullptr;
        IRFunc* initFunc = nullptr;
    };
    List<ExtractedInitializer> m_extractedInitializers;

    void processModule(
        IRModule* module,
        TargetProgram* targetProgram,
        GlobalInitSelectionMode selectionMode,
        List<IRGlobalVar*>* outResourceDependentState = nullptr)
    {
        // In resource-dependency mode, we first summarize the module and select the dependency
        // closure. In either mode, we then traverse globals in module order, extracting each
        // initializer selected by the active policy. Finally, we reproduce those initializers at
        // every defined entry point. We share the latter two steps so resource legalization adds
        // only a selection policy, not a second initialization mechanism.
        m_module = module;
        m_targetProgram = targetProgram;
        m_selectionMode = selectionMode;
        m_outResourceDependentState = outResourceDependentState;

        if (m_selectionMode == GlobalInitSelectionMode::ResourceDependencies)
        {
            buildCodeBodySummaries();
            selectResourceDependentInitializers();
            collectStateMutatedByResourceDependentInitializers();
            writeResourceDependentStateOutput();
        }

        extractSelectedInitializers();
        injectInitializersIntoEntryPoints();
    }

    // ## Resource-dependency preselection and state summary

    bool isInitializerRequiredByTargetPolicy(IRGlobalVar* globalVar)
    {
        // We preserve the established target policy. Non-D3D targets move every initializer. D3D
        // targets keep initializers that HLSL can represent at global scope, but move cooperative-
        // vector construction because DXC cannot perform it there.
        if (isD3DTarget(m_targetProgram->getTargetReq()))
        {
            auto valueType = globalVar->getDataType()->getValueType();
            if (as<IRCoopVectorType>(valueType))
                return true;
            return false;
        }
        return true;
    }

    bool shouldExtractInitializer(IRGlobalVar* globalVar)
    {
        // We can reproduce an initializer independently in every entry point only when the global
        // does not have `ActualGlobal` storage. Among the eligible globals, the two policies answer
        // separate selection questions: resource-dependency processing moves only its selected
        // closure, while the ordinary target policy runs later for everything else.
        if (as<IRActualGlobalRate>(globalVar->getRate()))
            return false;
        if (m_selectionMode == GlobalInitSelectionMode::ResourceDependencies)
            return m_selectedResourceDependentInitializerGlobals.contains(globalVar);
        return isInitializerRequiredByTargetPolicy(globalVar);
    }

    void buildCodeBodySummaries()
    {
        // Resource-dependency selection must answer two related questions: which initializer values
        // transitively depend on resource-state globals, and which additional globals the selected
        // initializer code may mutate. We summarize every code-bearing global value once so both
        // answers use the same view of calls, global-value dependencies, and address effects.
        //
        // We build the summary in three passes. We first collect the resource-state seeds and then
        // assign an index to every code body; indexing must finish before we can represent forward
        // edges. Finally, we inspect each body to record dependencies and mutations.
        collectResourceStateGlobals();
        indexCodeBodies();
        for (auto& summary : m_codeBodySummaries)
        {
            for (auto block : summary.code->getBlocks())
            {
                for (auto inst : block->getChildren())
                    summarizeInstruction(summary, inst);
            }
        }
    }

    void collectResourceStateGlobals()
    {
        // Resource dependence starts at linked, per-invocation globals whose values contain a
        // resource. We collect those seeds separately from initializer targets: a resource global
        // may have no initializer body, while a non-resource global may depend on one transitively.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;
            if (isPerInvocationResourceStateGlobalVar(globalVar))
                m_resourceStateGlobals.add(globalVar);
        }
    }

    void indexCodeBodies()
    {
        // We assign an index to every function or global initializer with a body. Indices let the
        // summary represent graph edges compactly. We mark a resource global's own initializer
        // body as dependent immediately; other direct resource uses are found when bodies are
        // scanned.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto code = as<IRGlobalValueWithCode>(inst);
            if (!code || !code->getFirstBlock())
                continue;

            Index index = m_codeBodySummaries.getCount();
            m_codeBodySummaries.add(CodeBodySummary{code});
            m_codeBodyIndices.add(code, index);

            if (auto globalVar = as<IRGlobalVar>(code))
            {
                if (m_resourceStateGlobals.contains(globalVar))
                    m_codeBodySummaries[index].dependsOnResourceState = true;
            }
        }
    }

    void summarizeInstruction(CodeBodySummary& summary, IRInst* inst)
    {
        // Each instruction can contribute call edges, global-value dependencies, and mutation
        // effects to its enclosing body. A direct call is both a value dependency and a call edge.
        // Keeping call edges distinct lets side-effect collection later follow only code that
        // actually executes as part of a selected initializer.
        if (auto call = as<IRCall>(inst))
        {
            if (auto callee = as<IRGlobalValueWithCode>(call->getCallee()))
            {
                if (auto dependencyIndex = m_codeBodyIndices.tryGetValue(callee))
                {
                    summary.dependencies.add(*dependencyIndex);
                    summary.callees.add(*dependencyIndex);
                }
            }
        }

        // When an operand is a global with an initializer body, we add that body as a value
        // dependency. We separately inspect this exact operand use for writes; the recursive
        // analysis follows every address derived from it.
        for (UInt operandIndex = 0; operandIndex < inst->getOperandCount(); ++operandIndex)
        {
            auto globalVar = as<IRGlobalVar>(inst->getOperand(operandIndex));
            if (!globalVar)
                continue;

            if (m_resourceStateGlobals.contains(globalVar))
                summary.dependsOnResourceState = true;
            if (auto dependencyIndex = m_codeBodyIndices.tryGetValue(globalVar))
                summary.dependencies.add(*dependencyIndex);

            HashSet<IRInst*> visitedAddresses;
            if (mayWriteThroughAddressUse(inst->getOperandUse(operandIndex), visitedAddresses))
                summary.mutatedGlobals.add(globalVar);
        }
    }

    void selectResourceDependentInitializers()
    {
        // We begin with bodies marked by direct resource uses. We then repeatedly mark a body when
        // any of its dependencies is marked. Iterating to a fixed point handles arbitrary call and
        // initializer-reference depth without relying on module order.
        bool changed = false;
        do
        {
            changed = false;
            for (auto& summary : m_codeBodySummaries)
            {
                if (summary.dependsOnResourceState)
                    continue;

                for (auto dependencyIndex : summary.dependencies)
                {
                    if (!m_codeBodySummaries[dependencyIndex].dependsOnResourceState)
                        continue;

                    summary.dependsOnResourceState = true;
                    changed = true;
                    break;
                }
            }
        } while (changed);

        // The fixed point includes functions and global initializer targets. Every dependent
        // initializer target belongs to the wider boundary-check state, but only linked
        // per-invocation targets are safe for this pass to move into each entry point.
        for (auto const& summary : m_codeBodySummaries)
        {
            if (!summary.dependsOnResourceState)
                continue;
            if (auto globalVar = as<IRGlobalVar>(summary.code))
            {
                m_resourceDependentInitializerGlobals.add(globalVar);
                if (isLinkedPerInvocationGlobal(globalVar))
                    m_selectedResourceDependentInitializerGlobals.add(globalVar);
            }
        }
    }

    void collectStateMutatedByResourceDependentInitializers()
    {
        // Moving a resource-dependent initializer also moves its side effects. An initializer can
        // mutate ordinary state directly or through a helper even when that state is not part of
        // its value dependencies. We walk the call graph rooted at the selected resource-dependent
        // initializers and collect all summarized mutations. The caller treats that state as
        // resource-dependent so a function that can execute without first passing through an entry
        // point cannot observe effects that occur only during entry-point initialization.
        List<Index> workList;
        HashSet<Index> reachableCode;
        for (auto globalVar : m_selectedResourceDependentInitializerGlobals)
        {
            if (auto codeIndex = m_codeBodyIndices.tryGetValue(globalVar))
            {
                if (reachableCode.add(*codeIndex))
                    workList.add(*codeIndex);
            }
        }
        while (workList.getCount())
        {
            auto codeIndex = workList.getLast();
            workList.removeLast();
            auto const& summary = m_codeBodySummaries[codeIndex];
            for (auto globalVar : summary.mutatedGlobals)
                m_globalsMutatedByResourceDependentInitializers.add(globalVar);
            for (auto calleeIndex : summary.callees)
            {
                if (reachableCode.add(calleeIndex))
                    workList.add(calleeIndex);
            }
        }
    }

    void writeResourceDependentStateOutput()
    {
        // We report the conservative state whose semantics will depend on entry-point
        // initialization once we move the selected initializers. This is the union of
        // resource-state globals, every resource-dependent initializer global, and ordinary state
        // that the selected initializer call graph may mutate. We emit the union in module order to
        // make downstream behavior and diagnostics deterministic.
        if (!m_outResourceDependentState)
            return;

        m_outResourceDependentState->clear();
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;
            if (m_resourceStateGlobals.contains(globalVar) ||
                m_resourceDependentInitializerGlobals.contains(globalVar) ||
                m_globalsMutatedByResourceDependentInitializers.contains(globalVar))
            {
                m_outResourceDependentState->add(globalVar);
            }
        }
    }

    // ## Extracted storage and initializer functions

    void extractSelectedInitializers()
    {
        // We consider initializer-bearing globals in module order, which also determines the order
        // in which entry-point injection emits their calls and stores. We exclude shared
        // `ActualGlobal` storage because reproducing its initialization independently in every
        // entry point would change its lifetime. Resource-dependent analysis still reports such a
        // target so that the caller can diagnose that unsupported boundary.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;

            auto firstBlock = globalVar->getFirstBlock();
            if (!firstBlock || !shouldExtractInitializer(globalVar))
                continue;

            extractInitializer(globalVar, firstBlock);
        }
    }

    void extractInitializer(IRGlobalVar* globalVar, IRBlock* firstBlock)
    {
        // A selected global combines storage with a body that computes its initial value, while
        // explicit entry-point initialization needs those pieces separately. We create a
        // zero-argument function, move the existing initializer blocks into it, and remember the
        // storage/function pair for entry-point injection.

        IRBuilder builder(m_module);
        builder.setInsertBefore(globalVar);

        // An `IRGlobalVar` has pointer type, so we use its pointee as the initializer function's
        // result type.
        auto valueType = globalVar->getDataType()->getValueType();

        // Global initializer bodies have no parameters, so the extracted function has none.
        auto initFunc = builder.createFunc();
        initFunc->setFullType(builder.getFuncType(0, nullptr, valueType));

        // We move the existing body rather than cloning it. The global is left with storage only,
        // and the new function preserves the exact initializer control flow.
        IRBlock* nextBlock = nullptr;
        for (IRBlock* block = firstBlock; block; block = nextBlock)
        {
            nextBlock = block->getNextBlock();

            block->removeFromParent();
            block->insertAtEnd(initFunc);
        }

        ExtractedInitializer info;
        info.globalVar = globalVar;
        info.initFunc = initFunc;
        m_extractedInitializers.add(info);
    }

    // ## Selected initialization at each entry point

    void injectInitializersIntoEntryPoints()
    {
        // We reproduce the selected initialization at the start of every defined entry point.
        // Declarations have no body in which to insert it, so they remain unchanged.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto func = as<IRFunc>(inst);
            if (!func || !func->findDecoration<IREntryPointDecoration>())
                continue;

            injectInitializersAtEntryPoint(func);
        }
    }

    IRInst* findDirectModuleScopeInitializerValue(IRFunc* initFunc)
    {
        // We can replace an initializer call with its returned module-scope value only when the
        // function has no other observable effect. We therefore require one block whose only
        // ordinary instruction is the return, and we require the returned value to live at module
        // scope so that it is available at every entry point. Merely inspecting the return operand
        // is insufficient: `static Texture2D t = (++count, inputTexture);` also returns a
        // module-scope value, but the increment preceding that return must still execute.
        auto block = initFunc->getFirstBlock();
        if (!block || block->getNextBlock())
            return nullptr;

        auto returnInst = as<IRReturn>(block->getFirstOrdinaryInst());
        if (!returnInst || returnInst != block->getTerminator())
            return nullptr;

        auto value = returnInst->getVal();
        if (!value || value->getParent() != m_module->getModuleInst())
            return nullptr;

        return value;
    }

    void injectInitializersAtEntryPoint(IRFunc* entryPointFunc)
    {
        // Each defined entry point must reproduce the selected global initialization in the same
        // module order used during extraction. We insert one store for each storage/function pair
        // at the start of the entry block. When the initializer merely returns a module-scope
        // value, we store that value directly; all other initializer bodies remain explicit calls.
        auto firstBlock = entryPointFunc->getFirstBlock();
        if (!firstBlock)
            return;

        // Initialization must precede the entry point's first ordinary instruction.
        IRBuilder builder(m_module);
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());

        for (auto initializer : m_extractedInitializers)
        {
            auto globalVar = initializer.globalVar;
            auto initFunc = initializer.initFunc;

            // The extracted function returns the pointee value stored in the global.
            auto valType = globalVar->getDataType()->getValueType();

            // We avoid a call when the initializer is already a module-scope value that can be
            // stored directly.
            IRInst* initVal = findDirectModuleScopeInitializerValue(initFunc);
            if (!initVal)
            {
                // Otherwise, we execute the initializer through its extracted zero-argument
                // function.
                initVal = builder.emitCallInst(valType, initFunc, 0, nullptr);
            }
            builder.emitStore(globalVar, initVal);
        }
    }
};

// ## Classifying movable globals and initializer writes

static bool isLinkedPerInvocationGlobal(IRGlobalVar* globalVar)
{
    // We select linked globals because linkage identifies the source declarations whose
    // per-invocation semantics this policy handles. We leave unlinked storage to its producer's
    // lifetime policy, while a rate explicitly denotes a different lifetime; neither category can
    // be assumed safe to initialize independently at every entry point.
    return !globalVar->getRate() && globalVar->findDecoration<IRLinkageDecoration>();
}

static bool canCallWriteThroughArgument(IRCall* call, IRUse* use)
{
    // We match this argument use to its formal parameter. An ordinary value exposes no address, and
    // `BorrowIn` is pointer-shaped in IR but promises read-only access; we therefore reject that
    // case before the generic pointer test. `out`, `inout`, `ref`, and raw-pointer parameters can
    // mutate the pointee. If the signature is unavailable, we conservatively report a possible
    // write so boundary validation cannot omit affected state.
    auto paramType = findCallArgumentParameterType(call, use);
    if (!paramType)
        return true;
    if (as<IRBorrowInParamType>(paramType))
        return false;
    return as<IROutParamType>(paramType) || as<IRBorrowInOutParamType>(paramType) ||
           as<IRRefParamType>(paramType) || as<IRPtrTypeBase>(paramType);
}

static bool mayWriteThroughAddressUse(IRUse* use, HashSet<IRInst*>& visitedAddresses)
{
    // We follow address derivations within one function or initializer body to determine whether
    // code can write through a global address. We classify known readers and writers directly,
    // classify calls from their parameter contracts, and recursively inspect instructions that
    // derive another address. Unknown consumers may let the address escape, so we conservatively
    // report a possible write. The visited set makes the recursive walk terminate if address-
    // producing IR contains a cycle.
    auto user = use->getUser();
    if (as<IRLoad>(user) || as<IRAtomicLoad>(user) || doesInstOnlyDependOnOperandTypes(user))
        return false;

    if (as<IRStore>(user))
    {
        // A store can either write through this address or store the address itself. In either
        // case, the surrounding body may mutate this global, so we include it in the mutation
        // summary.
        return true;
    }
    if (as<IRAtomicOperation>(user) || as<IRSwizzledStore>(user) || as<IRMatrixSwizzleStore>(user))
    {
        return true;
    }
    if (auto call = as<IRCall>(user))
        return canCallWriteThroughArgument(call, use);

    if (doesUseDeriveAddress(use))
    {
        if (!visitedAddresses.add(user))
            return false;
        for (auto derivedUse = user->firstUse; derivedUse; derivedUse = derivedUse->nextUse)
        {
            if (mayWriteThroughAddressUse(derivedUse, visitedAddresses))
                return true;
        }
        return false;
    }

    // Any remaining use may let the address escape beyond code we can inspect. We conservatively
    // report a possible mutation so boundary validation cannot omit affected state.
    return true;
}

void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram)
{
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(module, targetProgram, GlobalInitSelectionMode::TargetPolicy);
}

void moveResourceDependentGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    List<IRGlobalVar*>& outResourceDependentState)
{
    // We use resource-dependency selection here and return its wider state set so resource-global
    // legalization can validate storage and invocation boundaries before localizing that state.
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(
        module,
        targetProgram,
        GlobalInitSelectionMode::ResourceDependencies,
        &outResourceDependentState);
}

} // namespace Slang
