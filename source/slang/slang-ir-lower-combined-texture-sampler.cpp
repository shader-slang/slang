#include "slang-ir-lower-combined-texture-sampler.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"

namespace Slang
{
struct LoweredCombinedSamplerStructInfo
{
    IRStructKey* texture;
    IRStructKey* sampler;
    IRStructType* type;
    IRType* samplerType;
    IRType* textureType;
    IRTypeLayout* typeLayout;
};

IRTextureTypeBase* isCombinedTextureSamplerType(IRInst* typeInst)
{
    auto textureType = as<IRTextureTypeBase>(typeInst);
    if (!textureType)
        return nullptr;
    if (!textureType->isCombined())
        return nullptr;
    return textureType;
}

struct LowerCombinedSamplerContext
{
    Dictionary<IRType*, LoweredCombinedSamplerStructInfo> mapTypeToLoweredInfo;
    Dictionary<IRType*, LoweredCombinedSamplerStructInfo> mapLoweredTypeToLoweredInfo;
    CodeGenTarget codeGenTarget;

    std::optional<LoweredCombinedSamplerStructInfo> getLoweredTypeInfo(
        IRType* textureTypeOrLoweredType)
    {
        if (auto combinedSamplerType = as<IRTextureTypeBase>(textureTypeOrLoweredType))
        {
            return lowerCombinedTextureSamplerType(combinedSamplerType);
        }
        else
        {
            auto loweredInfoPtr = mapLoweredTypeToLoweredInfo.tryGetValue(textureTypeOrLoweredType);
            if (!loweredInfoPtr)
                return std::nullopt;
            return *loweredInfoPtr;
        }
    }

    LoweredCombinedSamplerStructInfo lowerCombinedTextureSamplerType(IRTextureTypeBase* textureType)
    {
        if (auto loweredInfo = mapTypeToLoweredInfo.tryGetValue(textureType))
            return *loweredInfo;
        LoweredCombinedSamplerStructInfo info;
        IRBuilder builder(textureType);
        builder.setInsertBefore(textureType);
        auto structType = builder.createStructType();
        StringBuilder sb;
        getTypeNameHint(sb, textureType);
        builder.addNameHintDecoration(structType, sb.getUnownedSlice());
        info.sampler = builder.createStructKey();
        builder.addNameHintDecoration(info.sampler, toSlice("sampler"));
        info.texture = builder.createStructKey();
        builder.addNameHintDecoration(info.texture, toSlice("texture"));
        info.type = structType;

        bool isMutable =
            getIntVal(textureType->getAccessInst()) == kCoreModule_ResourceAccessReadOnly ? false
                                                                                          : true;

        info.textureType = getTextureTypeFromCombinedTextureSampler(textureType);
        builder.createStructField(structType, info.texture, info.textureType);
        info.samplerType = getSamplerTypeFromCombinedTextureSampler(textureType);
        builder.createStructField(structType, info.sampler, info.samplerType);

        // Type layout.

        bool isWGSLTarget = codeGenTarget == CodeGenTarget::WGSL;
        LayoutResourceKind textureResourceKind =
            isMutable ? LayoutResourceKind::UnorderedAccess : LayoutResourceKind::ShaderResource;
        LayoutResourceKind samplerResourceKind = LayoutResourceKind::SamplerState;
        if (isWGSLTarget)
        {
            textureResourceKind = LayoutResourceKind::DescriptorTableSlot;
            samplerResourceKind = LayoutResourceKind::DescriptorTableSlot;
        }

        IRTypeLayout::Builder textureTypeLayoutBuilder(&builder);
        textureTypeLayoutBuilder.addResourceUsage(textureResourceKind, LayoutSize(1));
        auto textureTypeLayout = textureTypeLayoutBuilder.build();

        IRTypeLayout::Builder samplerTypeLayoutBuilder(&builder);
        samplerTypeLayoutBuilder.addResourceUsage(samplerResourceKind, LayoutSize(1));
        auto samplerTypeLayout = samplerTypeLayoutBuilder.build();

        IRVarLayout::Builder textureVarLayoutBuilder(&builder, textureTypeLayout);
        textureVarLayoutBuilder.findOrAddResourceInfo(textureResourceKind)->offset = 0;
        auto textureVarLayout = textureVarLayoutBuilder.build();

        IRVarLayout::Builder samplerVarLayoutBuilder(&builder, samplerTypeLayout);
        samplerVarLayoutBuilder.findOrAddResourceInfo(samplerResourceKind)->offset =
            isWGSLTarget ? 1u : 0u;
        auto samplerVarLayout = samplerVarLayoutBuilder.build();

        IRStructTypeLayout::Builder layoutBuilder(&builder);
        layoutBuilder.addField(info.texture, textureVarLayout);
        layoutBuilder.addField(info.sampler, samplerVarLayout);
        info.typeLayout = layoutBuilder.build();
        builder.addLayoutDecoration(structType, info.typeLayout);

        mapTypeToLoweredInfo.add(textureType, info);
        mapLoweredTypeToLoweredInfo.add(info.type, info);
        return info;
    }

