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
// Two modes share these extraction and injection steps. `RequiredByTarget` selects initializer
// bodies that the target cannot represent at module scope and injects them into shader entry
// points. `FileOrNamespaceScopeStaticResources` selects only the resource-valued `static`
// variables declared at file or namespace scope that `legalizeResourceGlobalVars` will replace. It
// also injects their initializers into CUDA kernels. In that mode, we move an initializer only
// after the analysis proves that it has no externally observable side effects and does not read
// preexisting mutable storage, resource contents, or non-resource data from a source-declared
// parameter group. Moving such a computation to the start of each entry point cannot change
// observable behavior.

/// A `GlobalInitializerSelection` identifies why an initializer body is selected for execution
/// inside an entry point.
enum class GlobalInitializerSelection
{
    /// Initializers that the current target cannot represent at module scope.
    RequiredByTarget,

    /// Resource initializers for `static` variables declared at file or namespace scope.
    FileOrNamespaceScopeStaticResources,
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

    /// The sink used to diagnose resource initializers that cannot safely be moved.
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
        // For `FileOrNamespaceScopeStaticResources`, we validate every selected initializer before
        // modifying the module, so a diagnostic cannot leave a partial transformation. We then
        // extract all selected bodies before injecting any of them, which gives every selected
        // entry point the same complete list in module order.
        m_module = module;
        m_targetProgram = targetProgram;
        SLANG_RELEASE_ASSERT(m_targetProgram);
        m_selection = selection;
        m_sink = sink;

        if (m_selection == GlobalInitializerSelection::FileOrNamespaceScopeStaticResources &&
            diagnoseResourceInitializersThatCannotBeMovedSafely())
        {
            return;
        }

