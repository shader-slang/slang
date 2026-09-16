// slang-ir-explicit-global-init.cpp
#include "slang-ir-explicit-global-init.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{

// This pass is responsible for taking code in a form like:
//
//      static int gCounter = 1;
//
//      void computeMain()
//      {
//          ...
//          int tmp = gCounter++;
//      }
//
// and transforming it so that the initialization of global
// variables is performed explicitly at the start of each
// entry-point function:
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
// Transforming the code in this way may be required for targets
// that do not support initial-value expressions on global
// variables (e.g., SPIR-V is such a target). It can also be
// useful as a pre-process before other transformations that
// might work with global variables, because the selected global
// variables will no longer have initializers afterward.

enum class GlobalInitSelection
{
    DefaultForTarget,
    ResourceDependentGlobals,
};

static bool canCallArgumentWrite(IRCall* call, IRUse* use)
{
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
    auto user = use->getUser();
    if (as<IRLoad>(user) || as<IRAtomicLoad>(user) || isTypeOnlyInst(user))
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

struct MoveGlobalVarInitializationToEntryPointsPass
{
    struct InitDependencyInfo
    {
        IRGlobalValueWithCode* code = nullptr;
        List<Index> dependencies;
        List<Index> callees;
        List<IRGlobalVar*> mutatedGlobals;
        bool requiresEntryPointInitialization = false;
    };

    IRModule* m_module;
    TargetProgram* m_targetProgram;
    GlobalInitSelection m_selection;
    HashSet<IRGlobalVar*> m_resourceStateGlobals;
    HashSet<IRGlobalVar*> m_resourceDependentInitializerGlobals;
    HashSet<IRGlobalVar*> m_earlyInitializedGlobals;
    HashSet<IRGlobalVar*> m_resourceDependentStateGlobals;
    List<IRGlobalVar*>* m_outResourceDependentState;
    List<InitDependencyInfo> m_initDependencyInfos;
    Dictionary<IRGlobalValueWithCode*, Index> m_initDependencyIndices;

    // In the Slang IR, a global variable represents a pointer
    // to the storage for the variable but it *also* encodes
    // the logic used to compute the initial value of that
    // variable. This works because `IRGlobalVar` is a subtype
    // of `IRGlobalValueWithCode`, which is also the base
    // type of `IRFunc`. Thus a global variable behaves a
    // bit like a function, which just happens to compute
    // the initial value for the variable.
    //
    // Part of the work in this pass will be to split those
    // two parts of the variable, so that we end up with
    // a global variable with no initialization logic,
    // plus an ordinary `IRFunc` to compute the initial
    // value.
    //
    // We will compute this split representation and then
    // hold onto it so that we can use it for injecting
    // the initialization logic into entry points.
    //
    struct GlobalVarInfo
    {
        IRGlobalVar* globalVar = nullptr;
        IRFunc* initFunc = nullptr;
    };
    List<GlobalVarInfo> m_globalVarsWithInit;

    void processModule(
        IRModule* module,
        TargetProgram* targetProgram,
        GlobalInitSelection selection,
        List<IRGlobalVar*>* outResourceDependentState = nullptr)
    {
        m_module = module;
        m_targetProgram = targetProgram;
        m_selection = selection;
        m_outResourceDependentState = outResourceDependentState;

        if (m_selection == GlobalInitSelection::ResourceDependentGlobals)
        {
            collectInitializerDependencyGraph();
            findResourceDependentGlobals();
            collectMovedInitializerState();
            collectResourceDependentState();
        }

        // We start by looking for global variables with
        // initialization logic in the IR, and processing
        // each to produce a split variable (now without
        // initialization) and function (to compute the
        // initial value).
        //
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;

            // If it's an `Actual Global` we don't want to move initialization
            if (as<IRActualGlobalRate>(globalVar->getRate()))
            {
                continue;
            }

            auto firstBlock = globalVar->getFirstBlock();
            if (!firstBlock)
                continue;

            processGlobalVarWithInit(globalVar, firstBlock);
        }

        // Then we loop over all the entry points in the
        // module and modify them to explicitly initialize
        // all the global variables that were identified
        // and processed in the first pass.
        //
        for (auto inst : m_module->getGlobalInsts())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            if (!func->findDecoration<IREntryPointDecoration>())
                continue;

            processEntryPoint(func);
        }
    }

    bool shouldMoveGlobalVarInitialization(IRGlobalVar* globalVar)
    {
        if (m_selection == GlobalInitSelection::ResourceDependentGlobals)
            return m_earlyInitializedGlobals.contains(globalVar);

        // Currently CoopVector for DXC cannot be created from
        // constructors with arguments. When CoopVector is used as a
        // global variable, its initialization has to happen at the
        // beginning of the entry point.
        //
        // At the same time, we don't want to apply
        // "moveGlobalVarInitializationToEntryPoints" to the rest of
        // the global variables when targeting HLSL.
        //
        if (isD3DTarget(m_targetProgram->getTargetReq()))
        {
            auto valueType = globalVar->getDataType()->getValueType();
            if (as<IRCoopVectorType>(valueType))
                return true;
            return false;
        }
        return true;
    }

    void collectInitializerDependencyGraph()
    {
        // Build one graph for both questions this early move must answer: which initializers depend
        // on resource state, and which globals the selected initializer call graph may mutate.
        // Keeping calls, value dependencies, and mutation effects in one record prevents those two
        // closures from drifting apart as new IR address or call forms are introduced.
        for (auto inst : m_module->getGlobalInsts())
        {
            auto globalVar = as<IRGlobalVar>(inst);
            if (!globalVar)
                continue;
            if (isPerInvocationResourceStateGlobalVar(globalVar))
                m_resourceStateGlobals.add(globalVar);
        }

        for (auto inst : m_module->getGlobalInsts())
        {
            auto code = as<IRGlobalValueWithCode>(inst);
            if (!code || !code->getFirstBlock())
                continue;

            Index index = m_initDependencyInfos.getCount();
            m_initDependencyInfos.add(InitDependencyInfo{code});
            m_initDependencyIndices.add(code, index);

            if (auto globalVar = as<IRGlobalVar>(code))
            {
                if (m_resourceStateGlobals.contains(globalVar))
                {
                    m_initDependencyInfos[index].requiresEntryPointInitialization = true;
                }
            }
        }

        for (auto& info : m_initDependencyInfos)
        {
            for (auto block : info.code->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    if (auto call = as<IRCall>(inst))
                    {
                        if (auto callee = as<IRGlobalValueWithCode>(call->getCallee()))
                        {
                            if (auto dependencyIndex = m_initDependencyIndices.tryGetValue(callee))
                            {
                                info.dependencies.add(*dependencyIndex);
                                info.callees.add(*dependencyIndex);
                            }
                        }
                    }

                    for (UInt operandIndex = 0; operandIndex < inst->getOperandCount();
                         ++operandIndex)
                    {
                        auto globalVar = as<IRGlobalVar>(inst->getOperand(operandIndex));
                        if (!globalVar)
                            continue;

                        if (m_resourceStateGlobals.contains(globalVar))
                            info.requiresEntryPointInitialization = true;
                        if (auto dependencyIndex = m_initDependencyIndices.tryGetValue(globalVar))
                            info.dependencies.add(*dependencyIndex);

                        HashSet<IRInst*> visitedAddresses;
                        if (canUseWriteThroughAddress(
                                inst->getOperandUse(operandIndex),
                                visitedAddresses))
                        {
                            info.mutatedGlobals.add(globalVar);
                        }
                    }
                }
            }
        }
    }

    void findResourceDependentGlobals()
    {
        // A non-resource initializer can still evaluate a resource global. Such code must move to
        // an entry point along with the resource initializer; otherwise resource-global
        // legalization would have no entry-point-local address with which to replace that use.
        // Follow both calls and initialized-global references so the selection includes the full
        // dependency closure rather than just direct users.

        bool changed = false;
        do
        {
            changed = false;
            for (auto& info : m_initDependencyInfos)
            {
                if (info.requiresEntryPointInitialization)
                    continue;

                for (auto dependencyIndex : info.dependencies)
                {
                    if (!m_initDependencyInfos[dependencyIndex].requiresEntryPointInitialization)
                        continue;

                    info.requiresEntryPointInitialization = true;
                    changed = true;
                    break;
                }
            }
        } while (changed);

        for (auto const& info : m_initDependencyInfos)
        {
            if (!info.requiresEntryPointInitialization)
                continue;
            if (auto globalVar = as<IRGlobalVar>(info.code))
            {
                m_resourceDependentInitializerGlobals.add(globalVar);
                if (isPerInvocationGlobalVar(globalVar))
                    m_earlyInitializedGlobals.add(globalVar);
            }
        }
    }

    void collectMovedInitializerState()
    {
        // Initialization code can mutate ordinary state through a helper even when that state is
        // not a value dependency of the resource initializer. Record every file-scope global the
        // moved initializer call graph may mutate, so an independent call root cannot observe a
        // value whose initialization now happens only inside an entry point.
        List<Index> workList;
        HashSet<Index> reachableCode;
        for (auto globalVar : m_earlyInitializedGlobals)
        {
            if (auto codeIndex = m_initDependencyIndices.tryGetValue(globalVar))
            {
                if (reachableCode.add(*codeIndex))
                    workList.add(*codeIndex);
            }
        }
        while (workList.getCount())
        {
            auto codeIndex = workList.getLast();
            workList.removeLast();
            auto const& info = m_initDependencyInfos[codeIndex];
            for (auto globalVar : info.mutatedGlobals)
                m_resourceDependentStateGlobals.add(globalVar);
            for (auto calleeIndex : info.callees)
            {
                if (reachableCode.add(calleeIndex))
                    workList.add(calleeIndex);
            }
        }
    }

    void collectResourceDependentState()
    {
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
                m_resourceDependentStateGlobals.contains(globalVar))
            {
                m_outResourceDependentState->add(globalVar);
            }
        }
    }

    void processGlobalVarWithInit(IRGlobalVar* globalVar, IRBlock* firstBlock)
    {
        if (!shouldMoveGlobalVarInitialization(globalVar))
            return;

        IRBuilder builder(m_module);
        builder.setInsertBefore(globalVar);

        // Because an `IRGlobalVar` represents a pointer to the storage
        // for the variable, we need to extract the underlying value
        // type from the pointer type.
        //
        auto valueType = globalVar->getDataType()->getValueType();

        // We are going to construct an explicit IR function to compute
        // the initial value of the variable. That function will always
        // take zero parameters.
        //
        auto initFunc = builder.createFunc();
        initFunc->setFullType(builder.getFuncType(0, nullptr, valueType));

        // The basic blocks under the `IRGlobalVar` define its initialization
        // logic, and we can simply move those blocks over to the new
        // `IRFunc` to define its behavior.
        //
        // As a result, the `globalVar` will no longer have its own
        // initialization logic, which is a postcondition this pass
        // needed to guarantee.
        //
        IRBlock* nextBlock = nullptr;
        for (IRBlock* block = firstBlock; block; block = nextBlock)
        {
            nextBlock = block->getNextBlock();

            block->removeFromParent();
            block->insertAtEnd(initFunc);
        }

        // We need to remember the variable and the associated
        // initial-value function so that we can iterate over
        // them in the per-entry-point logic below.
        //
        GlobalVarInfo info;
        info.globalVar = globalVar;
        info.initFunc = initFunc;
        m_globalVarsWithInit.add(info);
    }

    void processEntryPoint(IRFunc* entryPointFunc)
    {
        // We can only process entry point definitions, not declarations.
        //
        auto firstBlock = entryPointFunc->getFirstBlock();
        if (!firstBlock)
            return;

        // We are going to insert initialization logic at the start
        // of the first block of the entry point.
        //
        IRBuilder builder(m_module);
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());

        for (auto globalVarInfo : m_globalVarsWithInit)
        {
            // The earlier step split each global variable into
            // a variable with no initialization logic, plus a function
            // that can be called to compute the initial value.
            //
            auto globalVar = globalVarInfo.globalVar;
            auto initFunc = globalVarInfo.initFunc;

            // Because the `IRGlobalVar` represents a pointer to
            // storage, we need to get the pointed-to type to
            // get the type of the initial value.
            //
            auto valType = globalVar->getDataType()->getValueType();

            // To simplify the resulting code a bit, if we see the
            // initFunc just returns a constant value, then we just
            // use that inline.
            IRInst* initVal = nullptr;
            if (auto initFirstBlock = initFunc->getFirstBlock())
            {
                if (auto returnInst = as<IRReturn>(initFirstBlock->getTerminator()))
                {
                    if (returnInst->getVal() &&
                        returnInst->getVal()->getParent() == m_module->getModuleInst())
                    {
                        initVal = returnInst->getVal();
                    }
                }
            }
            if (!initVal)
            {
                // We compute the initial value for the variable by calling
                // the initial-value function with no arguments, and then
                // we store that value into the corresponding global.
                //
                initVal = builder.emitCallInst(valType, initFunc, 0, nullptr);
            }
            builder.emitStore(globalVar, initVal);
        }
    }
};

/// Move initialization logic off of global variables and onto each entry point
void moveGlobalVarInitializationToEntryPoints(IRModule* module, TargetProgram* targetProgram)
{
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(module, targetProgram, GlobalInitSelection::DefaultForTarget);
}

void moveResourceDependentGlobalVarInitializationToEntryPoints(
    IRModule* module,
    TargetProgram* targetProgram,
    List<IRGlobalVar*>& outResourceDependentState)
{
    MoveGlobalVarInitializationToEntryPointsPass pass;
    pass.processModule(
        module,
        targetProgram,
        GlobalInitSelection::ResourceDependentGlobals,
        &outResourceDependentState);
}

} // namespace Slang
