// slang-ir-explicit-global-init.cpp
#include "slang-ir-explicit-global-init.h"

#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{

// A global variable may contain both its storage declaration and the IR body that computes its
// initial value. `MoveGlobalVarInitializationToEntryPointsPass` separates those two roles. For
// example, it changes:
//
//      static int counter = 1;
//
//      [shader("compute")]
//      [numthreads(1, 1, 1)]
//      void computeMain()
//      {
//          int oldValue = counter++;
//      }
//
// into the equivalent form:
//
//      static int counter;
//
//      [shader("compute")]
//      [numthreads(1, 1, 1)]
//      void computeMain()
//      {
//          counter = 1;
//          int oldValue = counter++;
//      }
//
// We perform that transformation in three steps. We first choose the globals whose initializer
// bodies must move. We then move each selected body into a zero-argument function. Finally, we
// insert equivalent initialization at the start of each module-scope entry point selected by the
// caller's policy. A non-trivial body is evaluated by calling its new function; a body that only
// returns a module-scope value needs just a store.
//
// Two policies share these extraction and injection steps. `RequiredByTarget` selects initializer
// bodies that the target cannot represent at module scope and injects them into shader entry
// points. `RequiredByPerInvocationResourceReplacement` selects the marked resource globals that
// `legalizeResourceGlobalVars` will replace. A marked global represents either a mutable resource
// variable declared by the programmer at file or namespace scope, or a mutable uniform-parameter
// shadow synthesized for `-Gec`. This policy also injects the selected initializers into CUDA
// kernels. We move a resource initializer only after proving that the initializer and every
// function it can call return normally for all possible inputs, that they have no externally
// observable side effects, and that they do not read preexisting mutable storage, resource
// contents, or non-resource data from an explicit parameter group. Those restrictions ensure that
// evaluating the initializer at the start of each entry point cannot change observable behavior.

/// A `GlobalInitializerSelection` identifies why an initializer body is selected for execution
/// inside an entry point.
enum class GlobalInitializerSelection
{
    /// Initializers that the current target cannot represent at module scope.
    RequiredByTarget,

    /// Resource initializers required by per-invocation resource replacement.
    RequiredByPerInvocationResourceReplacement,
};

/// A `MoveGlobalVarInitializationToEntryPointsPass` extracts selected initializer bodies and
/// inserts equivalent initialization into the selected module-scope entry-point definitions.
struct MoveGlobalVarInitializationToEntryPointsPass
{
    /// An `ExtractedInitializer` pairs global storage with the function that computes its value.
    struct ExtractedInitializer
    {
        /// The storage that will receive the extracted initializer's result.
        IRGlobalVar* globalVar = nullptr;

        /// The new function that contains the initializer's original body.
        IRFunc* function = nullptr;
    };

    /// The module being transformed.
    IRModule* m_module = nullptr;

    /// The target program whose rules and capabilities apply to this transformation.
    TargetProgram* m_targetProgram = nullptr;

    /// The sink used to diagnose resource initializers that the analysis cannot prove safe to move.
    DiagnosticSink* m_sink = nullptr;

    /// The policy that decides which initializer bodies to extract.
    GlobalInitializerSelection m_selection = GlobalInitializerSelection::RequiredByTarget;

    /// The extracted initializers in their original module order.
    List<ExtractedInitializer> m_extractedInitializers;

    /// Move the selected initializer bodies and initialize their variables in selected entry
    /// points.
    void processModule(
        IRModule* module,
        TargetProgram* targetProgram,
        GlobalInitializerSelection selection,
        DiagnosticSink* sink = nullptr)
    {
        // For `RequiredByPerInvocationResourceReplacement`, we validate every selected initializer
        // before modifying the module, so a diagnostic cannot leave a partial transformation. We
        // then extract all selected bodies before injecting any of them, which gives every selected
        // entry point the same complete list in module order.
        m_module = module;
        m_targetProgram = targetProgram;
        SLANG_RELEASE_ASSERT(m_targetProgram);
        m_selection = selection;
        m_sink = sink;

        if (m_selection == GlobalInitializerSelection::RequiredByPerInvocationResourceReplacement)
        {
            if (diagnoseResourceInitializersNotProvenSafeToMove())
                return;
        }

        extractSelectedInitializers();
        injectInitializersIntoEntryPoints();
    }

