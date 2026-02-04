// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

// This file currently implements a pass that collects information about the shader parameters that
// are referenced in the IR. It's named 'metadata' in order to support other potential code
// analysis scenarios in the future.


// Inserts a single resource binding (which takes `count` slots, where 0 means unbounded) into the
// list of resource ranges.
static void _insertBinding(
    List<ShaderBindingRange>& ranges,
    LayoutResourceKind kind,
    UInt spaceIndex,
    UInt registerIndex,
    UInt count)
{
    // Construct a new range from the provided resource.
    ShaderBindingRange newRange;
    newRange.category = kind;
    newRange.spaceIndex = spaceIndex;
    newRange.registerIndex = registerIndex;
    newRange.registerCount = count;

    // See if the new range is adjacent to any of the existing ranges, merge with that.
    for (auto& range : ranges)
    {
        if (range.adjacentTo(newRange))
        {
            range.mergeWith(newRange);
            return;
        }
    }

    // No adjacent ranges found - create a new one.
    ranges.add(newRange);
}

// Forward declaration for recursive calls.
static void collectMetadataFromTypeLayout(
    IRTypeLayout* typeLayout,
    UInt spaceOffset,
    ArtifactPostEmitMetadata& outMetadata);

// Collect metadata from an IRVarLayout, which may represent a parameter or a struct field.
static void collectMetadataFromVarLayout(
    IRVarLayout* varLayout,
    UInt parentSpaceOffset,
    ArtifactPostEmitMetadata& outMetadata)
{
    if (!varLayout)
        return;

    UInt spaceOffset = parentSpaceOffset;
    if (auto spaceAttr = varLayout->findOffsetAttr(LayoutResourceKind::RegisterSpace))
    {
        spaceOffset += spaceAttr->getOffset();
    }
    auto typeLayout = varLayout->getTypeLayout();
    if (!typeLayout)
        return;

    for (auto sizeAttr : typeLayout->getSizeAttrs())
    {
        auto kind = sizeAttr->getResourceKind();

        // Only track resource types that we can reliably track, such as textures.
        // Do not track individual uniforms, for example.
        if (!ShaderBindingRange::isUsageTracked(kind))
            continue;

        if (auto offsetAttr = varLayout->findOffsetAttr(kind))
        {
            // Get the binding information from this attribute and insert it into the list
            auto spaceIndex = spaceOffset + offsetAttr->getSpace();
            auto registerIndex = offsetAttr->getOffset();
            auto size = sizeAttr->getSize();
            auto count = size.getFiniteValueOr(0);
            _insertBinding(outMetadata.m_usedBindings, kind, spaceIndex, registerIndex, count);
        }
    }

    // Recursively process the type layout to handle nested struct fields.
    // This is needed for targets like WGSL/Metal where varying inputs are packed
    // into a struct parameter, and the field layouts contain the binding information.
    collectMetadataFromTypeLayout(typeLayout, spaceOffset, outMetadata);
}

// Collect metadata from an IRTypeLayout, processing struct fields recursively.
static void collectMetadataFromTypeLayout(
    IRTypeLayout* typeLayout,
    UInt spaceOffset,
    ArtifactPostEmitMetadata& outMetadata)
{
    if (!typeLayout)
        return;

    // Handle struct type layouts by processing each field's layout.
    if (auto structTypeLayout = as<IRStructTypeLayout>(typeLayout))
    {
        for (auto fieldLayoutAttr : structTypeLayout->getFieldLayoutAttrs())
        {
            auto fieldLayout = fieldLayoutAttr->getLayout();
            collectMetadataFromVarLayout(fieldLayout, spaceOffset, outMetadata);
        }
    }
}

void collectMetadataFromInst(IRInst* param, ArtifactPostEmitMetadata& outMetadata)
{
    auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
    if (!layoutDecoration)
        return;

    auto varLayout = as<IRVarLayout>(layoutDecoration->getLayout());
    if (!varLayout)
        return;

    // Collect metadata from the parameter's layout (handles both direct bindings
    // and recursively processes struct field layouts for targets like WGSL/Metal).
    collectMetadataFromVarLayout(varLayout, 0, outMetadata);

    // If the global parameter is a parameter block, make sure to collect bindings for its
    // default constant buffer, if there is one.
    // The default constant buffer binding will be represented in the container var layout.
    //
    auto paramGroupTypeLayout = as<IRParameterGroupTypeLayout>(varLayout->getTypeLayout());
    if (!paramGroupTypeLayout)
        return;
    auto containerVarLayout = paramGroupTypeLayout->getContainerVarLayout();
    if (!containerVarLayout)
        return;
    auto containerSpaceOffset =
        varLayout->findOffsetAttr(LayoutResourceKind::SubElementRegisterSpace);
    if (!containerSpaceOffset)
        return;
    UInt spaceOffset = containerSpaceOffset->getOffset();
    for (auto sizeAttr : containerVarLayout->getTypeLayout()->getSizeAttrs())
    {
        auto kind = sizeAttr->getResourceKind();

        if (!ShaderBindingRange::isUsageTracked(kind))
            continue;

        if (auto offsetAttr = containerVarLayout->findOffsetAttr(kind))
        {
            auto spaceIndex = spaceOffset + offsetAttr->getSpace();
            auto registerIndex = offsetAttr->getOffset();
            auto size = sizeAttr->getSize();
            auto count = size.getFiniteValueOr(0);
            _insertBinding(outMetadata.m_usedBindings, kind, spaceIndex, registerIndex, count);
        }
    }
}

// Collects the metadata from the provided IR module, saves it in outMetadata.
void collectMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata)
{
    // Scan the instructions looking for global resource declarations
    // and exported functions.
    for (const auto& inst : irModule->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (func->findDecoration<IRDownstreamModuleExportDecoration>())
            {
                auto name = func->findDecoration<IRExportDecoration>()->getMangledName();
                outMetadata.m_exportedFunctionMangledNames.add(name);
            }

            // Collect metadata from entrypoint params.
            for (auto param : func->getParams())
            {
                collectMetadataFromInst(param, outMetadata);
            }
        }

        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;
        collectMetadataFromInst(param, outMetadata);
    }
}

} // namespace Slang
