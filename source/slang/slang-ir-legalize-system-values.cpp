#include "slang-ir-legalize-system-values.h"

#include "core/slang-dictionary.h"
#include "core/slang-string.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"

namespace Slang
{

class ImplicitSystemValueLegalizationContext
{
public:
    ImplicitSystemValueLegalizationContext(
        IRModule* module,
        const Dictionary<IRInst*, HashSet<IRFunc*>>& functionReferenceGraph,
        const Dictionary<IRFunc*, HashSet<IRCall*>>& callReferenceGraph,
        const List<IRImplicitSystemValue*>& implicitSystemValueInstructions)
        : m_functionReferenceGraph(functionReferenceGraph)
        , m_callReferenceGraph(callReferenceGraph)
        , m_implicitSystemValueInstructions(implicitSystemValueInstructions)
        , m_builder(module)
        , m_paramType(m_builder.getUIntType())
    {
    }

    void legalize()
    {
        for (auto implicitSysVal : m_implicitSystemValueInstructions)
        {
            // for (auto entryPoint : *m_entryPointReferenceGraph.tryGetValue(implicitSysVal))
            for (auto entryPoint : *m_functionReferenceGraph.tryGetValue(implicitSysVal))
            {
                auto param = getOrCreateSystemValueVariable(entryPoint, implicitSysVal);
                implicitSysVal->replaceUsesWith(param);
                implicitSysVal->removeAndDeallocate();
            }
        }
    }

private:
    //
    // A function (including entry points) must have at most one parameter for each system value
    // semantic type.
    //
    // This map tracks the association between system value semantics and their corresponding
    // function parameters.
    //
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
    // Returns nullptr if parameter has not been created.
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

    struct ModifyCallWorkItem
    {
        IRCall* call;
        IRFunc* caller;
    };

    //
    // Implicit system values are "global variables" and can be used anywhere within the source
    // code. The implementation target(i.e WGSL) however requires system values, aka built-in
    // values, to be accessed via parameters to the entry point; they are not globally available.
    //
    // For any implicit system values found in non entry point functions, we need to ensure that
    // they are explicitly passed as parameters from the entry point to the relevant functions. This
    // means adding new parameters to the function signatures to include the required system values.
    //
    // This function traverses the call graph of a function that contains an implicit system value
    // instruction, and adds necessary parameters to pass in the system value variable up to the
    // entry point function. Returns work items of calls that need to be modified as a result of
    // adding the parameters.
    //
    List<ModifyCallWorkItem> createFunctionParams(
        IRFunc* func,
        SystemValueSemanticName systemValueName,
        UnownedStringSlice systemValueString)
    {
        List<IRFunc*> functionWorkList;
        List<ModifyCallWorkItem> modifyCallWorkList;

        const auto addWorkItems = [&](const HashSet<IRCall*>& calls)
        {
            for (auto call : calls)
            {
                for (auto caller : m_functionReferenceGraph.getValue(call))
                {
                    // The caller(of a function that was added a parameter) also requires a
                    // new parameter to pass in the system value variable to the callee.
                    functionWorkList.add(caller);

                    // The call needs to be modified to add the new parameter.
                    modifyCallWorkList.add({call, caller});
                }
            }
        };

        // Implicit system values are currently only being used for subgroup size and
        // subgroup invocation id.
        SLANG_ASSERT(
            (systemValueName == SystemValueSemanticName::WaveLaneCount) ||
            (systemValueName == SystemValueSemanticName::WaveLaneIndex));

        const auto createParamWork = [&](IRFunc* func)
        {
            // If the parameter for system value type has not been created, create it.
            if (!tryGetParam(func, systemValueName))
            {
                m_builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());

                auto param = m_builder.emitParam(m_paramType);

                // Add system value semantic decoration if adding to entry point.
                if (func->findDecoration<IREntryPointDecoration>())
                {
                    m_builder.addSemanticDecoration(param, systemValueString);
                }

                getParamMap(func).add(systemValueName, param);

                if (auto calls = m_callReferenceGraph.tryGetValue(func))
                {
                    addWorkItems(*calls);
                }
            }
        };

        functionWorkList.add(func);
        for (Index i = 0; i < functionWorkList.getCount(); i++)
        {
            createParamWork(functionWorkList[i]);
        }

        return modifyCallWorkList;
    }

    //
    // In addition to modifying the function signatures, we need to ensure the calls to the modified
    // function include the system value variable.
    //
    void modifyCalls(
        const List<ModifyCallWorkItem>& workItems,
        SystemValueSemanticName systemValueName)
    {
        for (const auto workItem : workItems)
        {
            auto call = workItem.call;
            auto param = tryGetParam(workItem.caller, systemValueName);
            SLANG_ASSERT(param);

            List<IRInst*> newCallParams;
            for (auto arg : call->getArgsList())
            {
                newCallParams.add(arg);
            }
            newCallParams.add(param);

            m_builder.setInsertAfter(call);
            auto newCall = m_builder.emitCallInst(m_paramType, call->getCallee(), newCallParams);

            call->replaceUsesWith(newCall);
            call->transferDecorationsTo(newCall);
            call->removeAndDeallocate();
        }
    }

    IRParam* getOrCreateSystemValueVariable(
        IRFunc* parentFunc,
        IRImplicitSystemValue* implicitSysVal)
    {
        auto systemValueName =
            convertSystemValueSemanticNameToEnum(implicitSysVal->getSystemValueName());
        SLANG_ASSERT(systemValueName != SystemValueSemanticName::Unknown);

        // If parameter for the specific function and system value type combination was already
        // created, return it directly.
        if (auto param = tryGetParam(parentFunc, systemValueName))
        {
            return param;
        }

        // Create new parameters for the relevant functions up to the entry point function.
        const auto callWorkItems =
            createFunctionParams(parentFunc, systemValueName, implicitSysVal->getSystemValueName());

        // Modify related function calls to account for the new parameters.
        modifyCalls(callWorkItems, systemValueName);

        return tryGetParam(parentFunc, systemValueName);
    }

    const Dictionary<IRInst*, HashSet<IRFunc*>>& m_functionReferenceGraph;
    const Dictionary<IRFunc*, HashSet<IRCall*>>& m_callReferenceGraph;
    const List<IRImplicitSystemValue*>& m_implicitSystemValueInstructions;

    Dictionary<IRFunc*, SystemValueParamMap> m_functionMap;

    IRBuilder m_builder;

    // Implicit system values are currently only being used for subgroup size and
    // subgroup invocation id, both of which are 32-bit unsigned.
    IRType* m_paramType;
};

void legalizeImplicitSystemValues(
    IRModule* module,
    const Dictionary<IRInst*, HashSet<IRFunc*>>& functionReferenceGraph,
    const Dictionary<IRFunc*, HashSet<IRCall*>>& callReferenceGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions)
{
    ImplicitSystemValueLegalizationContext(
        module,
        functionReferenceGraph,
        callReferenceGraph,
        implicitSystemValueInstructions)
        .legalize();
}

} // namespace Slang
