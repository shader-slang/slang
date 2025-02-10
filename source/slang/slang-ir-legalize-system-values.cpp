#include "slang-ir-legalize-system-values.h"

#include "core/slang-dictionary.h"
#include "core/slang-string.h"
#include "slang-diagnostics.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"

namespace Slang
{

class ImplicitSystemValueLegalizationContext
{
public:
    ImplicitSystemValueLegalizationContext(
        const Dictionary<IRInst*, HashSet<IRFunc*>>& entryPointReferenceGraph,
        const Dictionary<IRInst*, HashSet<IRFunc*>>& functionReferenceGraph,
        const Dictionary<IRFunc*, HashSet<IRCall*>>& callReferenceGraph,
        const List<IRImplicitSystemValue*>& implicitSystemValueInstructions)
        : m_entryPointReferenceGraph(entryPointReferenceGraph)
        , m_functionReferenceGraph(functionReferenceGraph)
        , m_callReferenceGraph(callReferenceGraph)
        , m_implicitSystemValueInstructions(implicitSystemValueInstructions)
    {
    }

    void legalize()
    {
        for (auto implicitSysVal : m_implicitSystemValueInstructions)
        {
            // for (auto entryPoint : *m_entryPointReferenceGraph.tryGetValue(implicitSysVal))
            for (auto entryPoint : *m_functionReferenceGraph.tryGetValue(implicitSysVal))
            {
                auto param = getOrCreateSystemValueParam(entryPoint, implicitSysVal);
                implicitSysVal->replaceUsesWith(param);
                implicitSysVal->removeAndDeallocate();
            }
        }
    }

private:
    using SystemValueParamMap = Dictionary<SystemValueSemanticName, IRParam*>;

    SystemValueParamMap& getParamMap(IRFunc* func)
    {
        if (auto map = m_functionMap.tryGetValue(func))
        {
            return *map;
        }
        else
        {
            m_functionMap.add(func, SystemValueParamMap());
            return m_functionMap.getValue(func);
        }
    }

    //
    // Attempt to retrieve a parameter for a specific function and system value type combination.
    //
    IRParam* tryGetParam(IRFunc* func, SystemValueSemanticName systemValueName)
    {
        if (auto param = getParamMap(func).tryGetValue(systemValueName))
        {
            return *param;
        }
        else
        {
            return nullptr;
        }
    }

    //
    // Implicit system values are "global variables" and can be used anywhere within the source
    // code. The implementation target(i.e WGSL) however requires system values, aka built-in
    // values, to be accessed via parameters to the entry point; they are not globally available.
    //
    // For any implicit system values found in non entry point functions, we need to ensure that
    // they are explicitly passed as parameters from the entry point to the relevant functions. This
    // means adding new parameters to the function signatures to include the required system values.
    //
    struct CreateParamWorkItem
    {
        /// Function to add a new param to.
        IRFunc* func;

        /// System value semantic info.
        SystemValueSemanticName systemValueName;
        UnownedStringSlice systemValueString;

        Index paramIndex;
    };

    //
    // In addition to modifying the function signatures, we need to ensure the calls to the modified
    // function include the system value variable.
    //
    struct ModifyCallWorkItem
    {
        /// Call to modify.
        IRCall* call;

        /// Parameter to add to the call.
        // IRParam* paramToAdd;
        Index paramIndex;
    };

    //
    // Creates a new parameter for functions that require the implicit system value.
    // Returns work items of calls that need to be modified as a result of adding the parameters.
    //
    List<ModifyCallWorkItem> createFunctionParams(
        IRFunc* parentFunc,
        IRImplicitSystemValue* implicitSysVal)
    {
        // Work list for parameter creation work.
        List<CreateParamWorkItem> createParamWorkList;

        // Work list for call replacement work.
        List<ModifyCallWorkItem> modifyCallWorkList;

        // List to store new parameters that represent system value variables - used to propagate
        // the param variables from parameter creation work to call replacement work.
        List<IRParam*> addedParams;

        const auto systemValueName =
            convertSystemValueSemanticNameToEnum(implicitSysVal->getSystemValueName());
        SLANG_ASSERT(systemValueName != SystemValueSemanticName::Unknown);

        createParamWorkList.add(
            {parentFunc,
             systemValueName,
             implicitSysVal->getSystemValueName(),
             addedParams.getCount()});
        addedParams.add(nullptr);

        const auto addWorkItems = [&](const HashSet<IRCall*>& calls, CreateParamWorkItem workItem)
        {
            for (auto call : calls)
            {
                for (auto callerFunc : m_functionReferenceGraph.getValue(call))
                {
                    // The caller(of a function that was added a parameter) also requires a
                    // new parameter to pass in the system value variable to the callee.
                    workItem.func = callerFunc;
                    workItem.paramIndex = addedParams.getCount();
                    createParamWorkList.add(workItem);
                    addedParams.add(nullptr);

                    // The call needs to be modified to add the new parameter.
                    modifyCallWorkList.add({call, workItem.paramIndex});
                }
            }
        };

        IRBuilder builder(parentFunc);

        // The new system value parameter's type is harcoded.
        //
        // Implicit system values are currently only being used for subgroup size and
        // subgroup invocation id, both of which are 32-bit unsigned.
        SLANG_ASSERT(
            (systemValueName == SystemValueSemanticName::WaveLaneCount) ||
            (systemValueName == SystemValueSemanticName::WaveLaneIndex));
        auto paramType = builder.getUIntType();

        const auto createParamWork = [&](CreateParamWorkItem workItem)
        {
            // If the parameter for system value type has not been created, create it.
            if (!tryGetParam(workItem.func, workItem.systemValueName))
            {
                builder.setInsertBefore(workItem.func->getFirstBlock()->getFirstOrdinaryInst());

                auto param = builder.emitParam(paramType);

                // Add system value semantic decoration if adding to entry point.
                if (parentFunc->findDecoration<IREntryPointDecoration>())
                {
                    builder.addSemanticDecoration(param, workItem.systemValueString);
                }

                getParamMap(workItem.func).add(systemValueName, param);
                addedParams[workItem.paramIndex] = param;

                if (auto calls = m_callReferenceGraph.tryGetValue(workItem.func))
                {
                    addWorkItems(*calls, workItem);
                }
            }
        };

        for (Index i = 0; i < createParamWorkList.getCount(); i++)
        {
            createParamWork(createParamWorkList[i]);
        }

        const auto modifyCallWork = [&](IRCall* call, IRParam* param)
        {
            List<IRInst*> newCallParams;
            for (auto arg : call->getArgsList())
            {
                newCallParams.add(arg);
            }
            newCallParams.add(param);

            IRBuilder newBuilder(call);
            newBuilder.setInsertAfter(call);
            auto newCall = newBuilder.emitCallInst(paramType, call->getCallee(), newCallParams);

            // call->replaceUsesWith(newCall);
            // call->transferDecorationsTo(newCall);
            // call->removeAndDeallocate();
        };

        for (const auto workItem : modifyCallWorkList)
        {
            modifyCallWork(workItem.call, addedParams[workItem.paramIndex]);
        }

        return modifyCallWorkList;
    }