    /// Return `typeLayout` with the layout of every combined texture-sampler inside `type`
    /// replaced by the layout of its lowered `{texture, sampler}` struct, rebuilding each
    /// enclosing array, struct and parameter-group layout on the way up while keeping their
    /// resource usage. Returns `typeLayout` itself when nothing inside it changes.
    ///
    /// Consider `struct S { float4 c; Sampler2D s; }; ConstantBuffer<S> cb;`. Once `Sampler2D`
    /// becomes a struct, type legalization looks up the layouts of the `texture` and `sampler`
    /// fields under `cb.s`, so the field layout of `s` has to become a struct layout too.
    /// Layout insts are hoisted and shared between parameters, so we build new layouts rather
    /// than modifying existing ones.
    ///
    IRTypeLayout* lowerTypeLayout(IRBuilder* builder, IRType* type, IRTypeLayout* typeLayout)
    {
        if (auto textureType = isCombinedTextureSamplerType(type))
            return lowerCombinedTextureSamplerType(textureType).typeLayout;

        if (auto arrayType = as<IRArrayTypeBase>(type))
        {
            auto arrayTypeLayout = as<IRArrayTypeLayout>(typeLayout);
            if (!arrayTypeLayout)
                return typeLayout;
            auto elementTypeLayout = arrayTypeLayout->getElementTypeLayout();
            auto newElementTypeLayout =
                lowerTypeLayout(builder, arrayType->getElementType(), elementTypeLayout);
            if (newElementTypeLayout == elementTypeLayout)
                return typeLayout;
            IRArrayTypeLayout::Builder newArrayTypeLayoutBuilder(builder, newElementTypeLayout);
            newArrayTypeLayoutBuilder.addResourceUsageFrom(typeLayout);
            return newArrayTypeLayoutBuilder.build();
        }

        if (auto structType = as<IRStructType>(type))
        {
            auto structTypeLayout = as<IRStructTypeLayout>(typeLayout);
            if (!structTypeLayout)
                return typeLayout;
            bool anyFieldChanged = false;
            IRStructTypeLayout::Builder newStructTypeLayoutBuilder(builder);
            newStructTypeLayoutBuilder.addResourceUsageFrom(typeLayout);
            for (auto fieldAttr : structTypeLayout->getFieldLayoutAttrs())
            {
                auto fieldKey = fieldAttr->getFieldKey();
                auto fieldLayout = fieldAttr->getLayout();
                auto newFieldLayout = fieldLayout;
                if (auto field = findStructField(structType, as<IRStructKey>(fieldKey)))
                    newFieldLayout = lowerVarLayout(builder, field->getFieldType(), fieldLayout);
                anyFieldChanged |= newFieldLayout != fieldLayout;
                newStructTypeLayoutBuilder.addField(fieldKey, newFieldLayout);
            }
            if (!anyFieldChanged)
                return typeLayout;
            return newStructTypeLayoutBuilder.build();
        }

        if (auto parameterGroupType = as<IRParameterGroupType>(type))
        {
            auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout);
            if (!parameterGroupTypeLayout)
                return typeLayout;
            auto elementType = parameterGroupType->getElementType();
            auto elementVarLayout = parameterGroupTypeLayout->getElementVarLayout();
            auto newElementVarLayout = lowerVarLayout(builder, elementType, elementVarLayout);
            if (newElementVarLayout == elementVarLayout)
                return typeLayout;
            // The offset element type layout is what `getFieldLayout` reads through a
            // parameter group, so it is lowered along with the element var layout.
            IRParameterGroupTypeLayout::Builder newParameterGroupTypeLayoutBuilder(builder);
            newParameterGroupTypeLayoutBuilder.addResourceUsageFrom(typeLayout);
            newParameterGroupTypeLayoutBuilder.setContainerVarLayout(
                parameterGroupTypeLayout->getContainerVarLayout());
            newParameterGroupTypeLayoutBuilder.setElementVarLayout(newElementVarLayout);
            newParameterGroupTypeLayoutBuilder.setOffsetElementTypeLayout(lowerTypeLayout(
                builder,
                elementType,
                parameterGroupTypeLayout->getOffsetElementTypeLayout()));
            return newParameterGroupTypeLayoutBuilder.build();
        }