    /// Return whether the target requires this initializer to execute inside an entry point.
    bool isInitializerRequiredByTarget(IRGlobalVar* globalVar)
    {
        // Non-HLSL target pipelines express global initialization as ordinary calls at each entry
        // point. HLSL emission can retain declaration initializers except for the cases below.
        if (!isHLSLBasedTarget(m_targetProgram->getTargetReq()->getTarget()))
            return true;

        auto valueType = getGlobalVarValueType(globalVar);

        // Consider `uniform uint input[2]; static uint copy[2] = input;`. We must create the
        // initializer function before array-return legalization can replace its return value with
        // an `out` parameter.
        if (as<IRArrayType>(valueType))
            return true;

        // DXC cannot initialize a global `CoopVector` by calling a constructor with arguments.
        // Moving the initializer executes that constructor outside the global declaration.
        if (as<IRCoopVectorType>(valueType))
            return true;

        return false;
    }

    /// Return whether `globalVar` is eligible and selected for extraction.
    bool shouldExtractInitializer(IRGlobalVar* globalVar)
    {
        // A declaration without an initializer body has no computation to move. `ActualGlobal`
        // storage persists across entry-point invocations, so injecting its initializer into each
        // entry point would reset shared state on every invocation.
        if (!globalVar->getFirstBlock() || as<IRActualGlobalRate>(globalVar->getRate()))
            return false;

        if (m_selection == GlobalInitializerSelection::RequiredByPerInvocationResourceReplacement)
            return isResourceGlobalCandidateForPerInvocationReplacement(globalVar);

        return isInitializerRequiredByTarget(globalVar);
    }

    /// Return whether every CFG path from `block` reaches an `IRReturn`, assuming each ordinary
    /// instruction completes.
    ///
    /// `blocksBeingAnalyzed` is the gray set for cycle detection, and `blocksProvenToReturn` is the
    /// black set. On success, the black set includes every block reachable from `block`; its
    /// contents are incomplete after failure.
    bool canProveEveryControlFlowPathFromBlockReachesReturn(
        IRBlock* block,
        HashSet<IRBlock*>& blocksBeingAnalyzed,
        HashSet<IRBlock*>& blocksProvenToReturn)
    {
        // An already-proved suffix needs no second traversal. Encountering a block that is still
        // being analyzed instead finds a reachable cycle. The cycle may terminate for particular
        // runtime values, but the CFG alone does not prove that every execution leaves it.
        if (blocksProvenToReturn.contains(block))
            return true;
        if (!blocksBeingAnalyzed.add(block))
            return false;

        // We require every successor path to reach `IRReturn`. A block with no successor must end
        // in an `IRReturn`; `IRUnreachable`, generic assembly, and every other terminal form fail
        // the proof.
        bool reachesReturn = true;
        bool hasSuccessor = false;
        for (auto successor : block->getSuccessors())
        {
            hasSuccessor = true;
            if (!canProveEveryControlFlowPathFromBlockReachesReturn(
                    successor,
                    blocksBeingAnalyzed,
                    blocksProvenToReturn))
            {
                reachesReturn = false;
                break;
            }
        }
        if (!hasSuccessor && !as<IRReturn>(block->getTerminator()))
            reachesReturn = false;

        blocksBeingAnalyzed.remove(block);
        if (reachesReturn)
            blocksProvenToReturn.add(block);
        return reachesReturn;
    }

    /// Return whether every reachable CFG path in `code` reaches an `IRReturn`, assuming each
    /// ordinary instruction completes.
    ///
    /// On success, `reachableBlocksProvenToReturn` contains all and only the blocks reachable from
    /// `code`'s entry. Its contents are unspecified after failure. The caller uses this set to
    /// exclude unreachable instructions from the later effect and call analysis.
    bool canProveEveryControlFlowPathReachesReturn(
        IRGlobalValueWithCode* code,
        HashSet<IRBlock*>& reachableBlocksProvenToReturn)
    {
        // We start at the entry block so that unreachable blocks do not affect the result. We
        // reject every reachable cycle because the IR has no termination proof that would let us
        // distinguish a bounded loop from a diverging one. The later instruction scan separately
        // proves that every call on these paths returns normally.
        auto entryBlock = code->getFirstBlock();
        if (!entryBlock)
            return false;

        HashSet<IRBlock*> blocksBeingAnalyzed;
        return canProveEveryControlFlowPathFromBlockReachesReturn(
            entryBlock,
            blocksBeingAnalyzed,
            reachableBlocksProvenToReturn);
    }

