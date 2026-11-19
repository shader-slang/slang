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
    // We replace every use of each type recorded here across the whole module, and IR types are
    // shared, so only combined texture-sampler types may be recorded.
    Dictionary<IRType*, LoweredCombinedSamplerStructInfo> mapTypeToLoweredInfo;
    Dictionary<IRType*, LoweredCombinedSamplerStructInfo> mapLoweredTypeToLoweredInfo;
    CodeGenTarget codeGenTarget;

    // Return the lowered struct info for a combined texture-sampler type, lowering it on first
    // use, or for a struct type this pass already produced for one. Return `std::nullopt` for
    // any other type, including a plain texture, which this pass leaves unchanged.
    std::optional<LoweredCombinedSamplerStructInfo> getLoweredTypeInfo(
        IRType* textureTypeOrLoweredType)
    {
        if (auto combinedSamplerType = isCombinedTextureSamplerType(textureTypeOrLoweredType))
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
        SLANG_RELEASE_ASSERT(textureType->isCombined());
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
};

IRTypeLayout* maybeCreateArrayLayout(
    IRBuilder* builder,
    IRTypeLayout* elementTypeLayout,
    IRType* type)
{
    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        auto newElementTypeLayout =
            maybeCreateArrayLayout(builder, elementTypeLayout, arrayType->getElementType());
        IRIntegerValue elementCount = -1;
        if (auto count = arrayType->getElementCount())
            elementCount = getIntVal(count);
        IRArrayTypeLayout::Builder arrayTypeLayoutBuilder(builder, newElementTypeLayout);
        for (auto sizeAttr : newElementTypeLayout->getSizeAttrs())
        {
            arrayTypeLayoutBuilder.addResourceUsage(
                sizeAttr->getResourceKind(),
                elementCount == -1 ? LayoutSize::infinite() : sizeAttr->getSize() * elementCount);
        }
        for (auto alignmentAttr : newElementTypeLayout->getAlignmentAttrs())
        {
            arrayTypeLayoutBuilder.addAlignment(alignmentAttr);
        }
        return arrayTypeLayoutBuilder.build();
    }
    return elementTypeLayout;
}

void lowerCombinedTextureSamplers(
    IRModule* module,
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    LowerCombinedSamplerContext context;
    context.codeGenTarget = codeGenContext->getTargetFormat();

    bool hasCombinedSampler = false;

    // Lower combined texture sampler type into a struct type.
    for (auto globalInst : module->getGlobalInsts())
    {
        if (isCombinedTextureSamplerType(globalInst))
            hasCombinedSampler = true;
        auto globalParam = as<IRGlobalParam>(globalInst);
        if (!globalParam)
            continue;
        auto elementType = unwrapArray(globalParam->getFullType());
        auto textureType = isCombinedTextureSamplerType(elementType);
        if (!textureType)
            continue;
        auto layoutDecor = globalParam->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            continue;
        // Replace the original VarLayout with the new StructTypeVarLayout.
        auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
        if (!varLayout)
            continue;
        IRBuilder subBuilder(globalInst);
        subBuilder.setInsertBefore(globalInst);

        auto typeInfo = context.lowerCombinedTextureSamplerType(textureType);
        auto newTypeLayout =
            maybeCreateArrayLayout(&subBuilder, typeInfo.typeLayout, globalParam->getFullType());
        IRVarLayout::Builder newVarLayoutBuilder(&subBuilder, newTypeLayout);
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
        if (resOffsetAttr && !descriptorTableSlotOffsetAttr)
        {
            auto info =
                newVarLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::DescriptorTableSlot);
            info->offset = resOffsetAttr->getOffset();
            info->space = resOffsetAttr->getSpace();
            info->kind = LayoutResourceKind::DescriptorTableSlot;
        }
        auto newVarLayout = newVarLayoutBuilder.build();
        subBuilder.addLayoutDecoration(globalParam, newVarLayout);
        varLayout->removeAndDeallocate();
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
                            // If the handle is still a DescriptorHandle, we are on a target where
                            // native resource handles are already bindless (Metal, CPU). On these
                            // targets, a handle to a combined texture-sampler is a struct
                            // containing texture and sampler fields, so we insert the extract
                            // operations. A handle to any other resource is emitted as that
                            // resource, and the cast as its operand, so we keep that cast.
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
