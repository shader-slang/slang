#include "slang-ir-wgsl-legalize.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-parameter-binding.h"
#include "slang-ir-legalize-varying-params.h"

namespace Slang
{

    struct EntryPointInfo
    {
        IRFunc* entryPointFunc;
        IREntryPointDecoration* entryPointDecor;
    };

    struct SystemValLegalizationWorkItem
    {
        IRInst* var;
        String attrName;
        UInt attrIndex;
    };

    struct WGSLSystemValueInfo
    {
        String wgslSystemValueName;
        SystemValueSemanticName wgslSystemValueNameEnum;
        ShortList<IRType*> permittedTypes;
        bool isUnsupported = false;
    };

    struct LegalizeWGSLEntryPointContext
    {
        LegalizeWGSLEntryPointContext(DiagnosticSink* sink, IRModule* module) :
            m_sink(sink), m_module(module) {}

        DiagnosticSink* m_sink;
        IRModule* m_module;

        std::optional<SystemValLegalizationWorkItem> makeSystemValWorkItem(IRInst* var);
        void legalizeSystemValue(
            EntryPointInfo entryPoint, SystemValLegalizationWorkItem& workItem
        );
        List<SystemValLegalizationWorkItem> collectSystemValFromEntryPoint(
            EntryPointInfo entryPoint
        );
        void legalizeSystemValueParameters(EntryPointInfo entryPoint);
        void legalizeEntryPointForWGSL(EntryPointInfo entryPoint);
        IRInst* tryConvertValue(IRBuilder& builder, IRInst* val, IRType* toType);
        WGSLSystemValueInfo getSystemValueInfo(
            String inSemanticName, String* optionalSemanticIndex, IRInst* parentVar
        );
    };

    IRInst* LegalizeWGSLEntryPointContext::tryConvertValue(
        IRBuilder& builder, IRInst* val, IRType* toType
    )
    {
        auto fromType = val->getFullType();
        if (auto fromVector = as<IRVectorType>(fromType))
        {
            if (auto toVector = as<IRVectorType>(toType))
            {
                if (fromVector->getElementCount() != toVector->getElementCount())
                {
                    fromType =
                        builder.getVectorType(
                            fromVector->getElementType(), toVector->getElementCount()
                        );
                    val = builder.emitVectorReshape(fromType, val);
                }
            }
            else if (as<IRBasicType>(toType))
            {
                UInt index = 0;
                val = builder.emitSwizzle(fromVector->getElementType(), val, 1, &index);
                if (toType->getOp() == kIROp_VoidType)
                    return nullptr;
            }
        }
        else if (auto fromBasicType = as<IRBasicType>(fromType))
        {
            if (fromBasicType->getOp() == kIROp_VoidType)
                return nullptr;
            if (!as<IRBasicType>(toType))
                return nullptr;
            if (toType->getOp() == kIROp_VoidType)
                return nullptr;
        }
        else
        {
            return nullptr;
        }
        return builder.emitCast(toType, val);
    }


    WGSLSystemValueInfo LegalizeWGSLEntryPointContext::getSystemValueInfo(
        String inSemanticName, String* optionalSemanticIndex, IRInst* parentVar
    )
    {
        IRBuilder builder(m_module);
        WGSLSystemValueInfo result = {};
        UnownedStringSlice semanticName;
        UnownedStringSlice semanticIndex;

        auto hasExplicitIndex =
            splitNameAndIndex(
                inSemanticName.getUnownedSlice(), semanticName, semanticIndex
            );
        if (!hasExplicitIndex && optionalSemanticIndex)
            semanticIndex = optionalSemanticIndex->getUnownedSlice();

        result.wgslSystemValueNameEnum =
            convertSystemValueSemanticNameToEnum(semanticName);

        switch (result.wgslSystemValueNameEnum)
        {

            case SystemValueSemanticName::DispatchThreadID:
            {
                result.wgslSystemValueName = toSlice("global_invocation_id");
                IRType *const vec3uType {
                    builder.getVectorType(
                        builder.getBasicType(BaseType::UInt),
                        builder.getIntValue(builder.getIntType(), 3)
                    )
                };
                result.permittedTypes.add(vec3uType);
            }
            break;

            case SystemValueSemanticName::GroupID:
            {
                result.wgslSystemValueName = toSlice("workgroup_id");
                result.permittedTypes.add(
                    builder.getVectorType(
                        builder.getBasicType(BaseType::UInt),
                        builder.getIntValue(builder.getIntType(), 3)
                    )
                );
            }
            break;

            case SystemValueSemanticName::GroupThreadID:
            {
                result.wgslSystemValueName = toSlice("local_invocation_id");
                result.permittedTypes.add(
                    builder.getVectorType(
                        builder.getBasicType(BaseType::UInt),
                        builder.getIntValue(builder.getIntType(), 3)
                    )
                );
            }
            break;

            case SystemValueSemanticName::GSInstanceID:
            {
                // No Geometry shaders in WGSL
                result.isUnsupported = true;
            }
            break;

            default:
            {
                m_sink->diagnose(
                    parentVar,
                    Diagnostics::unimplementedSystemValueSemantic, semanticName
                );
                return result;
            }

        }

        return result;
    }

