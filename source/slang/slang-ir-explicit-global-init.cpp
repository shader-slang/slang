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
// The mechanism is the same regardless of why an initializer must move. The pass first selects
// the affected globals, splits each selected global into storage and a zero-argument initializer
// function, and then emits calls and stores for those functions at every entry point. Selection has
// two independent modes. The normal target policy handles targets that do not support global
// initializers. The resource-aware policy selects the initializers that must move before source-
// static resource variables can become entry-point-local state.
//
// Resource-aware selection needs an analysis that the normal target policy does not. An
// initializer may depend on resource state indirectly through another initializer or a helper
// function, and a moved initializer may mutate additional global state. The analysis therefore
// computes both the initializer targets to move and a wider set of resource-dependent state that
// callers use to validate independent call roots. It moves only that dependency closure. The
// ordinary target-policy invocation still runs later and independently selects any remaining
// initializers required by the target. Initializers outside the resource-dependent closure remain
// independent under Slang's deliberately unspecified eager-versus-lazy global-initialization
// semantics.

enum class GlobalInitSelectionMode
{
    TargetPolicy,
    ResourceDependencies,
};

// The pass is presented before these low-level IR classifiers; their definitions follow it.
static bool isLinkedPerInvocationGlobal(IRGlobalVar* globalVar);
static bool canCallArgumentWrite(IRCall* call, IRUse* use);
static bool canUseWriteThroughAddress(IRUse* use, HashSet<IRInst*>& visitedAddresses);

struct MoveGlobalVarInitializationToEntryPointsPass
{
    struct CodeDependencyInfo
    {
        // Summarize one function or global initializer body for both resource-dependency and
        // side-effect analysis.
        IRGlobalValueWithCode* code = nullptr;

        // `dependencies` includes both callees and code-bearing globals this body references. The
        // resource-dependency fixed point follows both kinds of edge.
        List<Index> dependencies;

        // Side effects propagate through calls, but merely reading another initialized global does
        // not execute that global's initializer here. Keep the call edges as a separate subset.
        List<Index> callees;

        // This is a conservative summary of globals the code may mutate, either directly or by
        // allowing their addresses to escape.
        List<IRGlobalVar*> mutatedGlobals;

        // Direct resource uses seed this flag; the dependency fixed point then propagates it.
        bool dependsOnResourceState = false;
    };

    IRModule* m_module;
    TargetProgram* m_targetProgram;
    GlobalInitSelectionMode m_selectionMode;

    // Resource-dependent selection maintains related sets with distinct roles:
    //
    // * `m_resourceStateGlobals` contains the linked per-invocation globals whose values contain
    //   resources. They seed the dependency analysis.
    // * `m_resourceDependentInitializerGlobals` contains every initializer target that depends on
    //   those seeds, while `m_selectedResourceDependentInitializerGlobals` is the movable subset
    //   that establishes where explicit initialization must begin.
    // * `m_globalsMutatedByResourceDependentInitializers` contains additional state affected by
    //   the selected resource-dependent initializer bodies and their callees.
    //
    // The output is deliberately wider than the transformation set: later validation must reject
    // independent call roots that could observe any state whose initialization is entry-point-
    // dependent.
    HashSet<IRGlobalVar*> m_resourceStateGlobals;
    HashSet<IRGlobalVar*> m_resourceDependentInitializerGlobals;
    HashSet<IRGlobalVar*> m_selectedResourceDependentInitializerGlobals;
    HashSet<IRGlobalVar*> m_globalsMutatedByResourceDependentInitializers;
    List<IRGlobalVar*>* m_outResourceDependentState;
    List<CodeDependencyInfo> m_codeDependencyInfos;
    Dictionary<IRGlobalValueWithCode*, Index> m_codeDependencyIndices;

