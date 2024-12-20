#include "slang-ir-metal-legalize.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-parameter-binding.h"

#include <set>

namespace Slang
{

struct EntryPointInfo
{
    IRFunc* entryPointFunc;
    IREntryPointDecoration* entryPointDecor;
};

const UnownedStringSlice groupThreadIDString = UnownedStringSlice("sv_groupthreadid");
struct LegalizeMetalEntryPointContext
{
    ShortList<IRType*> permittedTypes_sv_target;
    Dictionary<IRFunc*, IRInst*> entryPointToGroupThreadId;
    HashSet<IRStructField*> semanticInfoToRemove;

    DiagnosticSink* m_sink;
    IRModule* m_module;

    LegalizeMetalEntryPointContext(DiagnosticSink* sink, IRModule* module)
        : m_sink(sink), m_module(module)
    {
    }

    void removeSemanticLayoutsFromLegalizedStructs()
    {
        // Metal does not allow duplicate attributes to appear in the same shader.
        // If we emit our own struct with `[[color(0)]`, all existing uses of `[[color(0)]]`
        // must be removed.
        for (auto field : semanticInfoToRemove)
        {
            auto key = field->getKey();
            // Some decorations appear twice, destroy all found
            for (;;)
            {
                if (auto semanticDecor = key->findDecoration<IRSemanticDecoration>())
                {
                    semanticDecor->removeAndDeallocate();
                    continue;
                }
                else if (auto layoutDecor = key->findDecoration<IRLayoutDecoration>())
                {
                    layoutDecor->removeAndDeallocate();
                    continue;
                }
                break;
            }
        }
    }

    void hoistEntryPointParameterFromStruct(EntryPointInfo entryPoint)
    {
        // If an entry point has a input parameter with a struct type, we want to hoist out
        // all the fields of the struct type to be individual parameters of the entry point.
        // This will canonicalize the entry point signature, so we can handle all cases uniformly.

        // For example, given an entry point:
        // ```
        // struct VertexInput { float3 pos; float 2 uv; int vertexId : SV_VertexID};
        // void main(VertexInput vin) { ... }
        // ```
        // We will transform it to:
        // ```
        // void main(float3 pos, float2 uv, int vertexId : SV_VertexID) {
        //     VertexInput vin = {pos,uv,vertexId};
        //     ...
        // }
        // ```

        auto func = entryPoint.entryPointFunc;
        List<IRParam*> paramsToProcess;
        for (auto param : func->getParams())
        {
            if (as<IRStructType>(param->getDataType()))
            {
                paramsToProcess.add(param);
            }
        }

        IRBuilder builder(func);
        builder.setInsertBefore(func);
        for (auto param : paramsToProcess)
        {
            auto structType = as<IRStructType>(param->getDataType());
            builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
            auto varLayout = findVarLayout(param);

            // If `param` already has a semantic, we don't want to hoist its fields out.
            if (varLayout->findSystemValueSemanticAttr() != nullptr ||
                param->findDecoration<IRSemanticDecoration>())
                continue;

            IRStructTypeLayout* structTypeLayout = nullptr;
            if (varLayout)
                structTypeLayout = as<IRStructTypeLayout>(varLayout->getTypeLayout());
            Index fieldIndex = 0;
            List<IRInst*> fieldParams;
            for (auto field : structType->getFields())
            {
                auto fieldParam = builder.emitParam(field->getFieldType());
                IRCloneEnv cloneEnv;
                cloneInstDecorationsAndChildren(
                    &cloneEnv,
                    builder.getModule(),
                    field->getKey(),
                    fieldParam);

                IRVarLayout* fieldLayout =
                    structTypeLayout ? structTypeLayout->getFieldLayout(fieldIndex) : nullptr;
                if (varLayout)
                {
                    IRVarLayout::Builder varLayoutBuilder(&builder, fieldLayout->getTypeLayout());
                    varLayoutBuilder.cloneEverythingButOffsetsFrom(fieldLayout);
                    for (auto offsetAttr : fieldLayout->getOffsetAttrs())
                    {
                        auto parentOffsetAttr =
                            varLayout->findOffsetAttr(offsetAttr->getResourceKind());
                        UInt parentOffset = parentOffsetAttr ? parentOffsetAttr->getOffset() : 0;
                        UInt parentSpace = parentOffsetAttr ? parentOffsetAttr->getSpace() : 0;
                        auto resInfo =
                            varLayoutBuilder.findOrAddResourceInfo(offsetAttr->getResourceKind());
                        resInfo->offset = parentOffset + offsetAttr->getOffset();
                        resInfo->space = parentSpace + offsetAttr->getSpace();
                    }
                    builder.addLayoutDecoration(fieldParam, varLayoutBuilder.build());
                }
                param->insertBefore(fieldParam);
                fieldParams.add(fieldParam);
                fieldIndex++;
            }
            builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
            auto reconstructedParam =
                builder.emitMakeStruct(structType, fieldParams.getCount(), fieldParams.getBuffer());
            param->replaceUsesWith(reconstructedParam);
            param->removeFromParent();
        }
        fixUpFuncType(func);
    }

