// slang-ir-metadata.cpp
#include "slang-ir-metadata.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

static void _insertBinding(List<ShaderBindingRange>& ranges, LayoutResourceKind kind, int spaceIndex, int registerIndex, int count)
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
        if (auto param = as<IRGlobalParam>(inst))
        {
            if (auto varLayout = as<IRVarLayout>(param->findDecoration<IRLayoutDecoration>()))
            {
                for(auto sizeAttr : varLayout->getTypeLayout()->getSizeAttrs())
                {
                    auto kind = sizeAttr->getResourceKind();
                    if (auto offsetAttr = varLayout->findOffsetAttr(kind))
                    {
                        auto spaceIndex = offsetAttr->getSpace();
                        auto registerIndex = offsetAttr->getOffset();
                        auto size = sizeAttr->getSize();
                        auto count = size.isFinite() ? size.getFiniteValue() : 0;
                        _insertBinding(outMetadata.usedBindings, kind, spaceIndex, registerIndex, (int)count);
                    }
                }
            }
        }
    }
}


}
