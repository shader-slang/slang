// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

static void _insertBinding(List<ShaderBindingRange>& ranges, LayoutResourceKind kind, UInt spaceIndex, UInt registerIndex, UInt count)
{
    ShaderBindingRange newRange;
    newRange.category = kind;
    newRange.spaceIndex = spaceIndex;
    newRange.registerIndex = registerIndex;
    newRange.registerCount = count;

    for (auto& range : ranges)
    {
        if (range.adjacentTo(newRange))
        {
            range.mergeWith(newRange);
            return;
        }
    }

    ranges.add(newRange);
}

void collectMetadata(const IRModule* irModule, PostEmitMetadata& outMetadata)
{
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

            if (!ShaderBindingRange::isUsageTracked(kind))
                continue;

            if (auto offsetAttr = varLayout->findOffsetAttr(kind))
            {
                auto spaceIndex = offsetAttr->getSpace();
                auto registerIndex = offsetAttr->getOffset();
                auto size = sizeAttr->getSize();
                auto count = size.isFinite() ? size.getFiniteValue() : 0;
                _insertBinding(outMetadata.usedBindings, kind, spaceIndex, registerIndex, count);
            }
        }
    }
}


}