    // An `IRGlobalVar` represents a pointer to storage and may also own the code that computes its
    // initial value. This works because `IRGlobalVar` and `IRFunc` both derive from
    // `IRGlobalValueWithCode`.
    //
    // Extraction separates those roles into a global with no initializer body and an ordinary
    // function that computes the initial value. Retain each pair so the injection phase can emit
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
        // The task is to replace selected global initializer bodies with explicit initialization
        // at the start of each entry point. The implementation has three phases:
        //
        // 1. Select the initializers. The target-policy mode can decide one global at a time. The
        //    resource-aware mode first summarizes the module and computes the transitive
        //    dependency closure, selects its movable initializer targets, and computes state
        //    mutated by their call graph. It reports the wider resource-dependent state for later
        //    call-boundary validation. The ordinary target-policy mode runs separately later.
        // 2. Split every selected global into uninitialized storage and a zero-argument function
        //    containing its former initializer body.
        // 3. At every entry point, call those functions and store their results into the
        //    corresponding globals.
        //
        // Keeping phases 2 and 3 common is important: resource-dependent initialization adds a
        // selection policy to the established mechanism; it is not a second initialization scheme.
        m_module = module;
        m_targetProgram = targetProgram;
        m_selectionMode = selectionMode;
        m_outResourceDependentState = outResourceDependentState;

        if (m_selectionMode == GlobalInitSelectionMode::ResourceDependencies)
        {
            collectInitializerDependencyGraph();
            selectResourceDependentInitializers();
            collectStateMutatedByResourceDependentInitializers();
            writeResourceDependentStateOutput();
        }

        // Split the selected initializer bodies. Iterating in module order also fixes the order in
        // which `injectInitializersAtEntryPoint` will emit their calls and stores.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;

            // `ActualGlobal` storage is shared outside an invocation, so its initialization cannot
            // be reproduced independently in every entry point. Resource-dependent analysis still
            // includes such a target in its output so that the caller can diagnose the boundary.
            if (as<IRActualGlobalRate>(globalVar->getRate()))
                continue;

            auto firstBlock = globalVar->getFirstBlock();
            if (!firstBlock)
                continue;