    /// Cache the functions whose initializer-safety proofs are complete and detect call cycles.
    struct InitializerSafetyAnalysis
    {
        HashSet<IRFunc*> functionsBeingAnalyzed;
        HashSet<IRFunc*> functionsProvenSafe;
    };

    /// The facts about storage that an address may identify.
    struct AddressProvenance
    {
        /// Every address represented by this result is rooted at an `IRVar` in the computation.
        bool isKnownComputationLocal = false;

        /// The address may identify mutable storage created outside the current computation.
        bool mayReferToPreexistingMutableStorage = false;

        /// The address may identify the contents of a texture or buffer resource.
        bool mayReferToResourceContents = false;

        /// The address may identify storage in an explicit parameter group.
        bool mayReferToExplicitParameterGroupStorage = false;

        /// Return the neutral value for combining a set of possible address sources.
        static AddressProvenance forNoAlternatives() { return {.isKnownComputationLocal = true}; }

        /// Include `other` as another possible address source.
        ///
        /// The receiver must start with `forNoAlternatives()` so that the first alternative can
        /// determine whether every possible address is computation-local.
        void addAlternative(AddressProvenance other)
        {
            isKnownComputationLocal &= other.isKnownComputationLocal;
            mayReferToPreexistingMutableStorage |= other.mayReferToPreexistingMutableStorage;
            mayReferToResourceContents |= other.mayReferToResourceContents;
            mayReferToExplicitParameterGroupStorage |=
                other.mayReferToExplicitParameterGroupStorage;
        }
    };