    IRParam* getOrCreateSystemValueParam(IRFunc* parentFunc, IRImplicitSystemValue* implicitSysVal)
    {
        const auto systemValueName =
            convertSystemValueSemanticNameToEnum(implicitSysVal->getSystemValueName());
        SLANG_ASSERT(systemValueName != SystemValueSemanticName::Unknown);

        if (auto param = tryGetParam(parentFunc, systemValueName))
        {
            return param;
        }

        createFunctionParams(parentFunc, implicitSysVal);
        return tryGetParam(parentFunc, systemValueName);
    }

    // void replaceCalls() {}

    IRParam* getOrCreateSystemValueParam2(IRFunc* parentFunc, IRImplicitSystemValue* implicitSysVal)
    {
        if (!m_functionMap.containsKey(parentFunc))
        {
            m_functionMap.add(parentFunc, SystemValueParamMap());
        }
        auto systemValueParamMap = m_functionMap.tryGetValue(parentFunc);

        const auto systemValueName =
            convertSystemValueSemanticNameToEnum(implicitSysVal->getSystemValueName());
        SLANG_ASSERT(systemValueName != SystemValueSemanticName::Unknown);

        IRParam* param;
        const auto paramPtr = systemValueParamMap->tryGetValue(systemValueName);
        if (paramPtr == nullptr)
        {
            IRBuilder builder(parentFunc);
            builder.setInsertBefore(parentFunc->getFirstBlock()->getFirstOrdinaryInst());

            // The new system value parameter's type is harcoded.
            //
            // Implicit system values are currently only being used for subgroup size and subgroup
            // invocation id, both of which are 32-bit unsigned.
            SLANG_ASSERT(
                (systemValueName == SystemValueSemanticName::WaveLaneCount) ||
                (systemValueName == SystemValueSemanticName::WaveLaneIndex));
            auto paramType = builder.getUIntType();
            param = builder.emitParam(paramType);


            if (parentFunc->findDecoration<IREntryPointDecoration>())
            {
                builder.addSemanticDecoration(param, implicitSysVal->getSystemValueName());
            }

            systemValueParamMap->add(systemValueName, param);

            if (m_callReferenceGraph.containsKey(parentFunc))
            {
                for (auto call : m_callReferenceGraph.getValue(parentFunc))
                {
                    for (auto callParentFunc : m_functionReferenceGraph.getValue(call))
                    {
                        SLANG_ASSERT(parentFunc != callParentFunc);
                        auto newCallParam =
                            getOrCreateSystemValueParam(callParentFunc, implicitSysVal);

                        List<IRInst*> newCallParams;
                        for (auto arg : call->getArgsList())
                        {
                            newCallParams.add(arg);
                        }
                        newCallParams.add(newCallParam);

                        builder.setInsertAfter(call);
                        auto newCall = builder.emitCallInst(paramType, parentFunc, newCallParams);

                        call->replaceUsesWith(newCall);
                        call->transferDecorationsTo(newCall);
                        call->removeAndDeallocate();
                    }
                }
            }
        }
        else
        {
            param = *paramPtr;
        }

        return param;
    }


    const Dictionary<IRInst*, HashSet<IRFunc*>>& m_entryPointReferenceGraph;
    const Dictionary<IRInst*, HashSet<IRFunc*>>& m_functionReferenceGraph;
    const Dictionary<IRFunc*, HashSet<IRCall*>>& m_callReferenceGraph;
    const List<IRImplicitSystemValue*>& m_implicitSystemValueInstructions;

    Dictionary<IRFunc*, SystemValueParamMap> m_functionMap;
};

void legalizeImplicitSystemValues(
    const Dictionary<IRInst*, HashSet<IRFunc*>>& entryPointReferenceGraph,
    const Dictionary<IRInst*, HashSet<IRFunc*>>& functionReferenceGraph,
    const Dictionary<IRFunc*, HashSet<IRCall*>>& callReferenceGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions)
{
    ImplicitSystemValueLegalizationContext(
        entryPointReferenceGraph,
        functionReferenceGraph,
        callReferenceGraph,
        implicitSystemValueInstructions)
        .legalize();
}

} // namespace Slang
