// slang-ir-uniform-usage.cpp
#include "slang-ir-uniform-usage.h"

#include "slang-ir.h"

namespace Slang
{

static UInt _getUniformSize(IRTypeLayout* typeLayout)
{
    if (!typeLayout)
        return 0;
    auto sizeAttr = typeLayout->findSizeAttr(LayoutResourceKind::Uniform);
    if (!sizeAttr)
        return 0;
    auto size = sizeAttr->getSize();
    if (!size.isFinite())
        return 0;
    return UInt(size.getFiniteValue().getValidValue());
}

static UInt _getUniformOffset(IRVarLayout* varLayout)
{
    if (!varLayout)
        return 0;
    auto offsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::Uniform);
    return offsetAttr ? offsetAttr->getOffset() : 0;
}

static IRVarLayout* _findFieldVarLayout(IRStructTypeLayout* structLayout, IRInst* fieldKey)
{
    for (auto attr : structLayout->getFieldLayoutAttrs())
    {
        if (attr->getFieldKey() == fieldKey)
            return attr->getLayout();
    }
    return nullptr;
}

static void _recordRange(List<UniformUsageRange>& ranges, UInt offset, UInt size)
{
    if (size == 0)
        return;
    UniformUsageRange r;
    r.byteOffset = offset;
    r.byteSize = size;
    ranges.add(r);
}

static void _walkUses(
    IRInst* inst,
    IRTypeLayout* currentTypeLayout,
    UInt currentOffset,
    List<UniformUsageRange>& outRanges)
{
    UInt currentSize = _getUniformSize(currentTypeLayout);
    if (currentSize == 0)
        return;

    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        IRInst* user = use->getUser();
        switch (user->getOp())
        {
        case kIROp_FieldAddress:
        case kIROp_FieldExtract:
            {
                auto structLayout = as<IRStructTypeLayout>(currentTypeLayout);
                if (!structLayout)
                {
                    _recordRange(outRanges, currentOffset, currentSize);
                    break;
                }
                auto fieldVar = _findFieldVarLayout(structLayout, user->getOperand(1));
                if (!fieldVar)
                {
                    _recordRange(outRanges, currentOffset, currentSize);
                    break;
                }
                _walkUses(
                    user,
                    fieldVar->getTypeLayout(),
                    currentOffset + _getUniformOffset(fieldVar),
                    outRanges);
            }
            break;

        case kIROp_GetElementPtr:
        case kIROp_GetElement:
            {
                auto arrayLayout = as<IRArrayTypeLayout>(currentTypeLayout);
                if (!arrayLayout)
                {
                    _recordRange(outRanges, currentOffset, currentSize);
                    break;
                }
                auto elemLayout = arrayLayout->getElementTypeLayout();
                auto elemSize = _getUniformSize(elemLayout);
                auto indexLit = as<IRIntLit>(user->getOperand(1));
                if (!indexLit || elemSize == 0)
                {
                    _recordRange(outRanges, currentOffset, currentSize);
                    break;
                }
                _walkUses(
                    user,
                    elemLayout,
                    currentOffset + UInt(getIntVal(indexLit)) * elemSize,
                    outRanges);
            }
            break;

        case kIROp_Load:
            _walkUses(user, currentTypeLayout, currentOffset, outRanges);
            break;

        default:
            _recordRange(outRanges, currentOffset, currentSize);
            break;
        }
    }
}

void collectUniformUsage(
    const IRModule* module,
    Dictionary<IRGlobalParam*, List<UniformUsageRange>>& outUsage)
{
    for (auto inst : module->getGlobalInsts())
    {
        auto param = as<IRGlobalParam>(inst);
        if (!param)
            continue;

        auto layoutDeco = param->findDecoration<IRLayoutDecoration>();
        if (!layoutDeco)
            continue;

        auto varLayout = as<IRVarLayout>(layoutDeco->getLayout());
        if (!varLayout)
            continue;

        IRTypeLayout* typeLayout = varLayout->getTypeLayout();
        UInt baseOffset = _getUniformOffset(varLayout);

        // Constant buffers and parameter blocks wrap an inner element layout;
        // the uniform bytes live there, not on the outer parameter group.
        if (auto groupLayout = as<IRParameterGroupTypeLayout>(typeLayout))
        {
            auto elementVar = groupLayout->getElementVarLayout();
            typeLayout = elementVar->getTypeLayout();
            baseOffset += _getUniformOffset(elementVar);
        }

        if (_getUniformSize(typeLayout) == 0)
            continue;

        List<UniformUsageRange>& ranges = outUsage.getOrAddValue(param, List<UniformUsageRange>());
        _walkUses(param, typeLayout, baseOffset, ranges);
    }
}

} // namespace Slang