            extractSelectedInitializer(globalVar, firstBlock);
        }

        // Reproduce the selected initialization at the start of every defined entry point.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            if (!func->findDecoration<IREntryPointDecoration>())
                continue;

            injectInitializersAtEntryPoint(func);
        }
    }

    // ## Phase 1: Select initializer targets and summarize dependent state

    bool isInitializerRequiredByTargetPolicy(IRGlobalVar* globalVar)
    {
        // Apply the established target policy. Non-D3D targets move every initializer. HLSL keeps
        // representable initialization global and moves only cooperative-vector construction that
        // DXC cannot perform at global scope.
        if (isD3DTarget(m_targetProgram->getTargetReq()))
        {
            auto valueType = globalVar->getDataType()->getValueType();
            if (as<IRCoopVectorType>(valueType))
                return true;
            return false;
        }
        return true;
    }

    bool isInitializerSelected(IRGlobalVar* globalVar)
    {
        // The two modes answer separate selection questions but share extraction and injection.
        // Resource-aware processing moves only the transitive resource dependency closure; the
        // ordinary target-policy pass runs later for everything else.
        if (m_selectionMode == GlobalInitSelectionMode::ResourceDependencies)
            return m_selectedResourceDependentInitializerGlobals.contains(globalVar);
        return isInitializerRequiredByTargetPolicy(globalVar);
    }

    void collectInitializerDependencyGraph()
    {
        // Resource-dependent selection must answer two related questions: which initializer values
        // transitively depend on resource state, and which additional globals the selected
        // initializer code may mutate. Summarize every code-bearing global value once so both
        // answers use the same view of calls, global-value dependencies, and address effects.
        //
        // Build the summary in three passes. First collect the resource-state seeds. Next assign an
        // index to every code body; indexing must finish before we can represent forward edges.
        // Finally inspect each body to record dependencies and mutations.
        collectResourceStateGlobals();
        indexCodeBodies();
        for (auto& info : m_codeDependencyInfos)
        {
            for (auto block : info.code->getBlocks())
            {
                for (auto inst : block->getChildren())
                    summarizeInstructionDependencies(info, inst);
            }
        }
    }

    void collectResourceStateGlobals()
    {
        // Resource dependence starts at linked, per-invocation globals whose values contain a
        // resource. Collect those seeds separately from initializer targets: a resource global may
        // have no initializer body, while a non-resource global may depend on one transitively.
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
        // Assign an index to every function or global initializer with a body. Indices let the
        // summary represent graph edges compactly. Mark a resource global's own initializer body
        // as dependent immediately; other direct resource uses are found when bodies are scanned.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto code = as<IRGlobalValueWithCode>(inst);
            if (!code || !code->getFirstBlock())
                continue;

            Index index = m_codeDependencyInfos.getCount();
            m_codeDependencyInfos.add(CodeDependencyInfo{code});
            m_codeDependencyIndices.add(code, index);

            if (auto globalVar = as<IRGlobalVar>(code))
            {
                if (m_resourceStateGlobals.contains(globalVar))
                    m_codeDependencyInfos[index].dependsOnResourceState = true;
            }
        }
    }

    void summarizeInstructionDependencies(CodeDependencyInfo& info, IRInst* inst)
    {
        // Each instruction can contribute call edges, global-value dependencies, and mutation
        // effects to its enclosing body. A direct call is both a value dependency and a call edge.
        // Keeping call edges distinct lets side-effect collection later follow only code that
        // actually executes as part of a selected initializer.
        if (auto call = as<IRCall>(inst))
        {
            if (auto callee = as<IRGlobalValueWithCode>(call->getCallee()))
            {
                if (auto dependencyIndex = m_codeDependencyIndices.tryGetValue(callee))
                {
                    info.dependencies.add(*dependencyIndex);
                    info.callees.add(*dependencyIndex);
                }
            }
        }

        // A global operand conservatively creates a dependency when that global has an initializer
        // body. Independently, classify the particular operand use to find writes through the
        // global's address. The write analysis follows derived addresses, so one root operand is
        // sufficient.
        for (UInt operandIndex = 0; operandIndex < inst->getOperandCount(); ++operandIndex)
        {
            auto globalVar = as<IRGlobalVar>(inst->getOperand(operandIndex));
            if (!globalVar)
                continue;

            if (m_resourceStateGlobals.contains(globalVar))
                info.dependsOnResourceState = true;
            if (auto dependencyIndex = m_codeDependencyIndices.tryGetValue(globalVar))
                info.dependencies.add(*dependencyIndex);

            HashSet<IRInst*> visitedAddresses;
            if (canUseWriteThroughAddress(inst->getOperandUse(operandIndex), visitedAddresses))
                info.mutatedGlobals.add(globalVar);
        }
    }

    void selectResourceDependentInitializers()
    {
        // Select every initializer whose value transitively depends on resource state. Starting
        // with the direct-use marks from `collectInitializerDependencyGraph`, propagate the mark
        // backward: code is resource-dependent if any code on which it depends is resource-
        // dependent. Repeating to a fixed point handles arbitrary call and initializer-reference
        // depth without making assumptions about module order.
        bool changed = false;
        do
        {
            changed = false;
            for (auto& info : m_codeDependencyInfos)
            {
                if (info.dependsOnResourceState)
                    continue;

                for (auto dependencyIndex : info.dependencies)
                {
                    if (!m_codeDependencyInfos[dependencyIndex].dependsOnResourceState)
                        continue;

                    info.dependsOnResourceState = true;
                    changed = true;
                    break;
                }
            }
        } while (changed);

        // The fixed point includes functions and global initializer targets. Every dependent
        // initializer target belongs to the wider boundary-check state, but only linked
        // per-invocation targets are safe for this pass to move into each entry point.
        for (auto const& info : m_codeDependencyInfos)
        {
            if (!info.dependsOnResourceState)
                continue;
            if (auto globalVar = as<IRGlobalVar>(info.code))
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
        // its value dependencies. Walk the call graph rooted at the selected resource-dependent
        // initializers and collect all summarized mutations. The caller will treat that state as
        // resource-dependent so an independent call root cannot observe entry-point-only effects.
        List<Index> workList;
        HashSet<Index> reachableCode;
        for (auto globalVar : m_selectedResourceDependentInitializerGlobals)
        {
            if (auto codeIndex = m_codeDependencyIndices.tryGetValue(globalVar))
            {
                if (reachableCode.add(*codeIndex))
                    workList.add(*codeIndex);
            }
        }
        while (workList.getCount())
        {
            auto codeIndex = workList.getLast();
            workList.removeLast();
            auto const& info = m_codeDependencyInfos[codeIndex];
            for (auto globalVar : info.mutatedGlobals)
                m_globalsMutatedByResourceDependentInitializers.add(globalVar);
            for (auto calleeIndex : info.callees)
            {
                if (reachableCode.add(calleeIndex))
                    workList.add(calleeIndex);
            }
        }
    }

    void writeResourceDependentStateOutput()
    {
        // Report the complete state whose semantics now depend on entry-point initialization. This
        // is the union of resource-containing state, every resource-dependent initializer target,
        // and ordinary state mutated by the selected initializer call graph. Emit the union in
        // module order to make downstream behavior and diagnostics deterministic.
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

    // ## Phase 2: Split selected globals into storage and initializer functions

    void extractSelectedInitializer(IRGlobalVar* globalVar, IRBlock* firstBlock)
    {
        // A selected global currently combines storage with a body that computes its initial value,
        // but explicit entry-point initialization needs those pieces separately. If this global
        // belongs to the current selection, create a zero-argument function and move the existing
        // initializer blocks into it. Remember the storage/function pair for entry-point emission.
        if (!isInitializerSelected(globalVar))
            return;

        IRBuilder builder(m_module);
        builder.setInsertBefore(globalVar);

        // The initializer function returns the value stored by the global pointer.
        auto valueType = globalVar->getDataType()->getValueType();

        // Preserve the initializer's implicit zero-argument calling convention explicitly.
        auto initFunc = builder.createFunc();
        initFunc->setFullType(builder.getFuncType(0, nullptr, valueType));

        // Move the existing body rather than cloning it. The global is left with storage only, and
        // the new function preserves the exact initializer control flow.
        IRBlock* nextBlock = nullptr;
        for (IRBlock* block = firstBlock; block; block = nextBlock)
        {
            nextBlock = block->getNextBlock();

            block->removeFromParent();
            block->insertAtEnd(initFunc);
        }

        // Record the pair in module order for the injection phase.
        ExtractedInitializer info;
        info.globalVar = globalVar;
        info.initFunc = initFunc;
        m_extractedInitializers.add(info);
    }

    // ## Phase 3: Reproduce selected initialization at each entry point

    IRInst* findTrivialInitializerValue(IRFunc* initFunc)
    {
        // We may replace an initializer call with its returned module-scope value, but only when
        // calling the function cannot have any other observable effect. Require its entire body to
        // be one block whose only ordinary instruction is the return. Merely inspecting the return
        // operand is insufficient: `static Texture2D t = (++count, inputTexture);` also returns a
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
        // module order used during extraction. Insert a store for each storage/function pair at the
        // start of the entry block. A function that merely returns a module-scope value can be
        // replaced by that value; all other initializer bodies remain explicit calls.
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

            // The call result has the value type pointed to by the global storage.
            auto valType = globalVar->getDataType()->getValueType();

            // Avoid calling a trivial function when the initializer is already a module-scope
            // value that can be stored directly.
            IRInst* initVal = findTrivialInitializerValue(initFunc);
            if (!initVal)
            {
                // Non-trivial initializers execute through the extracted zero-argument function.
                initVal = builder.emitCallInst(valType, initFunc, 0, nullptr);
            }
            builder.emitStore(globalVar, initVal);
        }
    }
};

