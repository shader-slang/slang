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
//      void computeMain()
//      {
//          int oldValue = counter++;
//      }
//
// into the equivalent form:
//
//      static int counter;
//
//      void computeMain()
//      {
//          counter = 1;
//          int oldValue = counter++;
//      }
//
// We perform that transformation in three steps. We first choose the globals whose initializer
// bodies must move. We then move each selected body into a zero-argument function. Finally, we
// insert equivalent initialization at the start of every defined entry point. A non-trivial body is
// evaluated by calling its new function; a body that only returns a module-scope value needs just a
// store.
//
// Two callers use these extraction and injection steps. The established target-specific policy
// chooses ordinary initializers. Resource-global legalization moves only the file-scope `static`
// resource initializers that it will replace with entry-point locals. We accept a resource
// initializer only when it has no side effects and does not read mutable state. Its evaluation is
// then independent of every initializer that remains at global scope, so moving it cannot change
// their observable order.

/// A `GlobalInitializerSelection` chooses the policy used to select initializer bodies.
enum class GlobalInitializerSelection
{
    RequiredByTarget,
    FileScopeStaticResources,
};

/// A `MoveGlobalVarInitializationToEntryPointsPass` extracts the selected initializer bodies and
/// inserts equivalent initialization into every defined entry point.
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

    /// The target policy for `RequiredByTarget`, or null for the resource-only selection.
    TargetProgram* m_targetProgram = nullptr;

    /// The sink used to diagnose unsupported resource initializers.
    DiagnosticSink* m_sink = nullptr;

    /// The policy that decides which initializer bodies to extract.
    GlobalInitializerSelection m_selection = GlobalInitializerSelection::RequiredByTarget;

    /// The extracted initializers in their original module order.
    List<ExtractedInitializer> m_extractedInitializers;

    /// Move the selected initializer bodies and initialize their variables in every entry point.
    void processModule(
        IRModule* module,
        TargetProgram* targetProgram,
        GlobalInitializerSelection selection,
        DiagnosticSink* sink = nullptr)
    {
        // We first validate the selected resource initializers, because reporting an error must not
        // leave the module partly transformed. We then separate extraction from injection because
        // every entry point must execute the same selected initializers in the same order.
        m_module = module;
        m_targetProgram = targetProgram;
        m_selection = selection;
        m_sink = sink;

        if (m_selection == GlobalInitializerSelection::FileScopeStaticResources &&
            diagnoseOrderDependentResourceInitializers())
        {
            return;
        }

        extractSelectedInitializers();
        injectInitializersIntoEntryPoints();
    }

    /// Return whether the target requires this initializer to execute inside an entry point.
    bool isInitializerRequiredByTarget(IRGlobalVar* globalVar)
    {
        // HLSL can represent ordinary global initializers, so we leave them at global scope.
        // Cooperative-vector construction is the exception because DXC cannot construct a
        // cooperative vector in a global initializer. For other targets, this policy selects every
        // eligible global initializer body.
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
        // `ActualGlobal` storage has one lifetime outside individual shader invocations. Repeating
        // its initialization in each entry point would change that lifetime, so neither selection
        // policy may move `globalVar`.
        if (as<IRActualGlobalRate>(globalVar->getRate()))
            return false;

        if (m_selection == GlobalInitializerSelection::FileScopeStaticResources)
            return isFileScopeStaticResourceGlobalToReplace(globalVar);

        return isInitializerRequiredByTarget(globalVar);
    }

    /// Return whether `value` can address mutable state not created inside its enclosing
    /// computation.
    bool canAddressReferToPreexistingMutableState(IRInst* value, HashSet<IRInst*>& visited)
    {
        // Address casts, selects, and other forwarding instructions retain their operands in the
        // IR, so we can find the original storage by walking their operands. Block parameters need
        // one extra step: we follow each argument supplied by a predecessor. The visited set bounds
        // the walk when a loop passes an address through a block parameter.
        if (!visited.add(value))
            return false;
        if (as<IRGlobalVar>(value))
            return true;

        // A pointer into a writable resource denotes state supplied from outside the shader. That
        // state can change between the original initialization point and an entry-point call.
        if (as<IRRWStructuredBufferGetElementPtr>(value))
            return true;
        if (as<IRImageSubscript>(value) && !isPointerToImmutableLocation(value))
            return true;

        if (auto globalConstant = as<IRGlobalConstant>(value))
        {
            // A constant pointer may still name mutable storage. We follow a known value and treat
            // an imported constant pointer conservatively because its target is unavailable. A
            // non-pointer constant used as an address index does not itself name storage.
            auto type = globalConstant->getDataType();
            if (!type || !as<IRPtrTypeBase>(unwrapAttributedType(type)))
                return false;
            auto constantValue = globalConstant->getValue();
            return constantValue
                       ? canAddressReferToPreexistingMutableState(constantValue, visited)
                       : true;
        }

        if (auto parameter = as<IRParam>(value))
        {
            auto block = as<IRBlock>(parameter->getParent());
            if (!block)
                return false;

            auto parameterIndex = getParamIndexInBlock(parameter);
            if (parameterIndex < 0)
                return false;
            for (auto predecessor : block->getPredecessors())
            {
                auto branch = as<IRUnconditionalBranch>(predecessor->getTerminator());
                if (!branch || UInt(parameterIndex) >= branch->getArgCount())
                    return true;
                if (canAddressReferToPreexistingMutableState(
                        branch->getArg(UInt(parameterIndex)),
                        visited))
                {
                    return true;
                }
            }
            return false;
        }

        for (UInt operandIndex = 0; operandIndex < value->getOperandCount(); ++operandIndex)
        {
            auto operand = value->getOperand(operandIndex);
            if (operand && canAddressReferToPreexistingMutableState(operand, visited))
                return true;
        }

        // An otherwise unknown module-scope pointer may refer to imported mutable storage. A value
        // with no pointer type cannot be the address consumed by the load that started this walk.
        if (value->getParent() == m_module->getModuleInst())
        {
            auto type = value->getDataType();
            return type && as<IRPtrTypeBase>(unwrapAttributedType(type)) != nullptr;
        }
        return false;
    }

    /// Return whether an argument to `call` contains a resource value.
    bool doesCallPassResourceValue(IRCall* call)
    {
        // `ReadNone` excludes ordinary global-memory reads. For an opaque callee, the remaining
        // mutable state it could read is resource content reached through an argument. Target
        // intrinsics use `ReadNone` even for operations such as `Texture.Load`, so we reject an
        // opaque call that receives a resource value.
        for (auto argument : call->getArgsList())
        {
            auto type = as<IRType>(unwrapAttributedType(argument->getDataType()));
            if (type && isOpaqueType(type, nullptr))
                return true;
        }
        return false;
    }

    /// Return whether `function` contains target code whose effects are not represented in IR.
    bool doesFunctionContainOpaqueTargetCode(IRFunc* function)
    {
        // `IRGenericAsm` is a terminator, so it does not appear among a block's ordinary
        // instructions. We inspect every terminator explicitly before deciding that the function's
        // body can summarize a call.
        for (auto block : function->getBlocks())
        {
            if (as<IRGenericAsm>(block->getTerminator()))
                return true;
        }
        return false;
    }

    /// Return whether executing `code` at a different time can change program behavior.
    bool isComputationOrderDependent(
        IRGlobalValueWithCode* code,
        HashSet<IRFunc*>& visitedFunctions)
    {
        // Moving a computation changes when its side effects and reads occur. We reject any side
        // effect or read of mutable state. For a call to a defined function, we inspect its body so
        // that resource-content reads are not hidden behind a `ReadNone` summary. For an opaque
        // callee, passing a resource value is conservatively treated as a possible content read.
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
                    if (canAddressReferToPreexistingMutableState(loadedAddress, visited))
                        return true;
                }

                if (isDebugInfoInst(inst))
                    continue;

                if (isResourceLoadNotReportedAsSideEffecting(inst->getOp()))
                    return true;

                if (auto call = as<IRCall>(inst))
                {
                    // `NoSideEffect` allows a callee to read global state. Only `ReadNone`
                    // guarantees that ordinary memory reads cannot depend on initialization order.
                    if (!isPureFunctionalCall(call))
                        return true;

                    auto callee = as<IRFunc>(getResolvedInstForDecorations(call->getCallee()));
                    bool hasInspectableBody = callee && callee->getFirstBlock() &&
                                              !callee->findDecoration<IRTargetIntrinsicDecoration>() &&
                                              !doesFunctionContainOpaqueTargetCode(callee);
                    if (!hasInspectableBody)
                    {
                        if (doesCallPassResourceValue(call))
                            return true;
                        continue;
                    }

                    if (visitedFunctions.add(callee) &&
                        isComputationOrderDependent(callee, visitedFunctions))
                    {
                        return true;
                    }
                    continue;
                }
                if (inst->mightHaveSideEffects())
                    return true;
            }
        }
        return false;
    }

    /// Return whether `globalVar`'s initializer can observe when it executes.
    bool isResourceInitializerOrderDependent(IRGlobalVar* globalVar)
    {
        // We inspect every reachable defined callee at most once. Revisiting a function through a
        // recursive cycle cannot reveal an instruction that its first visit did not inspect.
        HashSet<IRFunc*> visitedFunctions;
        return isComputationOrderDependent(globalVar, visitedFunctions);
    }

    /// Diagnose resource initializers whose evaluation order can affect program behavior.
    bool diagnoseOrderDependentResourceInitializers()
    {
        // The resource-only policy moves an initializer without moving adjacent ordinary
        // initializers. We can preserve semantics only when that move cannot be observed. We check
        // every selected initializer before changing the module and report all violations together.
        SLANG_RELEASE_ASSERT(m_sink);
        bool diagnosed = false;
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !globalVar->getFirstBlock() || !shouldExtractInitializer(globalVar))
                continue;
            if (!isResourceInitializerOrderDependent(globalVar))
                continue;

            m_sink->diagnose(Diagnostics::ResourceStaticInitializerHasObservableEffect{
                .variable = globalVar,
                .location = globalVar->sourceLoc});
            diagnosed = true;
        }
        return diagnosed;
    }

    /// Extract every selected initializer body, preserving module order.
    void extractSelectedInitializers()
    {
        // We visit globals in module order because the injection step uses that same order at every
        // entry point. This choice retains the order already present in the IR and makes the
        // generated code deterministic.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar || !globalVar->getFirstBlock())
                continue;
            if (!shouldExtractInitializer(globalVar))
                continue;

            extractInitializer(globalVar);
        }
    }

    /// Move `globalVar`'s initializer body into a new zero-argument function.
    void extractInitializer(IRGlobalVar* globalVar)
    {
        // An `IRGlobalVar` has pointer type, but its initializer body returns the value stored
        // through that pointer. We therefore use the pointer's value type as the new function's
        // result type.
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

    /// Insert the extracted initializers into every defined entry point.
    void injectInitializersIntoEntryPoints()
    {
        // Entry-point declarations have no block in which to insert initialization, so only
        // definitions can be changed.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto function = as<IRFunc>(inst);
            if (!function || !function->findDecoration<IREntryPointDecoration>())
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
    // We retain the established target policy for the ordinary global-initializer pass.
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(
        module,
        targetProgram,
        GlobalInitializerSelection::RequiredByTarget);
}

void moveGlobalVarInitializationToEntryPointsForResourceGlobalLegalization(
    IRModule* module,
    DiagnosticSink* sink)
{
    // We extract only the resource initializers that `legalizeResourceGlobalVars` will replace. We
    // first reject any selected initializer whose evaluation order could be observed.
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(
        module,
        nullptr,
        GlobalInitializerSelection::FileScopeStaticResources,
        sink);
}

} // namespace Slang
