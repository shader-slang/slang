#include "slang-ir-metal-legalize.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-specialize-address-space.h"

namespace Slang
{
    struct EntryPointInfo
    {
        IRFunc* entryPointFunc;
        IREntryPointDecoration* entryPointDecor;
    };

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
                cloneInstDecorationsAndChildren(&cloneEnv, builder.getModule(), field->getKey(), fieldParam);

                IRVarLayout* fieldLayout = structTypeLayout ? structTypeLayout->getFieldLayout(fieldIndex) : nullptr;
                if (varLayout)
                {
                    IRVarLayout::Builder varLayoutBuilder(&builder, fieldLayout->getTypeLayout());
                    varLayoutBuilder.cloneEverythingButOffsetsFrom(fieldLayout);
                    for (auto offsetAttr : fieldLayout->getOffsetAttrs())
                    {
                        auto parentOffsetAttr = varLayout->findOffsetAttr(offsetAttr->getResourceKind());
                        UInt parentOffset = parentOffsetAttr ? parentOffsetAttr->getOffset() : 0;
                        UInt parentSpace = parentOffsetAttr ? parentOffsetAttr->getSpace() : 0;
                        auto resInfo = varLayoutBuilder.findOrAddResourceInfo(offsetAttr->getResourceKind());
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
            auto reconstructedParam = builder.emitMakeStruct(structType, fieldParams.getCount(), fieldParams.getBuffer());
            param->replaceUsesWith(reconstructedParam);
            param->removeFromParent();
        }
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
            if(param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
                continue;
            paramsToPack.add(param);
        }

        if (paramsToPack.getCount() == 0)
            return;

        IRBuilder builder(func);
        builder.setInsertBefore(func);
        IRStructType* structType = builder.createStructType();
        auto stageText = getStageText(entryPoint.entryPointDecor->getProfile().getStage());
        builder.addNameHintDecoration(structType, (String(stageText) + toSlice("Input")).getUnownedSlice());
        List<IRStructKey*> keys;
        IRStructTypeLayout::Builder layoutBuilder(&builder);
        for (auto param : paramsToPack)
        {
            auto paramVarLayout = findVarLayout(param);
            auto key = builder.createStructKey();
            param->transferDecorationsTo(key);
            builder.createStructField(structType, key, param->getDataType());
            if (auto varyingInOffsetAttr = paramVarLayout->findOffsetAttr(LayoutResourceKind::VaryingInput))
            {
                if (!key->findDecoration<IRSemanticDecoration>() && !paramVarLayout->findAttr<IRSemanticAttr>())
                {
                    // If the parameter doesn't have a semantic, we need to add one for semantic matching.
                    builder.addSemanticDecoration(key, toSlice("_slang_attr"), (int)varyingInOffsetAttr->getOffset());
                }
            }
            if (isGeometryStage)
            {
                // For geometric stages, we need to translate VaryingInput offsets to MetalAttribute offsets.
                IRVarLayout::Builder elementVarLayoutBuilder(&builder, paramVarLayout->getTypeLayout());
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
        IRType* requiredType;
        IRType* altRequiredType;
        bool isUnsupported;
        bool isSpecial;
    };

    MetalSystemValueInfo getSystemValueInfo(IRBuilder& builder, String semanticName, UInt attrIndex)
    {
        SLANG_UNUSED(attrIndex);

        MetalSystemValueInfo result = {};

        semanticName = semanticName.toLower();

        if (semanticName == "sv_position")
        {
            result.metalSystemValueName = toSlice("position");
            result.requiredType = builder.getVectorType(builder.getBasicType(BaseType::Float), builder.getIntValue(builder.getIntType(), 4));
        }
        else if (semanticName == "sv_clipdistance")
        {
            result.isSpecial = true;
        }
        else if (semanticName == "sv_culldistance")
        {
            result.isSpecial = true;
        }
        else if (semanticName == "sv_coverage")
        {
            result.metalSystemValueName = toSlice("sample_mask");
            result.requiredType = builder.getBasicType(BaseType::UInt);
        }
        else if (semanticName == "sv_innercoverage")
        {
            result.isSpecial = true;

        }
        else if (semanticName == "sv_depth")
        {
            result.metalSystemValueName = toSlice("depth(any)");
            result.requiredType = builder.getBasicType(BaseType::Float);
        }
        else if (semanticName == "sv_depthgreaterequal")
        {
            result.metalSystemValueName = toSlice("depth(greater)");
            result.requiredType = builder.getBasicType(BaseType::Float);
        }
        else if (semanticName == "sv_depthlessequal")
        {
            result.metalSystemValueName = toSlice("depth(less)");
            result.requiredType = builder.getBasicType(BaseType::Float);
        }
        else if (semanticName == "sv_dispatchthreadid")
        {
            result.metalSystemValueName = toSlice("thread_position_in_grid");
            result.requiredType = builder.getVectorType(builder.getBasicType(BaseType::UInt), builder.getIntValue(builder.getIntType(), 3));
        }
        else if (semanticName == "sv_domainlocation")
        {
            result.metalSystemValueName = toSlice("position_in_patch");
            result.requiredType = builder.getVectorType(builder.getBasicType(BaseType::Float), builder.getIntValue(builder.getIntType(), 3));
            result.altRequiredType = builder.getVectorType(builder.getBasicType(BaseType::Float), builder.getIntValue(builder.getIntType(), 2));
        }
        else if (semanticName == "sv_groupid")
        {
            result.isSpecial = true;
        }
        else if (semanticName == "sv_groupindex")
        {
            result.isSpecial = true;
        }
        else if (semanticName == "sv_groupthreadid")
        {
            result.metalSystemValueName = toSlice("thread_position_in_threadgroup");
            result.requiredType = builder.getVectorType(builder.getBasicType(BaseType::UInt), builder.getIntValue(builder.getIntType(), 3));
        }
        else if (semanticName == "sv_gsinstanceid")
        {
            // Metal does not have geometry shader, so this is invalid.
            result.isUnsupported = true;
        }
        else if (semanticName == "sv_instanceid")
        {
            result.metalSystemValueName = toSlice("instance_id");
            result.requiredType = builder.getBasicType(BaseType::Int);
        }
        else if (semanticName == "sv_isfrontface")
        {
            result.metalSystemValueName = toSlice("front_facing");
            result.requiredType = builder.getBasicType(BaseType::Bool);
        }
        else if (semanticName == "sv_outputcontrolpointid")
        {
            // In metal, a hull shader is just a compute shader.
            // This needs to be handled separately, by lowering into an ordinary buffer.
        }
        else if (semanticName == "sv_pointsize")
        {
            result.metalSystemValueName = toSlice("point_size");
            result.requiredType = builder.getBasicType(BaseType::Float);
        }
        else if (semanticName == "sv_primitiveid")
        {
            result.metalSystemValueName = toSlice("patch_id");
            result.requiredType = builder.getBasicType(BaseType::UInt);
            result.altRequiredType = builder.getBasicType(BaseType::UInt16);
        }
        else if (semanticName == "sv_rendertargetarrayindex")
        {
            result.metalSystemValueName = toSlice("render_target_array_index");
            result.requiredType = builder.getBasicType(BaseType::UInt);
            result.altRequiredType = builder.getBasicType(BaseType::UInt16);
        }
        else if (semanticName == "sv_sampleindex")
        {
            result.metalSystemValueName = toSlice("sample_id");
            result.requiredType = builder.getBasicType(BaseType::UInt);
        }
        else if (semanticName == "sv_stencilref")
        {
            result.metalSystemValueName = toSlice("stencil");
            result.requiredType = builder.getBasicType(BaseType::UInt);
        }
        else if (semanticName == "sv_tessfactor")
        {
            // Tessellation factor outputs should be lowered into a write into a normal buffer.
        }
        else if (semanticName == "sv_vertexid")
        {
            result.metalSystemValueName = toSlice("vertex_id");
            result.requiredType = builder.getBasicType(BaseType::UInt);
        }
        else if (semanticName == "sv_viewid")
        {
            result.isUnsupported = true;
        }
        else if (semanticName == "sv_viewportarrayindex")
        {
            result.metalSystemValueName = toSlice("viewport_array_index");
            result.requiredType = builder.getBasicType(BaseType::UInt);
            result.altRequiredType = builder.getBasicType(BaseType::UInt16);
        }
        else if (semanticName == "sv_target")
        {
            result.metalSystemValueName = (StringBuilder() << "color(" << String(attrIndex) << ")").produceString();
        }
        else
        {
            result.isUnsupported = true;
        }
        return result;
    }

    void reportUnsupportedSystemAttribute(DiagnosticSink* sink, IRInst* param, String semanticName)
    {
        sink->diagnose(param->sourceLoc, Diagnostics::systemValueAttributeNotSupported, semanticName);
    }

    void ensureResultStructHasUserSemantic(DiagnosticSink* sink, IRStructType* structType, IRVarLayout* varLayout)
    {
        // Ensure each field in an output struct type has either a system semantic or a user semantic,
        // so that signature matching can happen correctly.
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
                    auto sysValInfo = getSystemValueInfo(builder, semanticDecor->getSemanticName(), semanticDecor->getSemanticIndex());
                    if (sysValInfo.isUnsupported || sysValInfo.isSpecial)
                    {
                        reportUnsupportedSystemAttribute(sink, field, semanticDecor->getSemanticName());
                    }
                    else
                    {
                        builder.addTargetSystemValueDecoration(key, sysValInfo.metalSystemValueName.getUnownedSlice());
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
                if (auto varOffsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::VaryingOutput))
                    varOffset = varOffsetAttr->getOffset();
                varOffset += offsetAttr->getOffset();
                builder.addSemanticDecoration(key, toSlice("_slang_attr"), (int)varOffset);
            }
            index++;
        }
    }