    // Flattens all struct parameters of an entryPoint to ensure parameters are a flat struct
    void flattenInputParameters(EntryPointInfo entryPoint)
    {
        // Goal is to ensure we have a flattened IRParam (0 nested IRStructType members).
        /*
            // Assume the following code
            struct NestedFragment
            {
                float2 p3;
            };
            struct Fragment
            {
                float4 p1;
                float3 p2;
                NestedFragment p3_nested;
            };

            // Fragment flattens into
            struct Fragment
            {
                float4 p1;
                float3 p2;
                float2 p3;
            };
        */

        // This is important since Metal does not allow semantic's on a struct
        /*
            // Assume the following code
            struct NestedFragment1
            {
                float2 p3;
            };
            struct Fragment1
            {
                float4 p1 : SV_TARGET0;
                float3 p2 : SV_TARGET1;
                NestedFragment p3_nested : SV_TARGET2; // error, semantic on struct
            };

        */

        // Metal does allow semantics on members of a nested struct but we are avoiding this
        // approach since there are senarios where legalization (and verification) is
        // hard/expensive without creating a flat struct:
        // 1. Entry points may share structs, semantics may be inconsistent across entry points
        // 2. Multiple of the same struct may be used in a param list
        /*
            // Assume the following code
            struct NestedFragment
            {
                float2 p3;
            };
            struct Fragment
            {
                float4 p1 : SV_TARGET0;
                NestedFragment p2 : SV_TARGET1;
                NestedFragment p3 : SV_TARGET2;
            };

            // Legalized without flattening -- abandoned
            struct NestedFragment1
            {
                float2 p3 : SV_TARGET1;
            };
            struct NestedFragment2
            {
                float2 p3 : SV_TARGET2;
            };
            struct Fragment
            {
                float4 p1 : SV_TARGET0;
                NestedFragment1 p2;
                NestedFragment2 p3;
            };

            // Legalized with flattening -- current approach
            struct Fragment
            {
                float4 p1 : SV_TARGET0;
                float2 p2 : SV_TARGET1;
                float2 p3 : SV_TARGET2;
            };
        */

        auto func = entryPoint.entryPointFunc;
        bool modified = false;
        for (auto param : func->getParams())
        {
            auto layout = findVarLayout(param);
            if (!layout)
                continue;
            if (!layout->findOffsetAttr(LayoutResourceKind::VaryingInput))
                continue;
            if (param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
                continue;
            // If we find a IRParam with a IRStructType member, we need to flatten the entire
            // IRParam
            if (auto structType = as<IRStructType>(param->getDataType()))
            {
                IRBuilder builder(func);
                MapStructToFlatStruct mapOldFieldToNewField;

                // Flatten struct if we have nested IRStructType
                auto flattenedStruct = maybeFlattenNestedStructs(
                    builder,
                    structType,
                    mapOldFieldToNewField,
                    semanticInfoToRemove);
                if (flattenedStruct != structType)
                {
                    // Validate/rearange all semantics which overlap in our flat struct
                    fixFieldSemanticsOfFlatStruct(flattenedStruct);

                    // Replace the 'old IRParam type' with a 'new IRParam type'
                    param->setFullType(flattenedStruct);

                    // Emit a new variable at EntryPoint of 'old IRParam type'
                    builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
                    auto dstVal = builder.emitVar(structType);
                    auto dstLoad = builder.emitLoad(dstVal);
                    param->replaceUsesWith(dstLoad);
                    builder.setInsertBefore(dstLoad);
                    // Copy the 'new IRParam type' to our 'old IRParam type'
                    mapOldFieldToNewField
                        .emitCopy<(int)MapStructToFlatStruct::CopyOptions::FlatStructIntoStruct>(
                            builder,
                            dstVal,
                            param);

                    modified = true;
                }
            }
        }
        if (modified)
            fixUpFuncType(func);
    }

    void packStageInParameters(EntryPointInfo entryPoint)
    {
        // If the entry point has any parameters whose layout contains VaryingInput,
        // we need to pack those parameters into a single `struct` type, and decorate
        // the fields with the appropriate `[[attribute]]` decorations.
        // For other parameters that are not `VaryingInput`, we need to leave them as is.
        //
        // For example, given this code after `hoistEntryPointParameterFromStruct`:
        // ```
        // void main(float3 pos, float2 uv, int vertexId : SV_VertexID) {
        //     VertexInput vin = {pos,uv,vertexId};
        //     ...
        // }
        // ```
        // We are going to transform it into:
        // ```
        // struct VertexInput {
        //     float3 pos [[attribute(0)]];
        //     float2 uv [[attribute(1)]];
        // };
        // void main(VertexInput vin, int vertexId : SV_VertexID) {
        //     let pos = vin.pos;
        //     let uv = vin.uv;
        //     ...
        // }

        auto func = entryPoint.entryPointFunc;

        bool isGeometryStage = false;
        switch (entryPoint.entryPointDecor->getProfile().getStage())
        {
        case Stage::Vertex:
        case Stage::Amplification:
        case Stage::Mesh:
        case Stage::Geometry:
        case Stage::Domain:
        case Stage::Hull:
            isGeometryStage = true;
            break;
        }

        List<IRParam*> paramsToPack;
        for (auto param : func->getParams())
        {
            auto layout = findVarLayout(param);
            if (!layout)
                continue;
            if (!layout->findOffsetAttr(LayoutResourceKind::VaryingInput))
                continue;
            if (param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
                continue;
            paramsToPack.add(param);
        }

        if (paramsToPack.getCount() == 0)
            return;

        IRBuilder builder(func);
        builder.setInsertBefore(func);
        IRStructType* structType = builder.createStructType();
        auto stageText = getStageText(entryPoint.entryPointDecor->getProfile().getStage());
        builder.addNameHintDecoration(
            structType,
            (String(stageText) + toSlice("Input")).getUnownedSlice());
        List<IRStructKey*> keys;
        IRStructTypeLayout::Builder layoutBuilder(&builder);
        for (auto param : paramsToPack)
        {
            auto paramVarLayout = findVarLayout(param);
            auto key = builder.createStructKey();
            param->transferDecorationsTo(key);
            builder.createStructField(structType, key, param->getDataType());
            if (auto varyingInOffsetAttr =
                    paramVarLayout->findOffsetAttr(LayoutResourceKind::VaryingInput))
            {
                if (!key->findDecoration<IRSemanticDecoration>() &&
                    !paramVarLayout->findAttr<IRSemanticAttr>())
                {
                    // If the parameter doesn't have a semantic, we need to add one for semantic
                    // matching.
                    builder.addSemanticDecoration(
                        key,
                        toSlice("_slang_attr"),
                        (int)varyingInOffsetAttr->getOffset());
                }
            }
            if (isGeometryStage)
            {
                // For geometric stages, we need to translate VaryingInput offsets to MetalAttribute
                // offsets.
                IRVarLayout::Builder elementVarLayoutBuilder(
                    &builder,
                    paramVarLayout->getTypeLayout());
                elementVarLayoutBuilder.cloneEverythingButOffsetsFrom(paramVarLayout);
                for (auto offsetAttr : paramVarLayout->getOffsetAttrs())
                {
                    auto resourceKind = offsetAttr->getResourceKind();
                    if (resourceKind == LayoutResourceKind::VaryingInput)
                    {
                        resourceKind = LayoutResourceKind::MetalAttribute;
                    }
                    auto resInfo = elementVarLayoutBuilder.findOrAddResourceInfo(resourceKind);
                    resInfo->offset = offsetAttr->getOffset();
                    resInfo->space = offsetAttr->getSpace();
                }
                paramVarLayout = elementVarLayoutBuilder.build();
            }
            layoutBuilder.addField(key, paramVarLayout);
            builder.addLayoutDecoration(key, paramVarLayout);
            keys.add(key);
        }
        builder.setInsertInto(func->getFirstBlock());
        auto packedParam = builder.emitParamAtHead(structType);
        auto typeLayout = layoutBuilder.build();
        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);

        // Add a VaryingInput resource info to the packed parameter layout, so that we can emit
        // the needed `[[stage_in]]` attribute in Metal emitter.
        varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::VaryingInput);
        auto paramVarLayout = varLayoutBuilder.build();
        builder.addLayoutDecoration(packedParam, paramVarLayout);

        // Replace the original parameters with the packed parameter
        builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
        for (Index paramIndex = 0; paramIndex < paramsToPack.getCount(); paramIndex++)
        {
            auto param = paramsToPack[paramIndex];
            auto key = keys[paramIndex];
            auto paramField = builder.emitFieldExtract(param->getDataType(), packedParam, key);
            param->replaceUsesWith(paramField);
            param->removeFromParent();
        }
        fixUpFuncType(func);
    }

    struct MetalSystemValueInfo
    {
        String metalSystemValueName;
        SystemValueSemanticName metalSystemValueNameEnum;
        ShortList<IRType*> permittedTypes;
        bool isUnsupported = false;
        bool isSpecial = false;
        MetalSystemValueInfo()
        {
            // most commonly need 2
            permittedTypes.reserveOverflowBuffer(2);
        }
    };

    IRType* getGroupThreadIdType(IRBuilder& builder)
    {
        return builder.getVectorType(
            builder.getBasicType(BaseType::UInt),
            builder.getIntValue(builder.getIntType(), 3));
    }

    // Get all permitted types of "sv_target" for Metal
    ShortList<IRType*>& getPermittedTypes_sv_target(IRBuilder& builder)
    {
        permittedTypes_sv_target.reserveOverflowBuffer(5 * 4);
        if (permittedTypes_sv_target.getCount() == 0)
        {
            for (auto baseType :
                 {BaseType::Float,
                  BaseType::Half,
                  BaseType::Int,
                  BaseType::UInt,
                  BaseType::Int16,
                  BaseType::UInt16})
            {
                for (IRIntegerValue i = 1; i <= 4; i++)
                {
                    permittedTypes_sv_target.add(
                        builder.getVectorType(builder.getBasicType(baseType), i));
                }
            }
        }
        return permittedTypes_sv_target;
    }

