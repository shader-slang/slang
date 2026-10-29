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

// Return true if a value of `type` holds a combined texture-sampler whose type this pass will
// replace (one in `mapTypeToLoweredInfo`), directly or inside arrays, struct fields or uniform
// parameter groups. These are the containers whose layouts `legalizeResourceTypes` reads to bind
// the resources of a global parameter; a structured-buffer element is never split into separate
// resources, so its layout is not followed. `getTypeLayoutWithLoweredCombinedTextureSamplers`
// handles exactly these four cases.
static bool containsCombinedTextureSamplerToLower(
    LowerCombinedSamplerContext& context,
    IRType* type)
{
    if (auto textureType = isCombinedTextureSamplerType(type))
        return context.mapTypeToLoweredInfo.containsKey(textureType);
    if (auto arrayType = as<IRArrayTypeBase>(type))
        return containsCombinedTextureSamplerToLower(context, arrayType->getElementType());
    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (containsCombinedTextureSamplerToLower(context, field->getFieldType()))
                return true;
        }
        return false;
    }
    if (auto groupType = as<IRUniformParameterGroupType>(type))
        return containsCombinedTextureSamplerToLower(context, groupType->getElementType());
    return false;
}

static IRVarLayout* getVarLayoutWithLoweredCombinedTextureSamplers(
    LowerCombinedSamplerContext& context,
    IRBuilder* builder,
    IRType* type,
    IRVarLayout* varLayout);

// Return `layout`, the layout of a value of `type`, with the layout of every combined
// texture-sampler inside `type` that this pass lowers replaced by the layout of its lowered
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
// struct layout with those fields.
static IRTypeLayout* getTypeLayoutWithLoweredCombinedTextureSamplers(
    LowerCombinedSamplerContext& context,
    IRBuilder* builder,
    IRType* type,
    IRTypeLayout* layout)
{
    if (!containsCombinedTextureSamplerToLower(context, type))
        return layout;

    // A combined texture-sampler gets the same layout as a lowered top-level one: field layouts
    // for the texture and the sampler with relative offsets, and no sizes of its own. When
    // `legalizeResourceTypes` splits the struct, `buildSimpleVarLayout` binds each split-out
    // resource from that field's own type layout plus the offsets of the enclosing var layouts,
    // so nothing reads the struct's sizes. On CPU-like targets and inside a Metal argument buffer
    // the struct is not split, and nothing reads these layouts at all.
    if (auto textureType = isCombinedTextureSamplerType(type))
        return context.mapTypeToLoweredInfo.getValue(textureType).typeLayout;

    // The cases below are the containers that `containsCombinedTextureSamplerToLower` follows.
    // Front-end layouts follow the shape of the type, so such a container always has the matching
    // layout kind; anything else would lose the bindings of the resources the struct is split into.
    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        auto arrayLayout = as<IRArrayTypeLayout>(layout);
        SLANG_RELEASE_ASSERT(arrayLayout);
        auto newElementLayout = getTypeLayoutWithLoweredCombinedTextureSamplers(
            context,
            builder,
            arrayType->getElementType(),
            arrayLayout->getElementTypeLayout());
        IRArrayTypeLayout::Builder arrayLayoutBuilder(builder, newElementLayout);
        arrayLayoutBuilder.addResourceUsageFrom(arrayLayout);
        return arrayLayoutBuilder.build();
    }

    if (auto structType = as<IRStructType>(type))
    {
        auto structLayout = as<IRStructTypeLayout>(layout);
        SLANG_RELEASE_ASSERT(structLayout);
        IRStructTypeLayout::Builder structLayoutBuilder(builder);
        structLayoutBuilder.addResourceUsageFrom(structLayout);
        for (auto fieldAttr : structLayout->getFieldLayoutAttrs())
        {
            auto fieldKey = cast<IRStructKey>(fieldAttr->getFieldKey());
            auto fieldLayout = fieldAttr->getLayout();
            // Only the struct of global-scope parameters has layout entries without a field:
            // `collectGlobalUniformParameters` leaves a parameter with no uniform data at global
            // scope but keeps its entry, under a key the struct has no field for. The driver loop
            // in `lowerCombinedTextureSamplers` visits such a parameter as a global parameter of
            // its own.
            if (auto field = findStructField(structType, fieldKey))
                fieldLayout = getVarLayoutWithLoweredCombinedTextureSamplers(
                    context,
                    builder,
                    field->getFieldType(),
                    fieldLayout);
            structLayoutBuilder.addField(fieldKey, fieldLayout);
        }
        return structLayoutBuilder.build();
    }

    // The last container that `containsCombinedTextureSamplerToLower` follows.
    auto groupType = as<IRUniformParameterGroupType>(type);
    SLANG_RELEASE_ASSERT(groupType);
    auto groupLayout = as<IRParameterGroupTypeLayout>(layout);
    SLANG_RELEASE_ASSERT(groupLayout);
    auto newElementVarLayout = getVarLayoutWithLoweredCombinedTextureSamplers(
        context,
        builder,
        groupType->getElementType(),
        groupLayout->getElementVarLayout());
    IRParameterGroupTypeLayout::Builder groupLayoutBuilder(builder);
    groupLayoutBuilder.addResourceUsageFrom(groupLayout);
    groupLayoutBuilder.setContainerVarLayout(groupLayout->getContainerVarLayout());
    groupLayoutBuilder.setElementVarLayout(newElementVarLayout);
    // `getFieldLayout` reads a field of a parameter group's element through the offset element
    // layout, so it is derived from the new element layout the same way the front end derives it:
    // the field offsets of a struct element include the element's own offsets. An element that is
    // itself a combined texture-sampler (`ParameterBlock<Sampler2D>`) has a layout with no sizes,
    // so `applyOffsetToTypeLayout` returns it unchanged; that is correct because such an element
    // starts at offset 0 of its group, which holds no uniform data that could come first.
    groupLayoutBuilder.setOffsetElementTypeLayout(applyOffsetToTypeLayout(
        builder,
        newElementVarLayout->getTypeLayout(),
        newElementVarLayout));
    return groupLayoutBuilder.build();
}

// Return `varLayout`, the layout of a variable of `type`, with its type layout rewritten by
// `getTypeLayoutWithLoweredCombinedTextureSamplers`. The offsets and every other property of
// `varLayout` are kept.
static IRVarLayout* getVarLayoutWithLoweredCombinedTextureSamplers(
    LowerCombinedSamplerContext& context,
    IRBuilder* builder,
    IRType* type,
    IRVarLayout* varLayout)
{
    auto typeLayout = varLayout->getTypeLayout();
    auto newTypeLayout =
        getTypeLayoutWithLoweredCombinedTextureSamplers(context, builder, type, typeLayout);
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

    // A combined texture-sampler nested inside a global parameter gets the layout of its lowered
    // struct here: a field of a constant buffer, a parameter block or a struct, including the
    // `globalParams` and `entryPointParams` structs that hold `uniform` globals and entry-point
    // parameters. The loops above added every combined type that is replaced below to
    // `mapTypeToLoweredInfo`, and the walk rewrites exactly those. A combined type that no loop
    // added keeps both its type and its layout; for example, a WGSL `Sampler2D` used only by
    // `Load` is never lowered, because its `$q` string does not extract the texture.
    // The first loop handled a global parameter that is itself a combined texture-sampler or an
    // array of them; that loop also gives an explicitly bound texture a descriptor-table offset,
    // which a nested field cannot need because it has no binding of its own.
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
        auto newVarLayout = getVarLayoutWithLoweredCombinedTextureSamplers(
            context,
            &builder,
            globalParam->getFullType(),
            varLayout);
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