    /// Return facts about the storage that `value` may address.
    AddressProvenance analyzeAddressProvenance(IRInst* value, HashSet<IRInst*>& visited)
    {
        // We distinguish mutable storage from resource contents because even a read-only resource
        // can produce different data when read at a different point in program execution. We
        // follow address projections, pointer-to-pointer casts, block arguments, selects, and
        // defined global constants toward their possible root storage. A function parameter or any
        // other pointer-producing instruction may refer to mutable storage created outside the
        // current computation, so we reject it conservatively. The visited set bounds the walk when
        // a loop carries an address through a block parameter.
        auto type = as<IRType>(unwrapAttributedType(value->getDataType()));
        bool isPointerType = as<IRPtrTypeBase>(type) != nullptr;
        auto parameterGroupType = as<IRParameterGroupType>(type);
        bool isResourceStorageType =
            as<IRGLSLShaderStorageBufferType>(type) || as<IRHLSLStructuredBufferTypeBase>(type);
        bool isTraceableAddressType = isPointerType;
        if (parameterGroupType)
            isTraceableAddressType = true;
        if (isResourceStorageType)
            isTraceableAddressType = true;
        if (!isTraceableAddressType)
        {
            // Only the recognized pointer and storage-root types below expose enough structure to
            // trace an address to its origin. For any other type, we cannot prove that the address
            // refers to storage created by this computation, so we conservatively classify it as
            // preexisting mutable storage.
            return {.mayReferToPreexistingMutableStorage = true};
        }
        if (!visited.add(value))
        {
            // Reaching the same value again closes a cycle through block parameters. The repeated
            // edge adds no new address source, so it is neutral when we combine the cycle's other
            // incoming values.
            return {.isKnownComputationLocal = true};
        }

        if (as<IRVar>(value))
            return {.isKnownComputationLocal = true};
        if (as<IRGlobalVar>(value))
            return {.mayReferToPreexistingMutableStorage = true};

        if (isResourceStorageType)
        {
            // Structured buffers and GLSL shader-storage blocks use non-pointer IR types as root
            // addresses. Any load through those roots reads resource contents.
            return {
                .mayReferToPreexistingMutableStorage = !isPointerToImmutableLocation(value),
                .mayReferToResourceContents = true,
            };
        }

        if (parameterGroupType)
        {
            // Consider an initializer such as
            //
            //     uniform Texture2D textures[4];
            //     uniform uint textureIndex;
            //     static Texture2D cachedTexture = textures[textureIndex];
            //
            // The entry-point invocation receives `textureIndex` as an immutable parameter, and
            // the moved initializer executes after that parameter is available. By this point,
            // lowering may have packed the parameter into a synthesized parameter group. That
            // representation does not make the read depend on execution order. We therefore
            // preserve the individual shader parameter's classification instead of treating the
            // compiler-generated container as an explicit parameter group. A mutable group still
            // sets `mayReferToPreexistingMutableStorage` and is rejected by the caller.
            auto elementType = parameterGroupType->getElementType();
            if (elementType && elementType->findDecoration<IRSynthesizedParameterGroupDecoration>())
            {
                return {
                    .mayReferToPreexistingMutableStorage = !isPointerToImmutableLocation(value),
                };
            }

            // A parameter group acts as the root address for its fields even though its IR type is
            // not an `IRPtrTypeBase`. We record parameter-group storage separately because loading
            // a field containing one resource value is safe, while loading non-resource data reads
            // that storage.
            return {
                .mayReferToPreexistingMutableStorage = !isPointerToImmutableLocation(value),
                .mayReferToExplicitParameterGroupStorage = true,
            };
        }

        if (doesOpProduceResourceContentAddress(value->getOp()))
        {
            // Both read-only and writable resource addresses identify resource contents. Writable
            // contents also count as preexisting mutable storage, while a read-only resource does
            // not.
            return {
                .mayReferToPreexistingMutableStorage = !isPointerToImmutableLocation(value),
                .mayReferToResourceContents = true,
            };
        }
        if (auto globalConstant = as<IRGlobalConstant>(value))
        {
            // A constant pointer may still name mutable storage. When an `IRGlobalConstant` has a
            // definition, we follow its initializer value. An imported constant has no definition
            // to inspect, so we conservatively assume that a pointer-valued import can name mutable
            // storage.
            auto constantValue = globalConstant->getValue();
            return constantValue ? analyzeAddressProvenance(constantValue, visited)
                                 : AddressProvenance{.mayReferToPreexistingMutableStorage = true};
        }

        if (auto parameter = as<IRParam>(value))
        {
            // We can analyze a parameter only when it belongs to a non-entry block in a code
            // object: its values then come from predecessor branches. Any other `IRParam` may have
            // been supplied from outside this computation, or lacks the CFG relationship that this
            // analysis needs, so we classify it conservatively. For a non-entry block parameter,
            // we find its position in the block and combine the corresponding argument from every
            // predecessor as an alternative address source.
            auto block = as<IRBlock>(parameter->getParent());
            if (!block)
                return {.mayReferToPreexistingMutableStorage = true};

            auto parentCode = as<IRGlobalValueWithCode>(block->getParent());
            if (!parentCode || block == parentCode->getFirstBlock())
                return {.mayReferToPreexistingMutableStorage = true};

            // The parameter index selects the corresponding argument on each incoming branch.
            auto parameterIndex = getParamIndexInBlock(parameter);
            if (parameterIndex < 0)
                return {.mayReferToPreexistingMutableStorage = true};

            // Every predecessor must provide an argument at that index. We also require at least
            // one predecessor; otherwise there is no address source that the analysis can prove.
            AddressProvenance result = AddressProvenance::forNoAlternatives();
            bool foundPredecessor = false;
            for (auto predecessor : block->getPredecessors())
            {
                foundPredecessor = true;
                auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                if (!branch || UInt(parameterIndex) >= branch->getArgCount())
                    return {.mayReferToPreexistingMutableStorage = true};
                result.addAlternative(
                    analyzeAddressProvenance(branch->getArg(UInt(parameterIndex)), visited));
            }
            return foundPredecessor
                       ? result
                       : AddressProvenance{.mayReferToPreexistingMutableStorage = true};
        }

        // An access through an address projection or pointer-to-pointer cast can depend on storage
        // reached through operand zero. An l-value implicit cast may use a temporary, but its
        // copy-in or copy-out transfers the relevant access to operand zero. We therefore follow
        // only that operand; an index does not contribute address provenance. An
        // integer-to-pointer cast does not qualify because it may manufacture an address of
        // arbitrary preexisting storage.
        if (value->getOperandCount() != 0 &&
            doesUserResultTransferStorageAccessFromUse(value->getOperandUse(0)))
            return analyzeAddressProvenance(value->getOperand(0), visited);

        if (as<IRSelect>(value))
        {
            // A select may produce any address supplied by its two result operands. We combine both
            // alternatives because the condition does not prove which address reaches the load.
            AddressProvenance result = AddressProvenance::forNoAlternatives();
            for (UInt operandIndex = 1; operandIndex < value->getOperandCount(); ++operandIndex)
            {
                auto operand = value->getOperand(operandIndex);
                SLANG_RELEASE_ASSERT(operand);
                result.addAlternative(analyzeAddressProvenance(operand, visited));
            }
            return result;
        }

        // No remaining instruction kind proves that its result points to storage created by this
        // computation. In particular, a `ReadNone` call may return the address of a global or the
        // address of an element in a resource without reading that storage itself. An immutable
        // pointer prevents writes through that pointer; it does not identify the storage that the
        // pointer addresses.
        return {.mayReferToPreexistingMutableStorage = true};
    }

