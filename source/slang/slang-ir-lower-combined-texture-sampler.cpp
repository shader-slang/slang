#include "slang-ir-lower-combined-texture-sampler.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-type-layout.h"

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

IRTextureTypeBase* isCombinedTextureSamplerType(IRInst* typeInst)
{
    auto textureType = as<IRTextureTypeBase>(typeInst);
    if (!textureType)
        return nullptr;
    if (!textureType->isCombined())
        return nullptr;
    return textureType;
}

static IRVarLayout* rewriteNestedVarLayout(
    LowerCombinedSamplerContext& context,
    IRBuilder* builder,
    IRType* type,
    IRVarLayout* varLayout);

// Return `layout`, the layout of a value of `type`, with the layout of every combined
// texture-sampler inside `type` that this pass lowers replaced by a layout of its lowered
// `{texture, sampler}` struct. Only the layouts on the path to such a texture-sampler are
// rebuilt; when `type` contains none, `layout` itself is returned.
//
// Consider this example:
//
//     cbuffer C { Sampler2D a; Sampler2D b; }
//
// The field `b` of the constant buffer's element struct has the layout of a combined
// texture-sampler: one texture register and one sampler register, and no field layouts. After
// the type of `b` becomes the lowered struct, `legalizeResourceTypes` looks up the layouts of
// its `texture` and `sampler` fields to bind the two resources it splits out, so `b` needs a
// struct layout with those fields. The walk follows the same type and layout pairs that
// `getFieldLayout` and `declareVars` follow: arrays, structs and uniform parameter groups.
static IRTypeLayout* rewriteNestedTypeLayout(
    LowerCombinedSamplerContext& context,
    IRBuilder* builder,
    IRType* type,
    IRTypeLayout* layout)
{
    if (auto textureType = isCombinedTextureSamplerType(type))
    {
        auto info = context.mapTypeToLoweredInfo.tryGetValue(textureType);
        if (!info)
            return layout;

        // The fields keep their offsets relative to this position, and the struct keeps the
        // sizes of the combined layout, so the enclosing layouts keep their sizes and
        // `applyOffsetToTypeLayout` can still offset the fields.
        IRStructTypeLayout::Builder structLayoutBuilder(builder);
        structLayoutBuilder.addResourceUsageFrom(layout);
        for (auto fieldAttr : cast<IRStructTypeLayout>(info->typeLayout)->getFieldLayoutAttrs())
            structLayoutBuilder.addField(fieldAttr->getFieldKey(), fieldAttr->getLayout());
        return structLayoutBuilder.build();
    }

    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        auto arrayLayout = as<IRArrayTypeLayout>(layout);
        if (!arrayLayout)
            return layout;
        auto elementLayout = arrayLayout->getElementTypeLayout();
        auto newElementLayout =
            rewriteNestedTypeLayout(context, builder, arrayType->getElementType(), elementLayout);
        if (newElementLayout == elementLayout)
            return layout;
        IRArrayTypeLayout::Builder arrayLayoutBuilder(builder, newElementLayout);
        arrayLayoutBuilder.addResourceUsageFrom(arrayLayout);
        return arrayLayoutBuilder.build();
    }

    if (auto structType = as<IRStructType>(type))
    {
        auto structLayout = as<IRStructTypeLayout>(layout);
        if (!structLayout)
            return layout;
        bool changed = false;
        IRStructTypeLayout::Builder structLayoutBuilder(builder);
        structLayoutBuilder.addResourceUsageFrom(structLayout);
        for (auto fieldAttr : structLayout->getFieldLayoutAttrs())
        {
            auto fieldLayout = fieldAttr->getLayout();
            auto newFieldLayout = fieldLayout;
            // The layout can still list a field that an earlier pass moved out of the struct,
            // such as a resource that left the struct of global-scope parameters.
            if (auto field = findStructField(structType, as<IRStructKey>(fieldAttr->getFieldKey())))
                newFieldLayout =
                    rewriteNestedVarLayout(context, builder, field->getFieldType(), fieldLayout);
            changed |= newFieldLayout != fieldLayout;
            structLayoutBuilder.addField(fieldAttr->getFieldKey(), newFieldLayout);
        }
        return changed ? structLayoutBuilder.build() : layout;
    }

    if (auto groupType = as<IRUniformParameterGroupType>(type))
    {
        auto groupLayout = as<IRParameterGroupTypeLayout>(layout);
        if (!groupLayout)
            return layout;
        auto elementVarLayout = groupLayout->getElementVarLayout();
        auto newElementVarLayout =
            rewriteNestedVarLayout(context, builder, groupType->getElementType(), elementVarLayout);
        if (newElementVarLayout == elementVarLayout)
            return layout;
        IRParameterGroupTypeLayout::Builder groupLayoutBuilder(builder);
        groupLayoutBuilder.addResourceUsageFrom(groupLayout);
        groupLayoutBuilder.setContainerVarLayout(groupLayout->getContainerVarLayout());
        groupLayoutBuilder.setElementVarLayout(newElementVarLayout);
        // The offset element layout is derived from the element layout the same way the front
        // end derives it. For `ParameterBlock<Sampler2D>` this also moves the element's offsets
        // into the fields of the lowered struct, because the element is now a struct.
        groupLayoutBuilder.setOffsetElementTypeLayout(applyOffsetToTypeLayout(
            builder,
            newElementVarLayout->getTypeLayout(),
            newElementVarLayout));
        return groupLayoutBuilder.build();
    }

    return layout;
}