        return typeLayout;
    }

    /// Return `varLayout` with its type layout lowered by `lowerTypeLayout` for a variable of
    /// `type`, keeping all of its offsets. Returns `varLayout` itself when nothing changes.
    IRVarLayout* lowerVarLayout(IRBuilder* builder, IRType* type, IRVarLayout* varLayout)
    {
        auto typeLayout = varLayout->getTypeLayout();
        auto newTypeLayout = lowerTypeLayout(builder, type, typeLayout);
        if (newTypeLayout == typeLayout)
            return varLayout;

        IRVarLayout::Builder newVarLayoutBuilder(builder, newTypeLayout);
        newVarLayoutBuilder.cloneEverythingButOffsetsFrom(varLayout);
        IRVarOffsetAttr* resOffsetAttr = nullptr;
        IRVarOffsetAttr* descriptorTableSlotOffsetAttr = nullptr;

        for (auto offsetAttr : varLayout->getOffsetAttrs())
        {
            LayoutResourceKind resKind = offsetAttr->getResourceKind();
            if (resKind == LayoutResourceKind::UnorderedAccess ||
                resKind == LayoutResourceKind::ShaderResource)
                resOffsetAttr = offsetAttr;
            else if (resKind == LayoutResourceKind::DescriptorTableSlot)
                descriptorTableSlotOffsetAttr = offsetAttr;
            auto info = newVarLayoutBuilder.findOrAddResourceInfo(resKind);
            info->offset = offsetAttr->getOffset();
            info->space = offsetAttr->getSpace();
            info->kind = offsetAttr->getResourceKind();
        }
        // If the user provided an layout offset for the texture but not for descriptor table
        // slot, then we use the texture offset for the descriptor table slot offset.
        if (isCombinedTextureSamplerType(unwrapArray(type)) && resOffsetAttr &&
            !descriptorTableSlotOffsetAttr)
        {
            auto info =
                newVarLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::DescriptorTableSlot);
            info->offset = resOffsetAttr->getOffset();
            info->space = resOffsetAttr->getSpace();
            info->kind = LayoutResourceKind::DescriptorTableSlot;
        }
        return newVarLayoutBuilder.build();
    }
};