    MetalSystemValueInfo getSystemValueInfo(
        String inSemanticName,
        String* optionalSemanticIndex,
        IRInst* parentVar)
    {
        IRBuilder builder(m_module);
        MetalSystemValueInfo result = {};
        UnownedStringSlice semanticName;
        UnownedStringSlice semanticIndex;

        auto hasExplicitIndex =
            splitNameAndIndex(inSemanticName.getUnownedSlice(), semanticName, semanticIndex);
        if (!hasExplicitIndex && optionalSemanticIndex)
            semanticIndex = optionalSemanticIndex->getUnownedSlice();

        result.metalSystemValueNameEnum = convertSystemValueSemanticNameToEnum(semanticName);

        switch (result.metalSystemValueNameEnum)
        {
        case SystemValueSemanticName::Position:
            {
                result.metalSystemValueName = toSlice("position");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::Float),
                    builder.getIntValue(builder.getIntType(), 4)));
                break;
            }
        case SystemValueSemanticName::ClipDistance:
            {
                result.isSpecial = true;
                break;
            }
        case SystemValueSemanticName::CullDistance:
            {
                result.isSpecial = true;
                break;
            }
        case SystemValueSemanticName::Coverage:
            {
                result.metalSystemValueName = toSlice("sample_mask");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                break;
            }
        case SystemValueSemanticName::InnerCoverage:
            {
                result.isSpecial = true;
                break;
            }
        case SystemValueSemanticName::Depth:
            {
                result.metalSystemValueName = toSlice("depth(any)");
                result.permittedTypes.add(builder.getBasicType(BaseType::Float));
                break;
            }
        case SystemValueSemanticName::DepthGreaterEqual:
            {
                result.metalSystemValueName = toSlice("depth(greater)");
                result.permittedTypes.add(builder.getBasicType(BaseType::Float));
                break;
            }
        case SystemValueSemanticName::DepthLessEqual:
            {
                result.metalSystemValueName = toSlice("depth(less)");
                result.permittedTypes.add(builder.getBasicType(BaseType::Float));
                break;
            }
        case SystemValueSemanticName::DispatchThreadID:
            {
                result.metalSystemValueName = toSlice("thread_position_in_grid");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::UInt),
                    builder.getIntValue(builder.getIntType(), 3)));
                break;
            }
        case SystemValueSemanticName::DomainLocation:
            {
                result.metalSystemValueName = toSlice("position_in_patch");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::Float),
                    builder.getIntValue(builder.getIntType(), 3)));
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::Float),
                    builder.getIntValue(builder.getIntType(), 2)));
                break;
            }
        case SystemValueSemanticName::GroupID:
            {
                result.metalSystemValueName = toSlice("threadgroup_position_in_grid");
                result.permittedTypes.add(builder.getVectorType(
                    builder.getBasicType(BaseType::UInt),
                    builder.getIntValue(builder.getIntType(), 3)));
                break;
            }
        case SystemValueSemanticName::GroupIndex:
            {
                result.isSpecial = true;
                break;
            }
        case SystemValueSemanticName::GroupThreadID:
            {
                result.metalSystemValueName = toSlice("thread_position_in_threadgroup");
                result.permittedTypes.add(getGroupThreadIdType(builder));
                break;
            }
        case SystemValueSemanticName::GSInstanceID:
            {
                result.isUnsupported = true;
                break;
            }
        case SystemValueSemanticName::InstanceID:
            {
                result.metalSystemValueName = toSlice("instance_id");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                break;
            }
        case SystemValueSemanticName::IsFrontFace:
            {
                result.metalSystemValueName = toSlice("front_facing");
                result.permittedTypes.add(builder.getBasicType(BaseType::Bool));
                break;
            }
        case SystemValueSemanticName::OutputControlPointID:
            {
                // In metal, a hull shader is just a compute shader.
                // This needs to be handled separately, by lowering into an ordinary buffer.
                break;
            }
        case SystemValueSemanticName::PointSize:
            {
                result.metalSystemValueName = toSlice("point_size");
                result.permittedTypes.add(builder.getBasicType(BaseType::Float));
                break;
            }
        case SystemValueSemanticName::PrimitiveID:
            {
                result.metalSystemValueName = toSlice("primitive_id");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt16));
                break;
            }
        case SystemValueSemanticName::RenderTargetArrayIndex:
            {
                result.metalSystemValueName = toSlice("render_target_array_index");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt16));
                break;
            }
        case SystemValueSemanticName::SampleIndex:
            {
                result.metalSystemValueName = toSlice("sample_id");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                break;
            }
        case SystemValueSemanticName::StencilRef:
            {
                result.metalSystemValueName = toSlice("stencil");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                break;
            }
        case SystemValueSemanticName::TessFactor:
            {
                // Tessellation factor outputs should be lowered into a write into a normal buffer.
                break;
            }
        case SystemValueSemanticName::VertexID:
            {
                result.metalSystemValueName = toSlice("vertex_id");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                break;
            }
        case SystemValueSemanticName::ViewID:
            {
                result.isUnsupported = true;
                break;
            }
        case SystemValueSemanticName::ViewportArrayIndex:
            {
                result.metalSystemValueName = toSlice("viewport_array_index");
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt));
                result.permittedTypes.add(builder.getBasicType(BaseType::UInt16));
                break;
            }
        case SystemValueSemanticName::Target:
            {
                result.metalSystemValueName =
                    (StringBuilder()
                     << "color(" << (semanticIndex.getLength() != 0 ? semanticIndex : toSlice("0"))
                     << ")")
                        .produceString();
                result.permittedTypes = getPermittedTypes_sv_target(builder);

                break;
            }
        default:
            m_sink->diagnose(
                parentVar,
                Diagnostics::unimplementedSystemValueSemantic,
                semanticName);
            return result;
        }
        return result;
    }

    void reportUnsupportedSystemAttribute(IRInst* param, String semanticName)
    {
        m_sink->diagnose(
            param->sourceLoc,
            Diagnostics::systemValueAttributeNotSupported,
            semanticName);
    }

    void ensureResultStructHasUserSemantic(IRStructType* structType, IRVarLayout* varLayout)
    {
        // Ensure each field in an output struct type has either a system semantic or a user
        // semantic, so that signature matching can happen correctly.
        auto typeLayout = as<IRStructTypeLayout>(varLayout->getTypeLayout());
        Index index = 0;
        IRBuilder builder(structType);
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (auto semanticDecor = key->findDecoration<IRSemanticDecoration>())
            {
                if (semanticDecor->getSemanticName().startsWithCaseInsensitive(toSlice("sv_")))
                {
                    auto indexAsString = String(UInt(semanticDecor->getSemanticIndex()));
                    auto sysValInfo =
                        getSystemValueInfo(semanticDecor->getSemanticName(), &indexAsString, field);
                    if (sysValInfo.isUnsupported || sysValInfo.isSpecial)
                    {
                        reportUnsupportedSystemAttribute(field, semanticDecor->getSemanticName());
                    }
                    else
                    {
                        builder.addTargetSystemValueDecoration(
                            key,
                            sysValInfo.metalSystemValueName.getUnownedSlice());
                        semanticDecor->removeAndDeallocate();
                    }
                }
                index++;
                continue;
            }
            typeLayout->getFieldLayout(index);
            auto fieldLayout = typeLayout->getFieldLayout(index);
            if (auto offsetAttr = fieldLayout->findOffsetAttr(LayoutResourceKind::VaryingOutput))
            {
                UInt varOffset = 0;
                if (auto varOffsetAttr =
                        varLayout->findOffsetAttr(LayoutResourceKind::VaryingOutput))
                    varOffset = varOffsetAttr->getOffset();
                varOffset += offsetAttr->getOffset();
                builder.addSemanticDecoration(key, toSlice("_slang_attr"), (int)varOffset);
            }
            index++;
        }
    }

    // Stores a hicharchy of members and children which map 'oldStruct->member' to
    // 'flatStruct->member' Note: this map assumes we map to FlatStruct since it is easier/faster to
    // process
    struct MapStructToFlatStruct
    {
        /*
        We need a hicharchy map to resolve dependencies for mapping
        oldStruct to newStruct efficently. Example:

        MyStruct
            |
          / | \
         /  |  \
        /   |   \
    M0<A> M1<A> M2<B>
       |    |    |
      A_0  A_0  B_0

      Without storing hicharchy information, there will be no way to tell apart
      `myStruct.M0.A0` from `myStruct.M1.A0` since IRStructKey/IRStructField
      only has 1 instance of `A::A0`
      */

        enum CopyOptions : int
        {
            // Copy a flattened-struct into a struct
            FlatStructIntoStruct = 0,

            // Copy a struct into a flattened-struct
            StructIntoFlatStruct = 1,
        };

    private:
        // Children of member if applicable.
        Dictionary<IRStructField*, MapStructToFlatStruct> members;

        // Field correlating to MapStructToFlatStruct Node.
        IRInst* node;
        IRStructKey* getKey()
        {
            SLANG_ASSERT(as<IRStructField>(node));
            return as<IRStructField>(node)->getKey();
        }
        IRInst* getNode() { return node; }
        IRType* getFieldType()
        {
            SLANG_ASSERT(as<IRStructField>(node));
            return as<IRStructField>(node)->getFieldType();
        }

        // Whom node maps to inside target flatStruct
        IRStructField* targetMapping;

        auto begin() { return members.begin(); }
        auto end() { return members.end(); }

        // Copies members of oldStruct to/from newFlatStruct. Assumes members of val1 maps to
        // members in val2 using `MapStructToFlatStruct`
        template<int copyOptions>
        static void _emitCopy(
            IRBuilder& builder,
            IRInst* val1,
            IRStructType* type1,
            IRInst* val2,
            IRStructType* type2,
            MapStructToFlatStruct& node)
        {
            for (auto& field1Pair : node)
            {
                auto& field1 = field1Pair.second;

                // Get member of val1
                IRInst* fieldAddr1 = nullptr;
                if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
                {
                    fieldAddr1 = builder.emitFieldAddress(type1, val1, field1.getKey());
                }
                else
                {
                    if (as<IRPtrTypeBase>(val1))
                        val1 = builder.emitLoad(val1);
                    fieldAddr1 = builder.emitFieldExtract(type1, val1, field1.getKey());
                }

                // If val1 is a struct, recurse
                if (auto fieldAsStruct1 = as<IRStructType>(field1.getFieldType()))
                {
                    _emitCopy<copyOptions>(
                        builder,
                        fieldAddr1,
                        fieldAsStruct1,
                        val2,
                        type2,
                        field1);
                    continue;
                }

                // Get member of val2 which maps to val1.member
                auto field2 = field1.getMapping();
                SLANG_ASSERT(field2);
                IRInst* fieldAddr2 = nullptr;
                if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
                {
                    if (as<IRPtrTypeBase>(val2))
                        val2 = builder.emitLoad(val1);
                    fieldAddr2 = builder.emitFieldExtract(type2, val2, field2->getKey());
                }
                else
                {
                    fieldAddr2 = builder.emitFieldAddress(type2, val2, field2->getKey());
                }

                // Copy val2/val1 member into val1/val2 member
                if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
                {
                    builder.emitStore(fieldAddr1, fieldAddr2);
                }
                else
                {
                    builder.emitStore(fieldAddr2, fieldAddr1);
                }
            }
        }

    public:
        void setNode(IRInst* newNode) { node = newNode; }
        // Get 'MapStructToFlatStruct' that is a child of 'parent'.
        // Make 'MapStructToFlatStruct' if no 'member' is currently mapped to 'parent'.
        MapStructToFlatStruct& getMember(IRStructField* member) { return members[member]; }
        MapStructToFlatStruct& operator[](IRStructField* member) { return getMember(member); }

        void setMapping(IRStructField* newTargetMapping) { targetMapping = newTargetMapping; }
        // Get 'MapStructToFlatStruct' that is a child of 'parent'.
        // Return nullptr if no member is mapped to 'parent'
        IRStructField* getMapping() { return targetMapping; }

        // Copies srcVal into dstVal using hicharchy map.
        template<int copyOptions>
        void emitCopy(IRBuilder& builder, IRInst* dstVal, IRInst* srcVal)
        {
            auto dstType = dstVal->getDataType();
            if (auto dstPtrType = as<IRPtrTypeBase>(dstType))
                dstType = dstPtrType->getValueType();
            auto dstStructType = as<IRStructType>(dstType);
            SLANG_ASSERT(dstStructType);

            auto srcType = srcVal->getDataType();
            if (auto srcPtrType = as<IRPtrTypeBase>(srcType))
                srcType = srcPtrType->getValueType();
            auto srcStructType = as<IRStructType>(srcType);
            SLANG_ASSERT(srcStructType);

            if constexpr (copyOptions == (int)CopyOptions::FlatStructIntoStruct)
            {
                // CopyOptions::FlatStructIntoStruct copy a flattened-struct (mapped member) into a
                // struct
                SLANG_ASSERT(node == dstStructType);
                _emitCopy<copyOptions>(
                    builder,
                    dstVal,
                    dstStructType,
                    srcVal,
                    srcStructType,
                    *this);
            }
            else
            {
                // CopyOptions::StructIntoFlatStruct copy a struct into a flattened-struct
                SLANG_ASSERT(node == srcStructType);
                _emitCopy<copyOptions>(
                    builder,
                    srcVal,
                    srcStructType,
                    dstVal,
                    dstStructType,
                    *this);
            }
        }
    };

    IRStructType* _flattenNestedStructs(
        IRBuilder& builder,
        IRStructType* dst,
        IRStructType* src,
        IRSemanticDecoration* parentSemanticDecoration,
        IRLayoutDecoration* parentLayout,
        MapStructToFlatStruct& mapFieldToField,
        HashSet<IRStructField*>& varsWithSemanticInfo)
    {
        // For all fields ('oldField') of a struct do the following:
        // 1. Check for 'decorations which carry semantic info' (IRSemanticDecoration,
        // IRLayoutDecoration), store these if found.
        //  * Do not propagate semantic info if the current node has *any* form of semantic
        //  information.
        // Update varsWithSemanticInfo.
        // 2. If IRStructType:
        //  2a. Recurse this function with 'decorations that carry semantic info' from parent.
        // 3. If not IRStructType:
        //  3a. Emit 'newField' equal to 'oldField', add 'decorations which carry semantic info'.
        //  3b. Store a mapping from 'oldField' to 'newField' in 'mapFieldToField'. This info is
        //  needed to copy between types.
        for (auto oldField : src->getFields())
        {
            auto& fieldMappingNode = mapFieldToField[oldField];
            fieldMappingNode.setNode(oldField);

            // step 1
            bool foundSemanticDecor = false;
            auto oldKey = oldField->getKey();
            IRSemanticDecoration* fieldSemanticDecoration = parentSemanticDecoration;
            if (auto oldSemanticDecoration = oldKey->findDecoration<IRSemanticDecoration>())
            {
                foundSemanticDecor = true;
                fieldSemanticDecoration = oldSemanticDecoration;
                parentLayout = nullptr;
            }

            IRLayoutDecoration* fieldLayout = parentLayout;
            if (auto oldLayout = oldKey->findDecoration<IRLayoutDecoration>())
            {
                fieldLayout = oldLayout;
                if (!foundSemanticDecor)
                    fieldSemanticDecoration = nullptr;
            }
            if (fieldSemanticDecoration != parentSemanticDecoration || parentLayout != fieldLayout)
                varsWithSemanticInfo.add(oldField);

            // step 2a
            if (auto structFieldType = as<IRStructType>(oldField->getFieldType()))
            {
                _flattenNestedStructs(
                    builder,
                    dst,
                    structFieldType,
                    fieldSemanticDecoration,
                    fieldLayout,
                    fieldMappingNode,
                    varsWithSemanticInfo);
                continue;
            }

            // step 3a
            auto newKey = builder.createStructKey();
            copyNameHintAndDebugDecorations(newKey, oldKey);

            auto newField = builder.createStructField(dst, newKey, oldField->getFieldType());
            copyNameHintAndDebugDecorations(newField, oldField);

            if (fieldSemanticDecoration)
                builder.addSemanticDecoration(
                    newKey,
                    fieldSemanticDecoration->getSemanticName(),
                    fieldSemanticDecoration->getSemanticIndex());

            if (fieldLayout)
            {
                IRLayout* oldLayout = fieldLayout->getLayout();
                List<IRInst*> instToCopy;
                // Only copy certain decorations needed for resolving system semantics
                for (UInt i = 0; i < oldLayout->getOperandCount(); i++)
                {
                    auto operand = oldLayout->getOperand(i);
                    if (as<IRVarOffsetAttr>(operand) || as<IRUserSemanticAttr>(operand) ||
                        as<IRSystemValueSemanticAttr>(operand) || as<IRStageAttr>(operand))
                        instToCopy.add(operand);
                }
                IRVarLayout* newLayout = builder.getVarLayout(instToCopy);
                builder.addLayoutDecoration(newKey, newLayout);
            }
            // step 3b
            fieldMappingNode.setMapping(newField);
        }

        return dst;
    }

    // Returns a `IRStructType*` without any `IRStructType*` members. `src` may be returned if there
    // was no struct flattening.
    // @param mapFieldToField Behavior maps all `IRStructField` of `src` to the new struct
    // `IRStructFields`s
    IRStructType* maybeFlattenNestedStructs(
        IRBuilder& builder,
        IRStructType* src,
        MapStructToFlatStruct& mapFieldToField,
        HashSet<IRStructField*>& varsWithSemanticInfo)
    {
        // Find all values inside struct that need flattening and legalization.
        bool hasStructTypeMembers = false;
        for (auto field : src->getFields())
        {
            if (as<IRStructType>(field->getFieldType()))
            {
                hasStructTypeMembers = true;
                break;
            }
        }
        if (!hasStructTypeMembers)
            return src;

        // We need to:
        // 1. Make new struct 1:1 with old struct but without nestested structs (flatten)
        // 2. Ensure semantic attributes propegate. This will create overlapping semantics (can be
        // handled later).
        // 3. Store the mapping from old to new struct fields to allow copying a old-struct to
        // new-struct.
        builder.setInsertAfter(src);
        auto newStruct = builder.createStructType();
        copyNameHintAndDebugDecorations(newStruct, src);
        mapFieldToField.setNode(src);
        return _flattenNestedStructs(
            builder,
            newStruct,
            src,
            nullptr,
            nullptr,
            mapFieldToField,
            varsWithSemanticInfo);
    }

    // Replaces all 'IRReturn' by copying the current 'IRReturn' to a new var of type 'newType'.
    // Copying logic from 'IRReturn' to 'newType' is controlled by 'copyLogicFunc' function.
    template<typename CopyLogicFunc>
    void _replaceAllReturnInst(
        IRBuilder& builder,
        IRFunc* targetFunc,
        IRStructType* newType,
        CopyLogicFunc copyLogicFunc)
    {
        for (auto block : targetFunc->getBlocks())
        {
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                builder.setInsertBefore(returnInst);
                auto returnVal = returnInst->getVal();
                returnInst->setOperand(0, copyLogicFunc(builder, newType, returnVal));
            }
        }
    }

    UInt _returnNonOverlappingAttributeIndex(std::set<UInt>& usedSemanticIndex)
    {
        // Find first unused semantic index of equal semantic type
        // to fill any gaps in user set semantic bindings
        UInt prev = 0;
        for (auto i : usedSemanticIndex)
        {
            if (i > prev + 1)
            {
                break;
            }
            prev = i;
        }
        usedSemanticIndex.insert(prev + 1);
        return prev + 1;
    }

    template<typename T>
    struct AttributeParentPair
    {
        IRLayoutDecoration* layoutDecor;
        T* attr;
    };

    IRLayoutDecoration* _replaceAttributeOfLayout(
        IRBuilder& builder,
        IRLayoutDecoration* parentLayoutDecor,
        IRInst* instToReplace,
        IRInst* instToReplaceWith)
    {
        // Replace `instToReplace` with a `instToReplaceWith`

        auto layout = parentLayoutDecor->getLayout();
        // Find the exact same decoration `instToReplace` in-case multiple of the same type exist
        List<IRInst*> opList;
        opList.add(instToReplaceWith);
        for (UInt i = 0; i < layout->getOperandCount(); i++)
        {
            if (layout->getOperand(i) != instToReplace)
                opList.add(layout->getOperand(i));
        }
        auto newLayoutDecor = builder.addLayoutDecoration(
            parentLayoutDecor->getParent(),
            builder.getVarLayout(opList));
        parentLayoutDecor->removeAndDeallocate();
        return newLayoutDecor;
    }

    IRLayoutDecoration* _simplifyUserSemanticNames(
        IRBuilder& builder,
        IRLayoutDecoration* layoutDecor)
    {
        // Ensure all 'ExplicitIndex' semantics such as "SV_TARGET0" are simplified into
        // ("SV_TARGET", 0) using 'IRUserSemanticAttr' This is done to ensure we can check semantic
        // groups using 'IRUserSemanticAttr1->getName() == IRUserSemanticAttr2->getName()'
        SLANG_ASSERT(layoutDecor);
        auto layout = layoutDecor->getLayout();
        List<IRInst*> layoutOps;
        layoutOps.reserve(3);
        bool changed = false;
        for (auto attr : layout->getAllAttrs())
        {
            if (auto userSemantic = as<IRUserSemanticAttr>(attr))
            {
                UnownedStringSlice outName;
                UnownedStringSlice outIndex;
                bool hasStringIndex = splitNameAndIndex(userSemantic->getName(), outName, outIndex);
                if (hasStringIndex)
                {
                    changed = true;
                    auto loweredName = String(outName).toLower();
                    auto loweredNameSlice = loweredName.getUnownedSlice();
                    auto newDecoration =
                        builder.getUserSemanticAttr(loweredNameSlice, stringToInt(outIndex));
                    userSemantic->replaceUsesWith(newDecoration);
                    userSemantic->removeAndDeallocate();
                    userSemantic = newDecoration;
                }
                layoutOps.add(userSemantic);
                continue;
            }
            layoutOps.add(attr);
        }
        if (changed)
        {
            auto parent = layoutDecor->parent;
            layoutDecor->removeAndDeallocate();
            builder.addLayoutDecoration(parent, builder.getVarLayout(layoutOps));
        }
        return layoutDecor;
    }
    // Find overlapping field semantics and legalize them
    void fixFieldSemanticsOfFlatStruct(IRStructType* structType)
    {
        // Goal is to ensure we do not have overlapping semantics:
        /*
            // Assume the following code
            struct Fragment
            {
                float4 p1 : SV_TARGET;
                float3 p2 : SV_TARGET;
                float2 p3 : SV_TARGET;
                float2 p4 : SV_TARGET;
            };

            // Translates into
            struct Fragment
            {
                float4 p1 : SV_TARGET0;
                float3 p2 : SV_TARGET1;
                float2 p3 : SV_TARGET2;
                float2 p4 : SV_TARGET3;
            };
        */

        IRBuilder builder(this->m_module);

        List<IRSemanticDecoration*> overlappingSemanticsDecor;
        Dictionary<UnownedStringSlice, std::set<UInt, std::less<UInt>>>
            usedSemanticIndexSemanticDecor;

        List<AttributeParentPair<IRVarOffsetAttr>> overlappingVarOffset;
        Dictionary<UInt, std::set<UInt, std::less<UInt>>> usedSemanticIndexVarOffset;

        List<AttributeParentPair<IRUserSemanticAttr>> overlappingUserSemantic;
        Dictionary<UnownedStringSlice, std::set<UInt, std::less<UInt>>>
            usedSemanticIndexUserSemantic;

        // We store a map from old `IRLayoutDecoration*` to new `IRLayoutDecoration*` since when
        // legalizing we may destroy and remake a `IRLayoutDecoration*`
        Dictionary<IRLayoutDecoration*, IRLayoutDecoration*> oldLayoutDecorToNew;

        // Collect all "semantic info carrying decorations". Any collected decoration will
        // fill up their respective 'Dictionary<SEMANTIC_TYPE, OrderedHashSet<UInt>>'
        // to keep track of in-use offsets for a semantic type.
        // Example: IRSemanticDecoration with name of "SV_TARGET1".
        // * This will have SEMANTIC_TYPE of "sv_target".
        // * This will use up index '1'
        //
        // Now if a second equal semantic "SV_TARGET1" is found, we add this decoration to
        // a list of 'overlapping semantic info decorations' so we can legalize this
        // 'semantic info decoration' later.
        //
        // NOTE: this is a flat struct, all members are children of the initial
        // IRStructType.
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (auto semanticDecoration = key->findDecoration<IRSemanticDecoration>())
            {
                // Ensure names are in a uniform lowercase format so we can bunch together simmilar
                // semantics
                UnownedStringSlice outName;
                UnownedStringSlice outIndex;
                bool hasStringIndex =
                    splitNameAndIndex(semanticDecoration->getSemanticName(), outName, outIndex);
                if (hasStringIndex)
                {
                    auto loweredName = String(outName).toLower();
                    auto loweredNameSlice = loweredName.getUnownedSlice();
                    auto newDecoration =
                        builder.addSemanticDecoration(key, loweredNameSlice, stringToInt(outIndex));
                    semanticDecoration->replaceUsesWith(newDecoration);
                    semanticDecoration->removeAndDeallocate();
                    semanticDecoration = newDecoration;
                }
                auto& semanticUse =
                    usedSemanticIndexSemanticDecor[semanticDecoration->getSemanticName()];
                if (semanticUse.find(semanticDecoration->getSemanticIndex()) != semanticUse.end())
                    overlappingSemanticsDecor.add(semanticDecoration);
                else
                    semanticUse.insert(semanticDecoration->getSemanticIndex());
            }
            if (auto layoutDecor = key->findDecoration<IRLayoutDecoration>())
            {
                // Ensure names are in a uniform lowercase format so we can bunch together simmilar
                // semantics
                layoutDecor = _simplifyUserSemanticNames(builder, layoutDecor);
                oldLayoutDecorToNew[layoutDecor] = layoutDecor;
                auto layout = layoutDecor->getLayout();
                for (auto attr : layout->getAllAttrs())
                {
                    if (auto offset = as<IRVarOffsetAttr>(attr))
                    {
                        auto& semanticUse = usedSemanticIndexVarOffset[offset->getResourceKind()];
                        if (semanticUse.find(offset->getOffset()) != semanticUse.end())
                            overlappingVarOffset.add({layoutDecor, offset});
                        else
                            semanticUse.insert(offset->getOffset());
                    }
                    else if (auto userSemantic = as<IRUserSemanticAttr>(attr))
                    {
                        auto& semanticUse = usedSemanticIndexUserSemantic[userSemantic->getName()];
                        if (semanticUse.find(userSemantic->getIndex()) != semanticUse.end())
                            overlappingUserSemantic.add({layoutDecor, userSemantic});
                        else
                            semanticUse.insert(userSemantic->getIndex());
                    }
                }
            }
        }

        // Legalize all overlapping 'semantic info decorations'
        for (auto decor : overlappingSemanticsDecor)
        {
            auto newOffset = _returnNonOverlappingAttributeIndex(
                usedSemanticIndexSemanticDecor[decor->getSemanticName()]);
            builder.addSemanticDecoration(
                decor->getParent(),
                decor->getSemanticName(),
                (int)newOffset);
            decor->removeAndDeallocate();
        }
        for (auto& varOffset : overlappingVarOffset)
        {
            auto newOffset = _returnNonOverlappingAttributeIndex(
                usedSemanticIndexVarOffset[varOffset.attr->getResourceKind()]);
            auto newVarOffset = builder.getVarOffsetAttr(
                varOffset.attr->getResourceKind(),
                newOffset,
                varOffset.attr->getSpace());
            oldLayoutDecorToNew[varOffset.layoutDecor] = _replaceAttributeOfLayout(
                builder,
                oldLayoutDecorToNew[varOffset.layoutDecor],
                varOffset.attr,
                newVarOffset);
        }
        for (auto& userSemantic : overlappingUserSemantic)
        {
            auto newOffset = _returnNonOverlappingAttributeIndex(
                usedSemanticIndexUserSemantic[userSemantic.attr->getName()]);
            auto newUserSemantic =
                builder.getUserSemanticAttr(userSemantic.attr->getName(), newOffset);
            oldLayoutDecorToNew[userSemantic.layoutDecor] = _replaceAttributeOfLayout(
                builder,
                oldLayoutDecorToNew[userSemantic.layoutDecor],
                userSemantic.attr,
                newUserSemantic);
        }
    }

    void wrapReturnValueInStruct(EntryPointInfo entryPoint)
    {
        // Wrap return value into a struct if it is not already a struct.
        // For example, given this entry point:
        // ```
        // float4 main() : SV_Target { return float3(1,2,3); }
        // ```
        // We are going to transform it into:
        // ```
        // struct Output {
        //     float4 value : SV_Target;
        // };
        // Output main() { return {float3(1,2,3)}; }

        auto func = entryPoint.entryPointFunc;

        auto returnType = func->getResultType();
        if (as<IRVoidType>(returnType))
            return;
        auto entryPointLayoutDecor = func->findDecoration<IRLayoutDecoration>();
        if (!entryPointLayoutDecor)
            return;
        auto entryPointLayout = as<IREntryPointLayout>(entryPointLayoutDecor->getLayout());
        if (!entryPointLayout)
            return;
        auto resultLayout = entryPointLayout->getResultLayout();

        // If return type is already a struct, just make sure every field has a semantic.
        if (auto returnStructType = as<IRStructType>(returnType))
        {
            IRBuilder builder(func);
            MapStructToFlatStruct mapOldFieldToNewField;
            // Flatten result struct type to ensure we do not have nested semantics
            auto flattenedStruct = maybeFlattenNestedStructs(
                builder,
                returnStructType,
                mapOldFieldToNewField,
                semanticInfoToRemove);
            if (returnStructType != flattenedStruct)
            {
                // Replace all return-values with the flattenedStruct we made.
                _replaceAllReturnInst(
                    builder,
                    func,
                    flattenedStruct,
                    [&](IRBuilder& copyBuilder, IRStructType* dstType, IRInst* srcVal) -> IRInst*
                    {
                        auto srcStructType = as<IRStructType>(srcVal->getDataType());
                        SLANG_ASSERT(srcStructType);
                        auto dstVal = copyBuilder.emitVar(dstType);
                        mapOldFieldToNewField.emitCopy<(
                            int)MapStructToFlatStruct::CopyOptions::StructIntoFlatStruct>(
                            copyBuilder,
                            dstVal,
                            srcVal);
                        return builder.emitLoad(dstVal);
                    });
                fixUpFuncType(func, flattenedStruct);
            }
            // Ensure non-overlapping semantics
            fixFieldSemanticsOfFlatStruct(flattenedStruct);
            ensureResultStructHasUserSemantic(flattenedStruct, resultLayout);
            return;
        }

        IRBuilder builder(func);
        builder.setInsertBefore(func);
        IRStructType* structType = builder.createStructType();
        auto stageText = getStageText(entryPoint.entryPointDecor->getProfile().getStage());
        builder.addNameHintDecoration(
            structType,
            (String(stageText) + toSlice("Output")).getUnownedSlice());
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, toSlice("output"));
        builder.addLayoutDecoration(key, resultLayout);
        builder.createStructField(structType, key, returnType);
        IRStructTypeLayout::Builder structTypeLayoutBuilder(&builder);
        structTypeLayoutBuilder.addField(key, resultLayout);
        auto typeLayout = structTypeLayoutBuilder.build();
        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
        auto varLayout = varLayoutBuilder.build();
        ensureResultStructHasUserSemantic(structType, varLayout);

        _replaceAllReturnInst(
            builder,
            func,
            structType,
            [](IRBuilder& copyBuilder, IRStructType* dstType, IRInst* srcVal) -> IRInst*
            { return copyBuilder.emitMakeStruct(dstType, 1, &srcVal); });

        // Assign an appropriate system value semantic for stage output
        auto stage = entryPoint.entryPointDecor->getProfile().getStage();
        switch (stage)
        {
        case Stage::Compute:
        case Stage::Fragment:
            {
                builder.addTargetSystemValueDecoration(key, toSlice("color(0)"));
                break;
            }
        case Stage::Vertex:
            {
                builder.addTargetSystemValueDecoration(key, toSlice("position"));
                break;
            }
        default:
            SLANG_ASSERT(false);
            return;
        }

        fixUpFuncType(func, structType);
    }

    void legalizeMeshEntryPoint(EntryPointInfo entryPoint)
    {
        auto func = entryPoint.entryPointFunc;

        IRBuilder builder{func->getModule()};
        for (auto param : func->getParams())
        {
            if (param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
            {
                IRVarLayout::Builder varLayoutBuilder(
                    &builder,
                    IRTypeLayout::Builder{&builder}.build());

                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                auto paramVarLayout = varLayoutBuilder.build();
                builder.addLayoutDecoration(param, paramVarLayout);

                IRPtrTypeBase* type = as<IRPtrTypeBase>(param->getDataType());

                const auto annotatedPayloadType = builder.getPtrType(
                    kIROp_ConstRefType,
                    type->getValueType(),
                    AddressSpace::MetalObjectData);

                param->setFullType(annotatedPayloadType);
            }
        }
        IROutputTopologyDecoration* outputDeco =
            entryPoint.entryPointFunc->findDecoration<IROutputTopologyDecoration>();
        if (outputDeco == nullptr)
        {
            SLANG_UNEXPECTED("Mesh shader output decoration missing");
            return;
        }
        const auto topology = outputDeco->getTopology();
        const auto topStr = topology->getStringSlice();
        UInt topologyEnum = 0;
        if (topStr.caseInsensitiveEquals(toSlice("point")))
        {
            topologyEnum = 1;
        }
        else if (topStr.caseInsensitiveEquals(toSlice("line")))
        {
            topologyEnum = 2;
        }
        else if (topStr.caseInsensitiveEquals(toSlice("triangle")))
        {
            topologyEnum = 3;
        }
        else
        {
            SLANG_UNEXPECTED("unknown topology");
            return;
        }

        IRInst* topologyConst = builder.getIntValue(builder.getIntType(), topologyEnum);

        IRType* vertexType = nullptr;
        IRType* indicesType = nullptr;
        IRType* primitiveType = nullptr;

        IRInst* maxVertices = nullptr;
        IRInst* maxPrimitives = nullptr;

        IRInst* verticesParam = nullptr;
        IRInst* indicesParam = nullptr;
        IRInst* primitivesParam = nullptr;
        for (auto param : func->getParams())
        {
            if (param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
            {
                IRVarLayout::Builder varLayoutBuilder(
                    &builder,
                    IRTypeLayout::Builder{&builder}.build());

                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                auto paramVarLayout = varLayoutBuilder.build();
                builder.addLayoutDecoration(param, paramVarLayout);
            }
            if (param->findDecorationImpl(kIROp_VerticesDecoration))
            {
                auto vertexRefType = as<IRPtrTypeBase>(param->getDataType());
                auto vertexOutputType = as<IRVerticesType>(vertexRefType->getValueType());
                vertexType = vertexOutputType->getElementType();
                maxVertices = vertexOutputType->getMaxElementCount();
                SLANG_ASSERT(vertexType);

                verticesParam = param;
                auto vertStruct = as<IRStructType>(vertexType);
                for (auto field : vertStruct->getFields())
                {
                    auto key = field->getKey();
                    if (auto deco = key->findDecoration<IRSemanticDecoration>())
                    {
                        if (deco->getSemanticName().caseInsensitiveEquals(toSlice("sv_position")))
                        {
                            builder.addTargetSystemValueDecoration(key, toSlice("position"));
                        }
                    }
                }
            }
            if (param->findDecorationImpl(kIROp_IndicesDecoration))
            {
                auto indicesRefType = (IRConstRefType*)param->getDataType();
                auto indicesOutputType = (IRIndicesType*)indicesRefType->getValueType();
                indicesType = indicesOutputType->getElementType();
                maxPrimitives = indicesOutputType->getMaxElementCount();
                SLANG_ASSERT(indicesType);

                indicesParam = param;
            }
            if (param->findDecorationImpl(kIROp_PrimitivesDecoration))
            {
                auto primitivesRefType = (IRConstRefType*)param->getDataType();
                auto primitivesOutputType = (IRPrimitivesType*)primitivesRefType->getValueType();
                primitiveType = primitivesOutputType->getElementType();
                SLANG_ASSERT(primitiveType);

                primitivesParam = param;
                auto primStruct = as<IRStructType>(primitiveType);
                for (auto field : primStruct->getFields())
                {
                    auto key = field->getKey();
                    if (auto deco = key->findDecoration<IRSemanticDecoration>())
                    {
                        if (deco->getSemanticName().caseInsensitiveEquals(
                                toSlice("sv_primitiveid")))
                        {
                            builder.addTargetSystemValueDecoration(key, toSlice("primitive_id"));
                        }
                    }
                }
            }
        }
        if (primitiveType == nullptr)
        {
            primitiveType = builder.getVoidType();
        }
        builder.setInsertBefore(entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());

        auto meshParam = builder.emitParam(builder.getMetalMeshType(
            vertexType,
            primitiveType,
            maxVertices,
            maxPrimitives,
            topologyConst));
        builder.addExternCppDecoration(meshParam, toSlice("_slang_mesh"));


        verticesParam->removeFromParent();
        verticesParam->removeAndDeallocate();

        indicesParam->removeFromParent();
        indicesParam->removeAndDeallocate();

        if (primitivesParam != nullptr)
        {
            primitivesParam->removeFromParent();
            primitivesParam->removeAndDeallocate();
        }
    }

    void legalizeDispatchMeshPayloadForMetal(EntryPointInfo entryPoint)
    {
        // Find out DispatchMesh function
        IRGlobalValueWithCode* dispatchMeshFunc = nullptr;
        for (const auto globalInst : entryPoint.entryPointFunc->getModule()->getGlobalInsts())
        {
            if (const auto func = as<IRGlobalValueWithCode>(globalInst))
            {
                if (const auto dec = func->findDecoration<IRKnownBuiltinDecoration>())
                {
                    if (dec->getName() == "DispatchMesh")
                    {
                        SLANG_ASSERT(!dispatchMeshFunc && "Multiple DispatchMesh functions found");
                        dispatchMeshFunc = func;
                    }
                }
            }
        }

        if (!dispatchMeshFunc)
            return;

        IRBuilder builder{entryPoint.entryPointFunc->getModule()};

        // We'll rewrite the call to use mesh_grid_properties.set_threadgroups_per_grid
        traverseUses(
            dispatchMeshFunc,
            [&](const IRUse* use)
            {
                if (const auto call = as<IRCall>(use->getUser()))
                {
                    SLANG_ASSERT(call->getArgCount() == 4);
                    const auto payload = call->getArg(3);

                    const auto payloadPtrType =
                        composeGetters<IRPtrType>(payload, &IRInst::getDataType);
                    SLANG_ASSERT(payloadPtrType);
                    const auto payloadType = payloadPtrType->getValueType();
                    SLANG_ASSERT(payloadType);

                    builder.setInsertBefore(
                        entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());
                    const auto annotatedPayloadType = builder.getPtrType(
                        kIROp_RefType,
                        payloadPtrType->getValueType(),
                        AddressSpace::MetalObjectData);
                    auto packedParam = builder.emitParam(annotatedPayloadType);
                    builder.addExternCppDecoration(packedParam, toSlice("_slang_mesh_payload"));
                    IRVarLayout::Builder varLayoutBuilder(
                        &builder,
                        IRTypeLayout::Builder{&builder}.build());

                    // Add the MetalPayload resource info, so we can emit [[payload]]
                    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                    auto paramVarLayout = varLayoutBuilder.build();
                    builder.addLayoutDecoration(packedParam, paramVarLayout);

                    // Now we replace the call to DispatchMesh with a call to the mesh grid
                    // properties But first we need to create the parameter
                    const auto meshGridPropertiesType = builder.getMetalMeshGridPropertiesType();
                    auto mgp = builder.emitParam(meshGridPropertiesType);
                    builder.addExternCppDecoration(mgp, toSlice("_slang_mgp"));
                }
            });
    }

    IRInst* tryConvertValue(IRBuilder& builder, IRInst* val, IRType* toType)
    {
        auto fromType = val->getFullType();
        if (auto fromVector = as<IRVectorType>(fromType))
        {
            if (auto toVector = as<IRVectorType>(toType))
            {
                if (fromVector->getElementCount() != toVector->getElementCount())
                {
                    fromType = builder.getVectorType(
                        fromVector->getElementType(),
                        toVector->getElementCount());
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

    struct SystemValLegalizationWorkItem
    {
        IRInst* var;
        String attrName;
        UInt attrIndex;
    };

    std::optional<SystemValLegalizationWorkItem> tryToMakeSystemValWorkItem(IRInst* var)
    {
        if (auto semanticDecoration = var->findDecoration<IRSemanticDecoration>())
        {
            if (semanticDecoration->getSemanticName().startsWithCaseInsensitive(toSlice("sv_")))
            {
                return {
                    {var,
                     String(semanticDecoration->getSemanticName()).toLower(),
                     (UInt)semanticDecoration->getSemanticIndex()}};
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

        return {{var, semanticName, sysAttrIndex}};
    }


    List<SystemValLegalizationWorkItem> collectSystemValFromEntryPoint(EntryPointInfo entryPoint)
    {
        List<SystemValLegalizationWorkItem> systemValWorkItems;
        for (auto param : entryPoint.entryPointFunc->getParams())
        {
            auto maybeWorkItem = tryToMakeSystemValWorkItem(param);
            if (maybeWorkItem.has_value())
                systemValWorkItems.add(std::move(maybeWorkItem.value()));
        }
        return systemValWorkItems;
    }

    void legalizeSystemValue(EntryPointInfo entryPoint, SystemValLegalizationWorkItem& workItem)
    {
        IRBuilder builder(entryPoint.entryPointFunc);

        auto var = workItem.var;
        auto semanticName = workItem.attrName;

        auto indexAsString = String(workItem.attrIndex);
        auto info = getSystemValueInfo(semanticName, &indexAsString, var);

        if (info.isSpecial)
        {
            if (info.metalSystemValueNameEnum == SystemValueSemanticName::InnerCoverage)
            {
                // Metal does not support conservative rasterization, so this is always false.
                auto val = builder.getBoolValue(false);
                var->replaceUsesWith(val);
                var->removeAndDeallocate();
            }
            else if (info.metalSystemValueNameEnum == SystemValueSemanticName::GroupIndex)
            {
                // Ensure we have a cached "sv_groupthreadid" in our entry point
                if (!entryPointToGroupThreadId.containsKey(entryPoint.entryPointFunc))
                {
                    auto systemValWorkItems = collectSystemValFromEntryPoint(entryPoint);
                    for (auto i : systemValWorkItems)
                    {
                        auto indexAsStringGroupThreadId = String(i.attrIndex);
                        if (getSystemValueInfo(i.attrName, &indexAsStringGroupThreadId, i.var)
                                .metalSystemValueNameEnum == SystemValueSemanticName::GroupThreadID)
                        {
                            entryPointToGroupThreadId[entryPoint.entryPointFunc] = i.var;
                        }
                    }
                    if (!entryPointToGroupThreadId.containsKey(entryPoint.entryPointFunc))
                    {
                        // Add the missing groupthreadid needed to compute sv_groupindex
                        IRBuilder groupThreadIdBuilder(builder);
                        groupThreadIdBuilder.setInsertInto(
                            entryPoint.entryPointFunc->getFirstBlock());
                        auto groupThreadId = groupThreadIdBuilder.emitParamAtHead(
                            getGroupThreadIdType(groupThreadIdBuilder));
                        entryPointToGroupThreadId[entryPoint.entryPointFunc] = groupThreadId;
                        groupThreadIdBuilder.addNameHintDecoration(
                            groupThreadId,
                            groupThreadIDString);

                        // Since "sv_groupindex" will be translated out to a global var and no
                        // longer be considered a system value we can reuse its layout and semantic
                        // info
                        Index foundRequiredDecorations = 0;
                        IRLayoutDecoration* layoutDecoration = nullptr;
                        UInt semanticIndex = 0;
                        for (auto decoration : var->getDecorations())
                        {
                            if (auto layoutDecorationTmp = as<IRLayoutDecoration>(decoration))
                            {
                                layoutDecoration = layoutDecorationTmp;
                                foundRequiredDecorations++;
                            }
                            else if (auto semanticDecoration = as<IRSemanticDecoration>(decoration))
                            {
                                semanticIndex = semanticDecoration->getSemanticIndex();
                                groupThreadIdBuilder.addSemanticDecoration(
                                    groupThreadId,
                                    groupThreadIDString,
                                    (int)semanticIndex);
                                foundRequiredDecorations++;
                            }
                            if (foundRequiredDecorations >= 2)
                                break;
                        }
                        SLANG_ASSERT(layoutDecoration);
                        layoutDecoration->removeFromParent();
                        layoutDecoration->insertAtStart(groupThreadId);
                        SystemValLegalizationWorkItem newWorkItem = {
                            groupThreadId,
                            groupThreadIDString,
                            semanticIndex};
                        legalizeSystemValue(entryPoint, newWorkItem);
                    }
                }

                IRBuilder svBuilder(builder.getModule());
                svBuilder.setInsertBefore(entryPoint.entryPointFunc->getFirstOrdinaryInst());
                auto computeExtent = emitCalcGroupExtents(
                    svBuilder,
                    entryPoint.entryPointFunc,
                    builder.getVectorType(
                        builder.getUIntType(),
                        builder.getIntValue(builder.getIntType(), 3)));
                auto groupIndexCalc = emitCalcGroupIndex(
                    svBuilder,
                    entryPointToGroupThreadId[entryPoint.entryPointFunc],
                    computeExtent);
                svBuilder.addNameHintDecoration(
                    groupIndexCalc,
                    UnownedStringSlice("sv_groupindex"));

                var->replaceUsesWith(groupIndexCalc);
                var->removeAndDeallocate();
            }
        }
        if (info.isUnsupported)
        {
            reportUnsupportedSystemAttribute(var, semanticName);
            return;
        }
        if (!info.permittedTypes.getCount())
            return;

        builder.addTargetSystemValueDecoration(var, info.metalSystemValueName.getUnownedSlice());

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
                    entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());

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
                        typeNameSB.produceString());
                }
            }
        }
    }

    void legalizeSystemValueParameters(EntryPointInfo entryPoint)
    {
        List<SystemValLegalizationWorkItem> systemValWorkItems =
            collectSystemValFromEntryPoint(entryPoint);

        for (auto index = 0; index < systemValWorkItems.getCount(); index++)
        {
            legalizeSystemValue(entryPoint, systemValWorkItems[index]);
        }
        fixUpFuncType(entryPoint.entryPointFunc);
    }

    void legalizeEntryPointForMetal(EntryPointInfo entryPoint)
    {
        // Input Parameter Legalize
        depointerizeInputParams(entryPoint.entryPointFunc);
        hoistEntryPointParameterFromStruct(entryPoint);
        packStageInParameters(entryPoint);
        flattenInputParameters(entryPoint);

        // System Value Legalize
        legalizeSystemValueParameters(entryPoint);

        // Output Value Legalize
        wrapReturnValueInStruct(entryPoint);

        // Other Legalize
        switch (entryPoint.entryPointDecor->getProfile().getStage())
        {
        case Stage::Amplification:
            legalizeDispatchMeshPayloadForMetal(entryPoint);
            break;
        case Stage::Mesh:
            legalizeMeshEntryPoint(entryPoint);
            break;
        default:
            break;
        }
    }
};

// metal textures only support writing 4-component values, even if the texture is only 1, 2, or
// 3-component in this case the other channels get ignored, but the signature still doesnt match so
// now we have to replace the value being written with a 4-component vector where the new components
// get ignored, nice
void legalizeImageStoreValue(IRBuilder& builder, IRImageStore* imageStore)
{
    builder.setInsertBefore(imageStore);
    auto originalValue = imageStore->getValue();
    auto valueBaseType = originalValue->getDataType();
    IRType* elementType = nullptr;
    List<IRInst*> components;
    if (auto valueVectorType = as<IRVectorType>(valueBaseType))
    {
        if (auto originalElementCount = as<IRIntLit>(valueVectorType->getElementCount()))
        {
            if (originalElementCount->getValue() == 4)
            {
                return;
            }
        }
        elementType = valueVectorType->getElementType();
        auto vectorValue = as<IRMakeVector>(originalValue);
        for (UInt i = 0; i < vectorValue->getOperandCount(); i++)
        {
            components.add(vectorValue->getOperand(i));
        }
    }
    else
    {
        elementType = valueBaseType;
        components.add(originalValue);
    }
    for (UInt i = components.getCount(); i < 4; i++)
    {
        components.add(builder.getIntValue(builder.getIntType(), 0));
    }
    auto fourComponentVectorType = builder.getVectorType(elementType, 4);
    imageStore->setOperand(2, builder.emitMakeVector(fourComponentVectorType, components));
}

void legalizeFuncBody(IRFunc* func)
{
    IRBuilder builder(func);
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getModifiableChildren())
        {
            if (auto call = as<IRCall>(inst))
            {
                ShortList<IRUse*> argsToFixup;
                // Metal doesn't support taking the address of a vector element.
                // If such an address is used as an argument to a call, we need to replace it with a
                // temporary. for example, if we see:
                // ```
                //     void foo(inout float x) { x = 1; }
                //     float4 v;
                //     foo(v.x);
                // ```
                // We need to transform it into:
                // ```
                //     float4 v;
                //     float temp = v.x;
                //     foo(temp);
                //     v.x = temp;
                // ```
                //
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    if (auto addr = as<IRGetElementPtr>(call->getArg(i)))
                    {
                        auto ptrType = addr->getBase()->getDataType();
                        auto valueType = tryGetPointedToType(&builder, ptrType);
                        if (!valueType)
                            continue;
                        if (as<IRVectorType>(valueType))
                            argsToFixup.add(call->getArgs() + i);
                    }
                }
                if (argsToFixup.getCount() == 0)
                    continue;

                // Define temp vars for all args that need fixing up.
                for (auto arg : argsToFixup)
                {
                    auto addr = as<IRGetElementPtr>(arg->get());
                    auto ptrType = addr->getDataType();
                    auto valueType = tryGetPointedToType(&builder, ptrType);
                    builder.setInsertBefore(call);
                    auto temp = builder.emitVar(valueType);
                    auto initialValue = builder.emitLoad(valueType, addr);
                    builder.emitStore(temp, initialValue);
                    builder.setInsertAfter(call);
                    builder.emitStore(addr, builder.emitLoad(valueType, temp));
                    arg->set(temp);
                }
            }
            if (auto write = as<IRImageStore>(inst))
            {
                legalizeImageStoreValue(builder, write);
            }
        }
    }
}