// Return `varLayout`, the layout of a variable of `type`, with its type layout rewritten by
// `rewriteNestedTypeLayout`. The offsets and every other property of `varLayout` are kept.
static IRVarLayout* rewriteNestedVarLayout(
    LowerCombinedSamplerContext& context,
    IRBuilder* builder,
    IRType* type,
    IRVarLayout* varLayout)
{
    auto typeLayout = varLayout->getTypeLayout();
    auto newTypeLayout = rewriteNestedTypeLayout(context, builder, type, typeLayout);
    if (newTypeLayout == typeLayout)
        return varLayout;
    IRVarLayout::Builder varLayoutBuilder(builder, newTypeLayout);
    varLayoutBuilder.cloneEverythingButOffsetsFrom(varLayout);
    for (auto offsetAttr : varLayout->getOffsetAttrs())
    {
        auto info = varLayoutBuilder.findOrAddResourceInfo(offsetAttr->getResourceKind());
        info->offset = offsetAttr->getOffset();
        info->space = offsetAttr->getSpace();
    }
    return varLayoutBuilder.build();
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
        // Layouts are deduplicated, so `varLayout` can also be the layout of a field nested in
        // another parameter: the first `Sampler2D` field of a constant buffer has the same
        // relative offsets as a global `Sampler2D` at t0/s0. The nested-layout loop below reads
        // that field layout, so only the decoration is removed here.
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

    // A combined texture-sampler nested inside a global parameter (a field of a constant buffer,
    // a parameter block or a struct) gets the layout of its lowered struct here. The loop above
    // already did this for a parameter that is itself a combined texture-sampler or an array of
    // them.
    for (auto globalInst : module->getGlobalInsts())
    {
        auto globalParam = as<IRGlobalParam>(globalInst);
        if (!globalParam || isCombinedTextureSamplerType(unwrapArray(globalParam->getFullType())))
            continue;
        auto layoutDecor = globalParam->findDecoration<IRLayoutDecoration>();
        if (!layoutDecor)
            continue;
        auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
        if (!varLayout)
            continue;
        builder.setInsertBefore(globalParam);
        auto newVarLayout =
            rewriteNestedVarLayout(context, &builder, globalParam->getFullType(), varLayout);
        if (newVarLayout == varLayout)
            continue;
        builder.addLayoutDecoration(globalParam, newVarLayout);
        layoutDecor->removeAndDeallocate();
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
