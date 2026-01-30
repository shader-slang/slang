#include "slang-ir-lower-dynamic-resource-heap.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "slang-ir-util.h"

namespace Slang
{

Index findRegisterSpaceResourceInfo(IRVarLayout* layout);
UInt findUnusedSpaceIndex(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink)
{
    HashSet<int> usedSpaces;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto varLayout = findVarLayout(inst))
        {
            // Get container space (for ParameterBlocks etc.)
            auto containerSpace = findRegisterSpaceResourceInfo(varLayout);
            if (containerSpace >= 0)
                usedSpaces.add((int)containerSpace);

            // Get base offset for nested resources
            UInt spaceOffset = 0;
            if (auto spaceAttr =
                    varLayout->findOffsetAttr(LayoutResourceKind::SubElementRegisterSpace))
                spaceOffset = spaceAttr->getOffset();

            // Get direct binding spaces
            for (auto sizeAttr : varLayout->getTypeLayout()->getSizeAttrs())
            {
                if (!ShaderBindingRange::isUsageTracked(sizeAttr->getResourceKind()))
                    continue;
                if (auto offsetAttr = varLayout->findOffsetAttr(sizeAttr->getResourceKind()))
                    usedSpaces.add((int)(spaceOffset + offsetAttr->getSpace()));
            }
        }
    }

    // Find next unused space index.
    int requestedIndex =
        targetProgram->getOptionSet().getIntOption(CompilerOptionName::BindlessSpaceIndex);
    int availableIndex = requestedIndex;

    while (usedSpaces.contains(availableIndex))
    {
        availableIndex++;
    }

    if (availableIndex != requestedIndex &&
        targetProgram->getOptionSet().hasOption(CompilerOptionName::BindlessSpaceIndex))
    {
        sink->diagnose(
            SourceLoc(),
            Diagnostics::requestedBindlessSpaceIndexUnavailable,
            requestedIndex,
            availableIndex);
    }
    return availableIndex;
}

IRVarLayout* createResourceHeapVarLayoutWithSpaceAndBinding(
    IRBuilder& builder,
    IRInst* param,
    UInt spaceIndex,
    UInt bindingIndex)
{
    SLANG_UNUSED(param);
    IRTypeLayout::Builder typeLayoutBuilder(&builder);
    typeLayoutBuilder.addResourceUsage(
        LayoutResourceKind::DescriptorTableSlot,
        LayoutSize::infinite());
    auto typeLayout = typeLayoutBuilder.build();
    IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::RegisterSpace)->offset = spaceIndex;
    varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::DescriptorTableSlot)->offset =
        bindingIndex;
    return varLayoutBuilder.build();
}

void lowerDynamicResourceHeap(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    auto unusedSpaceIndex = findUnusedSpaceIndex(targetProgram, module, sink);
    List<IRInst*> workList;
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->getOp() == kIROp_GetDynamicResourceHeap)
        {
            workList.add(globalInst);
        }
    }
    for (auto inst : workList)
    {
        auto arrayType = as<IRArrayTypeBase>(inst->getDataType());
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        auto bindingIndex = (UInt)as<IRIntLit>(inst->getOperand(0))->getValue();

        auto param = builder.createGlobalParam(arrayType);
        auto varLayout = createResourceHeapVarLayoutWithSpaceAndBinding(
            builder,
            param,
            unusedSpaceIndex,
            bindingIndex);
        builder.addLayoutDecoration(param, varLayout);
        builder.addNameHintDecoration(param, toSlice("__slang_resource_heap"));
        inst->replaceUsesWith(param);
    }
}

} // namespace Slang