    /// Return whether `function` has an ordinary body that completely describes its behavior for
    /// the current target.
    bool canInspectFunctionBodyForInitializerSafety(IRFunc* function)
    {
        // A declaration has no body from which we can prove effects or normal completion. An
        // applicable target intrinsic or generic-assembly implementation replaces the ordinary
        // body during emission, so inspecting that body would not establish either property of the
        // emitted program.
        if (!function)
            return false;
        if (!function->getFirstBlock())
            return false;
        if (findBestTargetIntrinsicDecoration(
                function,
                m_targetProgram->getTargetReq()->getTargetCaps()))
            return false;
        if (hasGenericAssemblyImplementation(function))
            return false;
        return true;
    }

    /// Return whether `inst` stores only to mutable storage created inside the computation.
    bool isPermittedStoreToComputationLocalStorage(IRInst* inst)
    {
        // A `ReadNone` function may still use `IRStore` or `IRSwizzledStore` for temporary storage.
        // We accept a store only when every possible destination is rooted at an `IRVar` created by
        // the initializer computation. Moving such a store changes when it executes, but no code
        // outside that computation can observe its destination.
        IRInst* writtenAddress = nullptr;
        if (auto store = as<IRStore>(inst))
            writtenAddress = store->getPtr();
        else if (as<IRSwizzledStore>(inst))
            writtenAddress = inst->getOperand(0);
        else
            return false;

        HashSet<IRInst*> visited;
        return analyzeAddressProvenance(writtenAddress, visited).isKnownComputationLocal;
    }