// ## Low-level IR classifiers

static bool isLinkedPerInvocationGlobal(IRGlobalVar* globalVar)
{
    // At this post-link pipeline point, a linked, rate-less initializer is eager source-global
    // state whose storage is instantiated separately for each entry-point invocation. A rate marks
    // storage with a different lifetime; an unlinked initializer belongs to compiler-generated
    // lazy function-static machinery and must not be moved into every entry point.
    return !globalVar->getRate() && globalVar->findDecoration<IRLinkageDecoration>();
}

static bool canCallArgumentWrite(IRCall* call, IRUse* use)
{
    // To summarize an initializer's side effects, determine whether a call can write through this
    // particular argument. Match the operand to its parameter and use the parameter direction as
    // the contract: ordinary values and `BorrowIn` are read-only, while output, in-out, reference,
    // and pointer parameters can write. An unrecognized signature or operand is conservatively
    // write-capable.
    for (UInt argIndex = 0; argIndex < call->getArgCount(); ++argIndex)
    {
        if (call->getOperandUse(argIndex + 1) != use)
            continue;

        auto funcType = as<IRFuncType>(call->getCallee()->getDataType());
        if (!funcType || argIndex >= funcType->getParamCount())
            return true;
        auto paramType = unwrapAttributedType(funcType->getParamType(argIndex));
        if (as<IRBorrowInParamType>(paramType))
            return false;
        return as<IROutParamType>(paramType) || as<IRBorrowInOutParamType>(paramType) ||
               as<IRRefParamType>(paramType) || as<IRPtrTypeBase>(paramType);
    }
    return true;
}