        extractSelectedInitializers();
        injectInitializersIntoEntryPoints();
    }

    /// Return whether the target requires this initializer to execute inside an entry point.
    bool isInitializerRequiredByTarget(IRGlobalVar* globalVar)
    {
        // HLSL can represent global initializers at module scope except when an initializer
        // constructs a cooperative vector, which DXC rejects. We therefore select only
        // cooperative-vector initializers for HLSL. Every initializer body that reaches this
        // helper is selected for other targets.
        SLANG_RELEASE_ASSERT(m_targetProgram);
        if (isD3DTarget(m_targetProgram->getTargetReq()))
        {
            auto valueType = globalVar->getDataType()->getValueType();
            return as<IRCoopVectorType>(valueType) != nullptr;
        }
        return true;
    }

    /// Return whether `globalVar` is eligible and selected for extraction.
    bool shouldExtractInitializer(IRGlobalVar* globalVar)
    {
        // A declaration without an initializer body has no computation to move. `ActualGlobal`
        // storage persists across entry-point invocations, so injecting its initializer into each
        // entry point would reset shared state on every invocation.
        if (!globalVar->getFirstBlock() || as<IRActualGlobalRate>(globalVar->getRate()))
            return false;

        if (m_selection == GlobalInitializerSelection::FileOrNamespaceScopeStaticResources)
            return isFileOrNamespaceScopeStaticResourceGlobalToReplace(globalVar);

        return isInitializerRequiredByTarget(globalVar);
    }

    /// The facts about storage that an address may identify.
    struct AddressProvenance
    {
        /// Every address represented by this result is rooted at an `IRVar` in the computation.
        bool isKnownComputationLocal = false;

        /// The address may identify mutable storage created outside the current computation.
        bool mayReferToPreexistingMutableStorage = false;

        /// The address may identify the contents of a texture or buffer resource.
        bool mayReferToResourceContents = false;

        /// The address may identify storage in a parameter group.
        bool mayReferToParameterGroupStorage = false;

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
            mayReferToParameterGroupStorage |= other.mayReferToParameterGroupStorage;
        }
    };

    /// Return whether `type` represents one resource value rather than aggregate data.
    bool isSingleResourceValueType(IRType* type)
    {
        // A parameter group may store a texture, sampler, or buffer value as a field. Loading that
        // field copies the resource value; it does not read the resource's contents. We
        // intentionally exclude arrays and structs because loading an aggregate could also read
        // non-resource data from the parameter group.
        type = as<IRType>(unwrapAttributedType(type));
        return as<IRResourceTypeBase>(type) || as<IRSamplerStateTypeBase>(type) ||
               as<IRHLSLStructuredBufferTypeBase>(type) || as<IRByteAddressBufferTypeBase>(type);
    }

    /// Return facts about the storage that `value` may address.
    AddressProvenance getAddressProvenance(IRInst* value, HashSet<IRInst*>& visited)
    {
        // We distinguish mutable storage from resource contents because even a read-only resource
        // can produce different data when read at a different point in program execution. We
        // follow address projections, pointer-to-pointer casts, block arguments, selects, and
        // defined global constants toward their possible root storage. A function parameter or any
        // other pointer-producing instruction may refer to mutable storage created outside the
        // current computation, so we reject it conservatively. The visited set bounds the walk when
        // a loop carries an address through a block parameter.
        auto type = as<IRType>(unwrapAttributedType(value->getDataType()));
        bool hasPtrTypeBase = as<IRPtrTypeBase>(type) != nullptr;
        auto parameterGroupType = as<IRParameterGroupType>(type);
        bool hasParameterGroupType = parameterGroupType != nullptr;
        bool hasResourceStorageType =
            as<IRGLSLShaderStorageBufferType>(type) || as<IRHLSLStructuredBufferTypeBase>(type);
        if (!hasPtrTypeBase && !hasParameterGroupType && !hasResourceStorageType)
        {
            // Slang IR has pointer-like types that are not `IRPtrTypeBase`, including
            // `BindExistentials<Ptr<T>, ...>`. This analysis does not yet trace values of those
            // types to an address root, so we conservatively treat them as references to
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

        if (hasResourceStorageType)
        {
            // Structured buffers and GLSL shader-storage blocks use non-pointer IR types as root
            // addresses. Any load through those roots reads resource contents.
            return {
                .mayReferToPreexistingMutableStorage = !isPointerToImmutableLocation(value),
                .mayReferToResourceContents = true,
            };
        }

        if (hasParameterGroupType)
        {
            // The compiler packs source-level global uniform parameters into one synthesized
            // parameter group. Those fields retain the semantics of the original parameters; the
            // generated container does not turn a uniform read into a source-level resource read.
            // We therefore classify only whether the synthesized group is mutable, without marking
            // its fields as parameter-group storage.
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
                .mayReferToParameterGroupStorage = true,
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
            return constantValue ? getAddressProvenance(constantValue, visited)
                                 : AddressProvenance{.mayReferToPreexistingMutableStorage = true};
        }

        if (auto parameter = as<IRParam>(value))
        {
            auto block = as<IRBlock>(parameter->getParent());
            if (!block)
                return {.mayReferToPreexistingMutableStorage = true};

            // A parameter in the entry block is supplied by the caller rather than by a CFG edge.
            // Its address may therefore refer to storage outside this computation.
            auto parentCode = as<IRGlobalValueWithCode>(block->getParent());
            if (!parentCode || block == parentCode->getFirstBlock())
                return {.mayReferToPreexistingMutableStorage = true};

            auto parameterIndex = getParamIndexInBlock(parameter);
            if (parameterIndex < 0)
                return {.mayReferToPreexistingMutableStorage = true};

            AddressProvenance result = AddressProvenance::forNoAlternatives();
            bool foundPredecessor = false;
            for (auto predecessor : block->getPredecessors())
            {
                foundPredecessor = true;
                auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                if (!branch || UInt(parameterIndex) >= branch->getArgCount())
                    return {.mayReferToPreexistingMutableStorage = true};
                result.addAlternative(
                    getAddressProvenance(branch->getArg(UInt(parameterIndex)), visited));
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
        if (value->getOperandCount() != 0 && mayUseTransferStorageAccess(value->getOperandUse(0)))
            return getAddressProvenance(value->getOperand(0), visited);

        if (as<IRSelect>(value))
        {
            // A select may produce any address supplied by its two result operands. We combine both
            // alternatives because the condition does not prove which address reaches the load.
            AddressProvenance result = AddressProvenance::forNoAlternatives();
            for (UInt operandIndex = 1; operandIndex < value->getOperandCount(); ++operandIndex)
            {
                auto operand = value->getOperand(operandIndex);
                SLANG_RELEASE_ASSERT(operand);
                result.addAlternative(getAddressProvenance(operand, visited));
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

    /// Return whether `type` is opaque after removing pointer and parameter-direction wrappers.
    bool isOpaqueValueOrAddressType(IRType* type)
    {
        // An argument for `out`, `inout`, `ref`, or `__constref` is an address of the source value.
        // We follow every pointer wrapper so that an applicable target intrinsic cannot hide an
        // opaque argument behind its parameter's calling convention.
        type = as<IRType>(unwrapAttributedType(type));
        while (auto pointerType = as<IRPtrTypeBase>(type))
            type = as<IRType>(unwrapAttributedType(pointerType->getValueType()));
        return type && isOpaqueType(type, nullptr);
    }

    /// Return whether an argument to `call` is, or points to, an opaque value.
    bool doesCallPassOpaqueValue(IRCall* call)
    {
        // A target intrinsic can be marked `ReadNone` even when it reads an opaque value through an
        // argument. `Texture.Load` is one example. Without an inspectable implementation, we cannot
        // determine which intrinsics read their arguments in this way. We therefore reject an
        // opaque value passed either directly or through an address, including an aggregate that
        // contains an opaque value.
        for (auto argument : call->getArgsList())
        {
            auto type = as<IRType>(unwrapAttributedType(argument->getDataType()));
            if (isOpaqueValueOrAddressType(type))
                return true;
        }
        return false;
    }

    /// Return whether `function` contains generic target assembly whose effects cannot be
    /// inspected.
    bool doesFunctionContainGenericAssembly(IRFunc* function)
    {
        // `IRGenericAsm` is a block terminator, so the instruction loop in
        // `mayComputationDependOnExecutionOrder` does not visit it. Its text does not provide a
        // structured memory or resource effect that this analysis can inspect, so we look for it
        // explicitly. Translation-unit lowering has already removed error-handling and `defer`
        // terminators. Apart from `IRGenericAsm`, the remaining terminators only describe control
        // flow.
        for (auto block : function->getBlocks())
        {
            if (as<IRGenericAsm>(block->getTerminator()))
                return true;
        }
        return false;
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
        return getAddressProvenance(writtenAddress, visited).isKnownComputationLocal;
    }

    /// Return whether executing `code` as part of an initializer at the start of an entry point
    /// could change observable behavior.
    bool mayComputationDependOnExecutionOrder(
        IRGlobalValueWithCode* code,
        HashSet<IRFunc*>& visitedFunctions)
    {
        // We reject an instruction unless its existing effect information is sufficient to prove
        // that moving it is safe. In particular, we reject reads from preexisting mutable storage,
        // reads of resource contents, reads of non-resource data from a source-declared parameter
        // group, and every side effect except the local-store forms recognized above.
        //
        // A defined callee gives us more information than its effect decoration alone. We inspect
        // its body so that a resource-content read cannot hide behind a `ReadNone` decoration. If
        // we cannot inspect the callee, we treat an opaque resource argument as a possible content
        // read.
        for (auto block : code->getBlocks())
        {
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
                    auto provenance = getAddressProvenance(loadedAddress, visited);
                    bool readsParameterGroupData = provenance.mayReferToParameterGroupStorage &&
                                                   !isSingleResourceValueType(inst->getDataType());
                    if (provenance.mayReferToPreexistingMutableStorage ||
                        provenance.mayReferToResourceContents || readsParameterGroupData)
                    {
                        return true;
                    }
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
                    bool hasInspectableBody =
                        callee && callee->getFirstBlock() &&
                        !findBestTargetIntrinsicDecoration(
                            callee,
                            m_targetProgram->getTargetReq()->getTargetCaps()) &&
                        !doesFunctionContainGenericAssembly(callee);
                    if (hasInspectableBody)
                    {
                        if (visitedFunctions.add(callee) &&
                            mayComputationDependOnExecutionOrder(callee, visitedFunctions))
                        {
                            return true;
                        }
                        continue;
                    }

                    // Without an inspectable body, only `ReadNone` guarantees that the call does
                    // not read or change mutable state. Even a `ReadNone` target intrinsic can read
                    // resource contents through an opaque argument, so we reject that case too.
                    if (!isPureFunctionalCall(call) || doesCallPassOpaqueValue(call))
                        return true;
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
    bool mayResourceInitializerDependOnExecutionOrder(IRGlobalVar* globalVar)
    {
        // We inspect every reachable defined callee at most once. Revisiting a function through a
        // recursive cycle cannot reveal an instruction that its first visit did not inspect.
        HashSet<IRFunc*> visitedFunctions;
        return mayComputationDependOnExecutionOrder(globalVar, visitedFunctions);
    }

    /// Diagnose resource initializers that the analysis cannot prove safe to move, and return
    /// whether any diagnostic was emitted.
    bool diagnoseResourceInitializersThatCannotBeMovedSafely()
    {
        // `FileOrNamespaceScopeStaticResources` moves selected resource initializers to the start
        // of each entry point while leaving every non-selected initializer at module scope. We
        // accept a selected initializer only when that move, including the change in its order
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
            if (!mayResourceInitializerDependOnExecutionOrder(globalVar))
                continue;

            m_sink->diagnose(Diagnostics::ResourceStaticInitializerCannotBeMovedSafely{
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

        auto valueType = globalVar->getDataType()->getValueType();
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
        if (m_selection == GlobalInitializerSelection::FileOrNamespaceScopeStaticResources)
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
                auto valueType = initializer.globalVar->getDataType()->getValueType();
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
        GlobalInitializerSelection::FileOrNamespaceScopeStaticResources,
        sink);
}

} // namespace Slang
