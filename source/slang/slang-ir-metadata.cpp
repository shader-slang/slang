// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "../compiler-core/slang-artifact-associated-impl.h"

namespace Slang
{

// This file currently implements a pass that collects information about the shader parameters that
// are referenced in the IR. It's named 'metadata' in order to support other potential code
// analysis scenarios in the future.


// Inserts a single resource binding (which takes `count` slots, where 0 means unbounded) into the list of resource ranges.
static void _insertBinding(List<ShaderBindingRange>& ranges, LayoutResourceKind kind, UInt spaceIndex, UInt registerIndex, UInt count)
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

// Collects the metadata from the provided IR module, saves it in outMetadata.
void collectMetadata(const IRModule* irModule, ArtifactPostEmitMetadata& outMetadata)
{
    // Scan the instructions looking for global resource declarations
    for (const auto& inst : irModule->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param) continue;
        
        auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
        if (!layoutDecoration) continue;
        
        auto varLayout = as<IRVarLayout>(layoutDecoration->getLayout());
        if (!varLayout) continue;
        
        for(auto sizeAttr : varLayout->getTypeLayout()->getSizeAttrs())
        {
            auto kind = sizeAttr->getResourceKind();

            // Only track resource types that we can reliably track, such as textures.
            // Do not track individual uniforms, for example.
            if (!ShaderBindingRange::isUsageTracked(kind))
                continue;

            if (auto offsetAttr = varLayout->findOffsetAttr(kind))
            {
                // Get the binding information from this attribute and insert it into the list
                auto spaceIndex = offsetAttr->getSpace();
                auto registerIndex = offsetAttr->getOffset();
                auto size = sizeAttr->getSize();
                auto count = size.isFinite() ? size.getFiniteValue() : 0;
                _insertBinding(outMetadata.m_usedBindings, kind, spaceIndex, registerIndex, count);
            }
        }
    }
}

}