void lowerCombinedTextureSamplers(
    IRModule* module,
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    LowerCombinedSamplerContext context;
    context.codeGenTarget = codeGenContext->getTargetFormat();

    bool hasCombinedSampler = false;

    // Lower the layout of every shader parameter that contains a combined texture sampler,
    // lowering each combined texture sampler type it reaches into a struct type.
    for (auto globalInst : module->getGlobalInsts())
    {
        if (isCombinedTextureSamplerType(globalInst))
            hasCombinedSampler = true;
        auto globalParam = as<IRGlobalParam>(globalInst);
        if (!globalParam)
            continue;
        auto layoutDecor = globalParam->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            continue;
        auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
        if (!varLayout)
            continue;
        IRBuilder subBuilder(globalInst);
        subBuilder.setInsertBefore(globalInst);

        auto newVarLayout =
            context.lowerVarLayout(&subBuilder, globalParam->getFullType(), varLayout);
        if (newVarLayout == varLayout)
            continue;
        subBuilder.addLayoutDecoration(globalParam, newVarLayout);
        layoutDecor->removeAndDeallocate();
    }

    // If no combined texture sampler type exist in the IR module,
    // we can exit now.
    if (!hasCombinedSampler)
        return;

    // We need to process all insts in the module, and replace
    // CombinedTextureSamplerGetTexture and CombinedTextureSamplerGetSampler into
    // FieldExtracts.
    IRBuilder builder(module);
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(getGenericReturnVal(globalInst));
        if (!func)
            continue;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getModifiableChildren())
            {
                switch (inst->getOp())
                {
                case kIROp_CombinedTextureSamplerGetTexture:
                case kIROp_CombinedTextureSamplerGetSampler:
                    {
                        auto loweredInfo =
                            context.getLoweredTypeInfo(inst->getOperand(0)->getDataType());
                        if (!loweredInfo)
                            continue;
                        builder.setInsertBefore(inst);
                        auto fieldExtract = builder.emitFieldExtract(
                            inst->getFullType(),
                            inst->getOperand(0),
                            inst->getOp() == kIROp_CombinedTextureSamplerGetSampler
                                ? loweredInfo->sampler
                                : loweredInfo->texture);
                        inst->replaceUsesWith(fieldExtract);
                        inst->removeAndDeallocate();
                    }
                    break;
                case kIROp_CastDescriptorHandleToResource:
                    {
                        auto handle = inst->getOperand(0);
                        if (as<IRDescriptorHandleType>(handle->getDataType()))
                        {
                            // If handle is still a DescriptorHandle, we are on a target that
                            // where native resource handles are already bindless, e.g. metal.
                            // On these platforms, the handle is a struct containing texture
                            // and sampler fields, so we just need to insert the extract operations.
                            auto loweredInfo = context.getLoweredTypeInfo(inst->getDataType());
                            if (!loweredInfo)
                                continue;
                            builder.setInsertBefore(inst);
                            auto textureVal = builder.emitFieldExtract(
                                loweredInfo->textureType,
                                handle,
                                loweredInfo->texture);
                            auto samplerVal = builder.emitFieldExtract(
                                loweredInfo->samplerType,
                                handle,
                                loweredInfo->sampler);
                            IRInst* args[] = {textureVal, samplerVal};
                            auto combinedSampler =
                                builder.emitMakeStruct(loweredInfo->type, 2, args);
                            inst->replaceUsesWith(combinedSampler);
                            inst->removeAndDeallocate();
                        }
                    }
                    break;

                case kIROp_MakeCombinedTextureSamplerFromHandle:
                    {
                        auto loweredInfo = context.getLoweredTypeInfo(inst->getDataType());
                        if (!loweredInfo)
                            continue;
                        auto handle = inst->getOperand(0);
                        builder.setInsertBefore(inst);
                        auto textureIndex = builder.emitElementExtract(handle, IRIntegerValue(0));
                        auto texture = builder.emitIntrinsicInst(
                            loweredInfo->textureType,
                            kIROp_LoadResourceDescriptorFromHeap,
                            1,
                            &textureIndex);
                        auto samplerIndex = builder.emitElementExtract(handle, IRIntegerValue(1));
                        auto sampler = builder.emitIntrinsicInst(
                            loweredInfo->samplerType,
                            kIROp_LoadSamplerDescriptorFromHeap,
                            1,
                            &samplerIndex);
                        IRInst* args[] = {texture, sampler};
                        auto combinedSampler = builder.emitMakeStruct(loweredInfo->type, 2, args);
                        inst->replaceUsesWith(combinedSampler);
                        inst->removeAndDeallocate();
                    }
                    break;
                }
            }
        }
    }

    // Replace all other type use with the lowered struct type.
    for (auto typeInfo : context.mapTypeToLoweredInfo)
    {
        auto loweredInfo = typeInfo.second;
        typeInfo.first->replaceUsesWith(loweredInfo.type);
        typeInfo.first->removeAndDeallocate();
    }
}

} // namespace Slang
