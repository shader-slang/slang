#include "slang-ir-metal-legalize.h"

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
            if (auto structType = as<IRStructType>(param->getDataType()))
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


    void ensureResultStructHasUserSemantic(IRStructType* structType, IRVarLayout* varLayout)
    {
        // Ensure each field in an output struct type has either a system semantic or a user semantic,
        // so that signature matching can happen correctly.
        auto typeLayout = as<IRStructTypeLayout>(varLayout->getTypeLayout());
        Index index = 0;
        IRBuilder builder(structType);
        for (auto field : structType->getFields())
        {
            auto key = field->getKey();
            if (key->findDecoration<IRSemanticDecoration>())
            {
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
            ensureResultStructHasUserSemantic(returnStructType, resultLayout);
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
        builder.createStructField(structType, key, returnType);
        IRStructTypeLayout::Builder structTypeLayoutBuilder(&builder);
        structTypeLayoutBuilder.addField(key, resultLayout);
        auto typeLayout = structTypeLayoutBuilder.build();
        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
        auto varLayout = varLayoutBuilder.build();
        ensureResultStructHasUserSemantic(structType, varLayout);

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

    void legalizeEntryPointForMetal(EntryPointInfo entryPoint, DiagnosticSink* sink)
    {
        SLANG_UNUSED(sink);

        hoistEntryPointParameterFromStruct(entryPoint);
        packStageInParameters(entryPoint);
        wrapReturnValueInStruct(entryPoint);
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