    void wrapReturnValueInStruct(DiagnosticSink* sink, EntryPointInfo entryPoint)
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
            ensureResultStructHasUserSemantic(sink, returnStructType, resultLayout);
            return;
        }

        // If not, we need to wrap the result into a struct type.
        IRBuilder builder(func);
        builder.setInsertBefore(func);
        IRStructType* structType = builder.createStructType();
        auto stageText = getStageText(entryPoint.entryPointDecor->getProfile().getStage());
        builder.addNameHintDecoration(structType, (String(stageText) + toSlice("Output")).getUnownedSlice());
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, toSlice("output"));
        builder.addLayoutDecoration(key, resultLayout);
        builder.addTargetSystemValueDecoration(key, toSlice("color(0)"));
        builder.createStructField(structType, key, returnType);
        IRStructTypeLayout::Builder structTypeLayoutBuilder(&builder);
        structTypeLayoutBuilder.addField(key, resultLayout);
        auto typeLayout = structTypeLayoutBuilder.build();
        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
        auto varLayout = varLayoutBuilder.build();
        ensureResultStructHasUserSemantic(sink, structType, varLayout);

        for (auto block : func->getBlocks())
        {
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                builder.setInsertBefore(returnInst);
                auto returnVal = returnInst->getVal();
                auto newResult = builder.emitMakeStruct(structType, 1, &returnVal);
                returnInst->setOperand(0, newResult);
            }
        }
        fixUpFuncType(func, structType);
    }

    void legalizeMeshEntryPoint(EntryPointInfo entryPoint)
    {
        auto func = entryPoint.entryPointFunc;

        if (entryPoint.entryPointDecor->getProfile().getStage() != Stage::Mesh)
        {
            return;
        }

        IRBuilder builder{ entryPoint.entryPointFunc->getModule() };
        for (auto param : func->getParams())
        {
            if(param->findDecorationImpl(kIROp_HLSLMeshPayloadDecoration))
            {
                IRVarLayout::Builder varLayoutBuilder(&builder, IRTypeLayout::Builder{&builder}.build());

                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                auto paramVarLayout = varLayoutBuilder.build();
                builder.addLayoutDecoration(param, paramVarLayout);
            }
        }

    }

    void legalizeDispatchMeshPayloadForMetal(EntryPointInfo entryPoint)
    {
        if (entryPoint.entryPointDecor->getProfile().getStage() != Stage::Amplification)
        {
            return;
        }
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

        IRBuilder builder{ entryPoint.entryPointFunc->getModule() };

        // We'll rewrite the call to use mesh_grid_properties.set_threadgroups_per_grid
        traverseUses(dispatchMeshFunc, [&](const IRUse* use) {
            if (const auto call = as<IRCall>(use->getUser()))
            {
                SLANG_ASSERT(call->getArgCount() == 4);
                const auto payload = call->getArg(3);

                const auto payloadPtrType = composeGetters<IRPtrType>(
                    payload,
                    &IRInst::getDataType
                );
                SLANG_ASSERT(payloadPtrType);
                const auto payloadType = payloadPtrType->getValueType();
                SLANG_ASSERT(payloadType);

                builder.setInsertBefore(entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());
                const auto annotatedPayloadType =
                    builder.getPtrType(
                        kIROp_RefType,
                        payloadPtrType->getValueType(),
                        AddressSpace::MetalObjectData
                    );
                auto packedParam = builder.emitParam(annotatedPayloadType);
                builder.addExternCppDecoration(packedParam, toSlice("_slang_mesh_payload"));
                IRVarLayout::Builder varLayoutBuilder(&builder, IRTypeLayout::Builder{&builder}.build());

                // Add the MetalPayload resource info, so we can emit [[payload]]
                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::MetalPayload);
                auto paramVarLayout = varLayoutBuilder.build();
                builder.addLayoutDecoration(packedParam, paramVarLayout);

                // Now we replace the call to DispatchMesh with a call to the mesh grid properties
                // But first we need to create the parameter
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
                    fromType = builder.getVectorType(fromVector->getElementType(), toVector->getElementCount());
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

    void legalizeSystemValueParameters(EntryPointInfo entryPoint, DiagnosticSink* sink)
    {
        SLANG_UNUSED(sink);

        struct SystemValLegalizationWorkItem
        {
            IRParam* param;
            String attrName;
            UInt attrIndex;
        };
        List<SystemValLegalizationWorkItem> systemValWorkItems;
        List<SystemValLegalizationWorkItem> workList;

        IRBuilder builder(entryPoint.entryPointFunc);
        List<IRParam*> params;

        for (auto param : entryPoint.entryPointFunc->getParams())
        {
            if (auto semanticDecoration = param->findDecoration<IRSemanticDecoration>())
            {
                if (semanticDecoration->getSemanticName().startsWithCaseInsensitive(toSlice("sv_")))
                {
                    systemValWorkItems.add({ param, String(semanticDecoration->getSemanticName()).toLower(), (UInt)semanticDecoration->getSemanticIndex() });
                    continue;
                }
            }

            auto layoutDecor = param->findDecoration<IRLayoutDecoration>();
            if (!layoutDecor)
                continue;
            auto sysValAttr = layoutDecor->findAttr<IRSystemValueSemanticAttr>();
            if (!sysValAttr)
                continue;
            auto semanticName = String(sysValAttr->getName());
            auto sysAttrIndex = sysValAttr->getIndex();
            systemValWorkItems.add({ param, semanticName, sysAttrIndex });
        }
        for (auto workItem : systemValWorkItems)
        {
            auto param = workItem.param;
            auto semanticName = workItem.attrName;
            auto sysAttrIndex = workItem.attrIndex;

            auto info = getSystemValueInfo(builder, semanticName, sysAttrIndex);
            if (info.isSpecial)
            {
                if (semanticName == "sv_innercoverage")
                {
                    // Metal does not support conservative rasterization, so this is always false.
                    auto val = builder.getBoolValue(false);
                    param->replaceUsesWith(val);
                    param->removeAndDeallocate();
                }
                else
                {
                    // Process special cases after trivial cases.
                    workList.add(workItem);
                }
            }
            if (info.isUnsupported)
            {
                reportUnsupportedSystemAttribute(sink, param, semanticName);
                continue;
            }
            if (!info.requiredType)
                continue;

            builder.addTargetSystemValueDecoration(param, info.metalSystemValueName.getUnownedSlice());

            // If the required type is different from the actual type, we need to insert a conversion.
            if (info.requiredType != param->getFullType() && info.altRequiredType != param->getFullType())
            {
                auto targetType = param->getFullType();
                builder.setInsertBefore(entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());
                param->setFullType(info.requiredType);
                List<IRUse*> uses;
                for (auto use = param->firstUse; use; use = use->nextUse)
                    uses.add(use);
                auto convertedValue = tryConvertValue(builder, param, targetType);
                copyNameHintAndDebugDecorations(convertedValue, param);
                if (!convertedValue)
                {
                    // If we can't convert the value, report an error.
                    StringBuilder typeNameSB;
                    getTypeNameHint(typeNameSB, info.requiredType);
                    sink->diagnose(param->sourceLoc, Diagnostics::systemValueTypeIncompatible, semanticName, typeNameSB.produceString());
                }
                else
                {
                    for (auto use : uses)
                        builder.replaceOperand(use, convertedValue);
                }
            }
        }
        fixUpFuncType(entryPoint.entryPointFunc);
    }

    void legalizeEntryPointForMetal(EntryPointInfo entryPoint, DiagnosticSink* sink)
    {
        hoistEntryPointParameterFromStruct(entryPoint);
        packStageInParameters(entryPoint);
        legalizeSystemValueParameters(entryPoint, sink);
        wrapReturnValueInStruct(sink, entryPoint);
        legalizeMeshEntryPoint(entryPoint);
        legalizeDispatchMeshPayloadForMetal(entryPoint);
    }


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
            }
        }

        for (auto entryPoint : entryPoints)
            legalizeEntryPointForMetal(entryPoint, sink);

        specializeAddressSpace(module);
    }

}