struct MetalAddressSpaceAssigner : InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst* inst, AddressSpace& outAddressSpace) override
    {
        switch (inst->getOp())
        {
        case kIROp_Var:
            outAddressSpace = AddressSpace::ThreadLocal;
            return true;
        case kIROp_RWStructuredBufferGetElementPtr:
            outAddressSpace = AddressSpace::Global;
            return true;
        default:
            return false;
        }
    }

    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type) override
    {
        if (as<IRUniformParameterGroupType>(type))
        {
            return AddressSpace::Uniform;
        }
        if (as<IRByteAddressBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRHLSLStructuredBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRGLSLShaderStorageBufferType>(type))
        {
            return AddressSpace::Global;
        }
        if (auto ptrType = as<IRPtrTypeBase>(type))
        {
            if (ptrType->hasAddressSpace())
                return ptrType->getAddressSpace();
            return AddressSpace::Global;
        }
        return AddressSpace::Generic;
    }

    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst) override
    {
        if (as<IRGroupSharedRate>(inst->getRate()))
            return AddressSpace::GroupShared;
        switch (inst->getOp())
        {
        case kIROp_RWStructuredBufferGetElementPtr:
            return AddressSpace::Global;
        case kIROp_Var:
            if (as<IRBlock>(inst->getParent()))
                return AddressSpace::ThreadLocal;
            break;
        default:
            break;
        }
        auto type = unwrapAttributedType(inst->getDataType());
        if (!type)
            return AddressSpace::Generic;
        return getAddressSpaceFromVarType(type);
    }
};

void legalizeIRForMetal(IRModule* module, DiagnosticSink* sink)
{
    List<EntryPointInfo> entryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (auto entryPointDecor = func->findDecoration<IREntryPointDecoration>())
            {
                EntryPointInfo info;
                info.entryPointDecor = entryPointDecor;
                info.entryPointFunc = func;
                entryPoints.add(info);
            }
            legalizeFuncBody(func);
        }
    }

    LegalizeMetalEntryPointContext context(sink, module);
    for (auto entryPoint : entryPoints)
        context.legalizeEntryPointForMetal(entryPoint);
    context.removeSemanticLayoutsFromLegalizedStructs();

    MetalAddressSpaceAssigner metalAddressSpaceAssigner;
    specializeAddressSpace(module, &metalAddressSpaceAssigner);
}

} // namespace Slang