    /// Return whether executing `code` as part of an initializer at the start of an entry point
    /// could change observable behavior.
    bool mayComputationDependOnExecutionOrder(
        IRGlobalValueWithCode* code,
        InitializerSafetyAnalysis& analysis)
    {
        // Moving a computation can change which later initializers execute when that computation
        // fails to return. We first require every reachable CFG path to reach `IRReturn`. This
        // intraprocedural proof assumes ordinary instructions complete; the call analysis below
        // recursively proves that assumption for each reachable call.
        HashSet<IRBlock*> reachableBlocks;
        if (!canProveEveryControlFlowPathReachesReturn(code, reachableBlocks))
            return true;

        // We then reject an instruction unless its existing effect information is sufficient to
        // prove that moving it is safe. In particular, we reject reads from preexisting mutable
        // storage, reads of resource contents, reads of non-resource data from an explicit
        // parameter group, and every side effect except the local-store forms recognized above.
        //
        // A defined callee gives us more information than its effect decoration alone. We inspect
        // its body so that a resource-content read cannot hide behind a `ReadNone` decoration. If
        // we cannot inspect the callee, we cannot prove either its effects or its completion
        // behavior, so we reject the initializer.
        for (auto block : code->getBlocks())
        {
            if (!reachableBlocks.contains(block))
                continue;

            auto terminator = block->getTerminator();
            for (auto inst = block->getFirstOrdinaryInst(); inst && inst != terminator;
                 inst = inst->getNextInst())
            {
                IRInst* loadedAddress = nullptr;
                if (auto load = as<IRLoad>(inst))
                    loadedAddress = load->getPtr();
                else if (auto atomicLoad = as<IRAtomicLoad>(inst))
                    loadedAddress = atomicLoad->getPtr();

                if (loadedAddress)
                {
                    HashSet<IRInst*> visited;
                    auto provenance = analyzeAddressProvenance(loadedAddress, visited);
                    // Loading one supported resource value, or a homogeneous fixed-size array of
                    // such values, copies resource identities without reading resource contents.
                    // Any other load can copy non-resource data from the parameter group.
                    bool readsExplicitParameterGroupData =
                        provenance.mayReferToExplicitParameterGroupStorage &&
                        !isResourceValueOrArrayTypeSupportedForPerInvocationReplacement(
                            inst->getDataType());
                    if (provenance.mayReferToPreexistingMutableStorage)
                        return true;
                    if (provenance.mayReferToResourceContents)
                        return true;
                    if (readsExplicitParameterGroupData)
                        return true;
                }

                if (isDebugInfoInst(inst))
                    continue;

                if (doesOpReadResourceContents(inst->getOp()))
                    return true;

                if (auto call = as<IRCall>(inst))
                {
                    // A defined function's IR blocks give us the most precise account of the
                    // callee's effects. We inspect those blocks only when they supply the emitted
                    // implementation for this target. Applicable target-intrinsic text and generic
                    // assembly supply a different implementation, and neither representation
                    // exposes effects this analysis can classify.
                    auto callee = as<IRFunc>(getResolvedInstForDecorations(call->getCallee()));
                    if (!canInspectFunctionBodyForInitializerSafety(callee))
                    {
                        // Effect decorations do not promise that a call returns. Without the
                        // emitted body, we cannot prove both the effects and completion behavior
                        // needed to move the initializer.
                        return true;
                    }

                    if (analysis.functionsProvenSafe.contains(callee))
                        continue;

                    // Re-entering a function finds a recursive call cycle. The cycle may terminate
                    // for particular arguments, but the IR supplies no proof that every invocation
                    // returns.
                    if (!analysis.functionsBeingAnalyzed.add(callee))
                        return true;
                    bool calleeMayDependOnExecutionOrder =
                        mayComputationDependOnExecutionOrder(callee, analysis);
                    analysis.functionsBeingAnalyzed.remove(callee);
                    if (calleeMayDependOnExecutionOrder)
                        return true;

                    analysis.functionsProvenSafe.add(callee);
                    continue;
                }
                if (inst->mightHaveSideEffects() &&
                    !isPermittedStoreToComputationLocalStorage(inst))
                {
                    return true;
                }
            }
        }
        return false;
    }

    /// Return whether evaluating `globalVar`'s initializer at the start of each entry point rather
    /// than during global initialization could change observable behavior.
    bool mayResourceInitializerMovementChangeObservableBehavior(IRGlobalVar* globalVar)
    {
        // The analysis proves each reachable defined callee once and rejects a recursive call cycle
        // instead of assuming that the re-entered call returns.
        InitializerSafetyAnalysis analysis;
        return mayComputationDependOnExecutionOrder(globalVar, analysis);
    }

    /// Diagnose resource initializers that the analysis cannot prove safe to move, and return
    /// whether any diagnostic was emitted.
    bool diagnoseResourceInitializersNotProvenSafeToMove()
    {
        // `RequiredByPerInvocationResourceReplacement` moves selected resource initializers to the
        // start of each entry point while leaving every non-selected initializer at module scope.
        // We accept a selected initializer only when that move, including the change in its order
        // relative to initializers left at module scope, cannot change observable behavior. We
        // check every selected initializer before changing the module and report all violations
        // together.
        SLANG_RELEASE_ASSERT(m_sink);
        bool diagnosed = false;
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !shouldExtractInitializer(globalVar))
                continue;
            if (!mayResourceInitializerMovementChangeObservableBehavior(globalVar))
                continue;

