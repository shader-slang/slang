#include "slang-ir-lower-dynamic-resource-heap.h"

#include "compiler-core/slang-artifact-associated-impl.h"
#include "slang-capability.h"
#include "slang-ir-util.h"
#include "slang-rich-diagnostics.h"
#include "slang-target-program.h"
#include "slang-type-layout.h"

namespace Slang
{

/// Get the bindless descriptor set/space index from the program layout.
/// This index was allocated during layout generation (before DCE),
/// ensuring consistency with reflection data.
bool tryGetBindlessSpaceIndex(TargetProgram* targetProgram, UInt& outBindlessSpaceIndex)
{
    auto programLayout = targetProgram->getLayoutIfAvailable();
    if (!programLayout)
        return false;

    // Do not fall back to the requested option here; only layout knows the
    // bindless space that was actually reserved after conflict resolution.
    if (programLayout->bindlessSpaceIndex < 0)
        return false;

    outBindlessSpaceIndex = (UInt)programLayout->bindlessSpaceIndex;
    return true;
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
    List<IRInst*> workList;
    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->getOp() == kIROp_GetDynamicResourceHeap)
        {
            workList.add(globalInst);
        }
    }

    if (workList.getCount() == 0)
        return;

    // If there are GetDynamicResourceHeap instructions, verify that the target
    // supports descriptor_handle capability.
    {
        auto targetCaps = targetProgram->getTargetReq()->getTargetCaps();
        if (targetCaps.atLeastOneSetImpliedInOther(CapabilitySet(
                CapabilityName::descriptor_handle)) != CapabilitySet::ImpliesReturnFlags::Implied)
        {
            sink->diagnose(Diagnostics::TargetDoesNotSupportDescriptorHandle{});
            return;
        }
    }

    UInt bindlessSpaceIndex = 0;
    SLANG_RELEASE_ASSERT(tryGetBindlessSpaceIndex(targetProgram, bindlessSpaceIndex));

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
            bindlessSpaceIndex,
            bindingIndex);
        builder.addLayoutDecoration(param, varLayout);
        builder.addNameHintDecoration(param, toSlice("__slang_resource_heap"));
        inst->replaceUsesWith(param);
    }
}

} // namespace Slang
