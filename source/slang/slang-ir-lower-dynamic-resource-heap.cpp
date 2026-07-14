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
UInt getBindlessSpaceIndex(TargetProgram* targetProgram)
{
    SLANG_RELEASE_ASSERT(targetProgram);
    auto programLayout = targetProgram->getExistingLayout();
    SLANG_RELEASE_ASSERT(programLayout);

    // Do not fall back to the requested option here; only layout knows the
    // bindless space that was actually reserved after conflict resolution.
    SLANG_RELEASE_ASSERT(programLayout->bindlessSpaceIndex >= 0);

    return (UInt)programLayout->bindlessSpaceIndex;
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

    auto bindlessSpaceIndex = getBindlessSpaceIndex(targetProgram);

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

void lowerUntypedResourceHandleToUInt(IRModule* module)
{
    // Collect the casts and the type ops in a single walk before mutating, so replacing/removing
    // insts does not invalidate the traversal.
    List<IRInst*> castsToForward;
    List<IRInst*> typesToRewrite;

    List<IRInst*> workList;
    workList.add(module->getModuleInst());
    while (workList.getCount())
    {
        auto inst = workList.getLast();
        workList.removeLast();
        for (auto child : inst->getChildren())
            workList.add(child);

        switch (inst->getOp())
        {
        case kIROp_CastUIntToUntypedResourceHandle:
        case kIROp_CastUntypedResourceHandleToUInt:
        case kIROp_CastUIntToUntypedSamplerHandle:
        case kIROp_CastUntypedSamplerHandleToUInt:
            castsToForward.add(inst);
            break;
        case kIROp_UntypedResourceHandleType:
        case kIROp_UntypedSamplerHandleType:
            typesToRewrite.add(inst);
            break;
        default:
            break;
        }
    }

    // Forward the casts first: after the type rewrite below both operand and result are `uint`, so
    // each cast is an identity that just passes its heap index through.
    for (auto cast : castsToForward)
    {
        cast->replaceUsesWith(cast->getOperand(0));
        cast->removeAndDeallocate();
    }

    // Rewrite the untyped handle types to the canonical `uint`, so no untyped handle reaches emit.
    if (typesToRewrite.getCount())
    {
        IRBuilder builder(module);
        auto uintType = builder.getUIntType();
        for (auto type : typesToRewrite)
        {
            type->replaceUsesWith(uintType);
            type->removeAndDeallocate();
        }
    }
}

} // namespace Slang