    std::optional<SystemValLegalizationWorkItem>
    LegalizeWGSLEntryPointContext::makeSystemValWorkItem(IRInst* var)
    {
        if (auto semanticDecoration = var->findDecoration<IRSemanticDecoration>())
        {
            bool svPrefix =
                semanticDecoration->getSemanticName().startsWithCaseInsensitive(
                    toSlice("sv_")
                );
            if (svPrefix)
            {
                return
                {
                    {
                        var,
                        String(semanticDecoration->getSemanticName()).toLower(),
                        (UInt)semanticDecoration->getSemanticIndex()
                    }
                };
            }
        }

        auto layoutDecor = var->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            return {};
        auto sysValAttr = layoutDecor->findAttr<IRSystemValueSemanticAttr>();
        if (!sysValAttr)
            return {};
        auto semanticName = String(sysValAttr->getName());
        auto sysAttrIndex = sysValAttr->getIndex();

        return { { var, semanticName, sysAttrIndex } };
    }

    List<SystemValLegalizationWorkItem>
    LegalizeWGSLEntryPointContext::collectSystemValFromEntryPoint(
        EntryPointInfo entryPoint
    )
    {
        List<SystemValLegalizationWorkItem> systemValWorkItems;
        for (auto param : entryPoint.entryPointFunc->getParams())
        {
            auto maybeWorkItem = makeSystemValWorkItem(param);
            if (maybeWorkItem.has_value())
                systemValWorkItems.add(std::move(maybeWorkItem.value()));
        }
        return systemValWorkItems;
    }

    void
    LegalizeWGSLEntryPointContext::legalizeSystemValue(
        EntryPointInfo entryPoint, SystemValLegalizationWorkItem& workItem
    )
    {
        IRBuilder builder(entryPoint.entryPointFunc);

        auto var = workItem.var;
        auto semanticName = workItem.attrName;

        auto indexAsString = String(workItem.attrIndex);
        auto info = getSystemValueInfo(semanticName, &indexAsString, var);

        if (!info.permittedTypes.getCount())
            return;

        builder.addTargetSystemValueDecoration(
            var, info.wgslSystemValueName.getUnownedSlice()
        );

        bool varTypeIsPermitted = false;
        auto varType = var->getFullType();
        for (auto& permittedType : info.permittedTypes)
        {
            varTypeIsPermitted = varTypeIsPermitted || permittedType == varType;
        }

        if (!varTypeIsPermitted)
        {
            // Note: we do not currently prefer any conversion
            // example:
            // * allowed types for semantic: `float4`, `uint4`, `int4`
            // * user used, `float2`
            // * Slang will equally prefer `float4` to `uint4` to `int4`.
            //   This means the type may lose data if slang selects `uint4` or `int4`.
            bool foundAConversion = false;
            for (auto permittedType : info.permittedTypes)
            {
                var->setFullType(permittedType);
                builder.setInsertBefore(
                    entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst()
                );

                // get uses before we `tryConvertValue` since this creates a new use
                List<IRUse*> uses;
                for (auto use = var->firstUse; use; use = use->nextUse)
                    uses.add(use);

                auto convertedValue = tryConvertValue(builder, var, varType);
                if (convertedValue == nullptr)
                    continue;

                foundAConversion = true;
                copyNameHintAndDebugDecorations(convertedValue, var);

                for (auto use : uses)
                    builder.replaceOperand(use, convertedValue);
            }
            if (!foundAConversion)
            {
                // If we can't convert the value, report an error.
                for (auto permittedType : info.permittedTypes)
                {
                    StringBuilder typeNameSB;
                    getTypeNameHint(typeNameSB, permittedType);
                    m_sink->diagnose(
                        var->sourceLoc,
                        Diagnostics::systemValueTypeIncompatible,
                        semanticName,
                        typeNameSB.produceString()
                    );
                }
            }
        }
    }

    void LegalizeWGSLEntryPointContext::legalizeSystemValueParameters(
        EntryPointInfo entryPoint
    )
    {
        List<SystemValLegalizationWorkItem> systemValWorkItems =
            collectSystemValFromEntryPoint(entryPoint);

        for (auto index = 0; index < systemValWorkItems.getCount(); index++)
        {
            legalizeSystemValue(entryPoint, systemValWorkItems[index]);
        }
    }

    void LegalizeWGSLEntryPointContext::legalizeEntryPointForWGSL(
        EntryPointInfo entryPoint
    )
    {
        legalizeSystemValueParameters(entryPoint);
    }

    void legalizeIRForWGSL(IRModule* module, DiagnosticSink* sink)
    {
        List<EntryPointInfo> entryPoints;
        for (auto inst : module->getGlobalInsts())
        {
            IRFunc *const func {as<IRFunc>(inst)};
            if (!func)
                continue;
            IREntryPointDecoration *const entryPointDecor =
                func->findDecoration<IREntryPointDecoration>();
            if (!entryPointDecor)
                continue;
            EntryPointInfo info;
            info.entryPointDecor = entryPointDecor;
            info.entryPointFunc = func;
            entryPoints.add(info);
        }

        LegalizeWGSLEntryPointContext context(sink, module);
        for (auto entryPoint : entryPoints)
            context.legalizeEntryPointForWGSL(entryPoint);
    }

}