static bool canUseWriteThroughAddress(IRUse* use, HashSet<IRInst*>& visitedAddresses)
{
    // A global address can be propagated through the initializer call graph before it is written.
    // Classify known readers and writers directly, inspect calls using their parameter contracts,
    // and recursively follow instructions that derive another address. Unknown consumers may let
    // the address escape, so they are conservatively treated as writes. The visited set makes the
    // recursive walk terminate if address-producing IR contains a cycle.
    auto user = use->getUser();
    if (as<IRLoad>(user) || as<IRAtomicLoad>(user) || doesInstOnlyDependOnOperandTypes(user))
        return false;

    if (as<IRStore>(user))
    {
        // A store through this address mutates the pointee. Storing the address as the value lets
        // it escape to code that may mutate the pointee later. Both cases require state threading.
        return true;
    }
    if (as<IRAtomicOperation>(user) || as<IRSwizzledStore>(user) || as<IRMatrixSwizzleStore>(user))
    {
        return true;
    }
    if (auto call = as<IRCall>(user))
        return canCallArgumentWrite(call, use);

    bool derivesAddress = isAddressInst(user);
    switch (user->getOp())
    {
    case kIROp_BitCast:
    case kIROp_Reinterpret:
    case kIROp_PtrCast:
    case kIROp_InOutImplicitCast:
        derivesAddress |= as<IRPtrTypeBase>(user->getDataType()) != nullptr;
        break;
    default:
        break;
    }
    if (derivesAddress && visitedAddresses.add(user))
    {
        for (auto derivedUse = user->firstUse; derivedUse; derivedUse = derivedUse->nextUse)
        {
            if (canUseWriteThroughAddress(derivedUse, visitedAddresses))
                return true;
        }
        return false;
    }

    // Returning, storing, or otherwise escaping an address may permit a write outside the code we
    // can inspect here. Treat unknown address consumers as mutating.
    return true;
}

void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram)
{
    // Apply the ordinary target selection policy using the common extraction and injection
    // mechanism.
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(module, targetProgram, GlobalInitSelectionMode::TargetPolicy);
}

void moveResourceDependentGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    List<IRGlobalVar*>& outResourceDependentState)
{
    // Apply resource-dependency selection using the common transformation and return the wider
    // state required for subsequent call-boundary validation.
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(
        module,
        targetProgram,
        GlobalInitSelectionMode::ResourceDependencies,
        &outResourceDependentState);
}

} // namespace Slang
