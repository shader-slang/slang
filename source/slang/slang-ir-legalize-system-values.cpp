#include "slang-ir-legalize-system-values.h"

#include "core/slang-dictionary.h"
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
        const List<IRImplicitSystemValue*>& implicitSystemValueInstructions)
        : m_entryPointReferenceGraph(entryPointReferenceGraph)
        , m_implicitSystemValueInstructions(implicitSystemValueInstructions)
    {
    }

    void legalize()
    {
        for (auto implicitSysVal : m_implicitSystemValueInstructions)
        {
            for (auto entryPoint : *m_entryPointReferenceGraph.tryGetValue(implicitSysVal))
            {
                auto param = getOrCreateSystemValueParam(entryPoint, implicitSysVal);
                implicitSysVal->replaceUsesWith(param);
                implicitSysVal->removeAndDeallocate();
            }
        }
    }

private:
    IRParam* getOrCreateSystemValueParam(IRFunc* entryPoint, IRImplicitSystemValue* implicitSysVal)
    {
        if (!m_entryPointMap.containsKey(entryPoint))
        {
            m_entryPointMap.add(entryPoint, SystemValueParamMap());
        }
        auto systemValueParamMap = m_entryPointMap.tryGetValue(entryPoint);

        const auto systemValueName =
            convertSystemValueSemanticNameToEnum(implicitSysVal->getSystemValueName());
        SLANG_ASSERT(systemValueName != SystemValueSemanticName::Unknown);

        IRParam* param;
        const auto paramPtr = systemValueParamMap->tryGetValue(systemValueName);
        if (paramPtr == nullptr)
        {
            IRBuilder builder(entryPoint);
            builder.setInsertBefore(entryPoint->getFirstBlock()->getFirstOrdinaryInst());

            // The new system value parameter's type is harcoded.
            //
            // Implicit system values are currently only being used for subgroup size and subgroup
            // invocation id, both of which are 32-bit unsigned.
            SLANG_ASSERT(
                (systemValueName == SystemValueSemanticName::WaveLaneCount) ||
                (systemValueName == SystemValueSemanticName::WaveLaneIndex));
            param = builder.emitParam(builder.getUIntType());
            builder.addSemanticDecoration(param, implicitSysVal->getSystemValueName());

            systemValueParamMap->add(systemValueName, param);
        }
        else
        {
            param = *paramPtr;
        }

        return param;
    }

    using SystemValueParamMap = Dictionary<SystemValueSemanticName, IRParam*>;

    const Dictionary<IRInst*, HashSet<IRFunc*>>& m_entryPointReferenceGraph;
    const List<IRImplicitSystemValue*>& m_implicitSystemValueInstructions;
    Dictionary<IRFunc*, SystemValueParamMap> m_entryPointMap;
};

void legalizeImplicitSystemValues(
    const Dictionary<IRInst*, HashSet<IRFunc*>>& entryPointReferenceGraph,
    const List<IRImplicitSystemValue*>& implicitSystemValueInstructions)
{
    ImplicitSystemValueLegalizationContext(
        entryPointReferenceGraph,
        implicitSystemValueInstructions)
        .legalize();
}

} // namespace Slang