            m_sink->diagnose(Diagnostics::CannotProveMutableResourceInitializerSafeToMove{
                .variable = globalVar,
                .location = globalVar->sourceLoc});
            diagnosed = true;
        }
        return diagnosed;
    }

    /// Extract every selected initializer body, preserving module order.
    void extractSelectedInitializers()
    {
        // We visit globals in module order so that each entry point executes the selected
        // initializers in their original relative order. The same order also makes generated code
        // deterministic.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !shouldExtractInitializer(globalVar))
                continue;

            extractInitializer(globalVar);
        }
    }

    /// Move `globalVar`'s initializer body into a new zero-argument function.
    void extractInitializer(IRGlobalVar* globalVar)
    {
        // We first create a function whose result type is the value stored by `globalVar`. We then
        // move the initializer blocks into that function, leaving only the global storage
        // declaration behind.
        IRBuilder builder(m_module);
        builder.setInsertBefore(globalVar);

        auto valueType = getGlobalVarValueType(globalVar);
        auto function = builder.createFunc();
        function->setFullType(builder.getFuncType(0, nullptr, valueType));

        // We move the blocks instead of cloning them. The global retains only its storage, while
        // the new function retains the initializer's original instructions and control flow.
        IRBlock* nextBlock = nullptr;
        for (IRBlock* block = globalVar->getFirstBlock(); block; block = nextBlock)
        {
            nextBlock = block->getNextBlock();
            block->removeFromParent();
            block->insertAtEnd(function);
        }

        m_extractedInitializers.add({globalVar, function});
    }

    /// Return whether `function` receives the initializers selected by this pass invocation.
    bool isSelectedEntryPoint(IRFunc* function)
    {
        // Target-based selection injects initializers into shader entry points. Resource selection
        // also includes CUDA kernels because `legalizeResourceGlobalVars` creates replacement
        // locals in those kernels.
        if (m_selection == GlobalInitializerSelection::RequiredByPerInvocationResourceReplacement)
            return isShaderOrCudaKernelEntryPoint(function);
        return function->findDecoration<IREntryPointDecoration>() != nullptr;
    }

    /// Insert the extracted initializers into every selected module-scope entry-point definition.
    void injectInitializersIntoEntryPoints()
    {
        // We scan the linked module's direct children and inject the same ordered initializer list
        // into each module-scope entry-point definition selected by the current policy. An
        // entry-point declaration has no block in which we can insert initialization, so we skip
        // it.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto function = as<IRFunc>(inst);
            if (!function || !isSelectedEntryPoint(function))
                continue;
            if (!function->getFirstBlock())
                continue;

            injectInitializersAtEntryPoint(function);
        }
    }

    /// Return a module-scope result when the initializer body contains only its return.
    IRInst* findResultOfReturnOnlyInitializer(IRFunc* function)
    {
        // We may omit a call only when the function contains a single return and that return uses a
        // module-scope value. Checking the entire body matters: an initializer such as
        // `(++counter, inputTexture)` returns a module-scope value, but the increment must still
        // execute.
        auto block = function->getFirstBlock();
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

    /// Insert one call and store, or one direct store, for each extracted initializer.
    void injectInitializersAtEntryPoint(IRFunc* entryPoint)
    {
        // We insert before the first ordinary instruction so that every selected initializer
        // completes before the entry point can read or write the corresponding global.
        auto firstBlock = entryPoint->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        IRBuilder builder(m_module);
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());

        for (auto initializer : m_extractedInitializers)
        {
            auto initialValue = findResultOfReturnOnlyInitializer(initializer.function);
            if (!initialValue)
            {
                auto valueType = getGlobalVarValueType(initializer.globalVar);
                initialValue = builder.emitCallInst(valueType, initializer.function, 0, nullptr);
            }
            builder.emitStore(initializer.globalVar, initialValue);
        }
    }
};

void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram)
{
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(module, targetProgram, GlobalInitializerSelection::RequiredByTarget);
}

void moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink)
{
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(
        module,
        targetProgram,
        GlobalInitializerSelection::RequiredByPerInvocationResourceReplacement,
        sink);
}

} // namespace Slang
