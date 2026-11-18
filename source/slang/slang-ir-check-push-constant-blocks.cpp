#include "slang-ir-check-push-constant-blocks.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

bool isPushConstantBlockByLayout(IRInst* globalParam)
{
    auto varLayout = findVarLayout(globalParam);
    if (!varLayout)
        return false;
    auto typeLayout = varLayout->getTypeLayout()->unwrapArray();
    if (auto parameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(typeLayout))
        varLayout = parameterGroupTypeLayout->getContainerVarLayout();
    return varLayout->usesResourceKind(LayoutResourceKind::PushConstantBuffer);
}

/// Return the struct type that holds the `uniform` parameters in the push-constant block that
/// `collectEntryPointUniformParams` creates for an entry point. On SPIR-V the block is a pointer
/// to the struct after legalization; on GLSL it is still a `ConstantBuffer` of the struct.
static IRStructType* getEntryPointUniformsStruct(IRInst* block)
{
    IRInst* type = block->getDataType();
    if (auto ptrType = as<IRPtrTypeBase>(type))
        type = ptrType->getValueType();
    if (auto groupType = as<IRParameterGroupType>(type))
        type = groupType->getElementType();
    auto structType = as<IRStructType>(type);
    SLANG_RELEASE_ASSERT(structType);
    return structType;
}

/// Describe `block` the way the user wrote it, for example "push-constant block `push`" for a
/// global `[[vk::push_constant]]` parameter, or "the push-constant block that holds the `uniform`
/// parameters `k`, `scale` of `main`" for the block that `collectEntryPointUniformParams` creates
/// for `main`.
///
/// The compiler-created block has no declaration of its own, so `collectEntryPointUniformParams`
/// gives it the source location of the entry point, and the passes that later rebuild a global
/// parameter (`moveEntryPointUniformParamsToGlobalScope`, and `legalizeGlobalParam` through
/// `declareSimpleVar`) copy that location to the replacement. A block located at `entryPoint` is
/// therefore that entry point's own block. We use the location because
/// `IREntryPointParamDecoration`, which records the same fact, is removed by
/// `stripLegalizationOnlyInstructions` before either target runs this check.
static String describePushConstantBlock(IRInst* block, IRFunc* entryPoint)
{
    StringBuilder sb;
    if (block->sourceLoc.isValid() && block->sourceLoc == entryPoint->sourceLoc)
    {
        sb << "the push-constant block that holds the `uniform` parameters ";
        auto structType = getEntryPointUniformsStruct(block);
        const char* separator = "";
        for (auto field : structType->getFields())
        {
            sb << separator << "`";
            printDiagnosticArg(sb, field->getKey());
            sb << "`";
            separator = ", ";
        }
        sb << " of `";
        printDiagnosticArg(sb, entryPoint);
        sb << "`";
    }
    else
    {
        sb << "push-constant block `";
        printDiagnosticArg(sb, block);
        sb << "`";
    }
    return sb.produceString();
}

void checkForMultiplePushConstantBlocksPerEntryPoint(
    IRModule* module,
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    bool (*isPushConstantBlock)(IRInst* globalParam),
    DiagnosticSink* sink)
{
    List<IRFunc*> entryPoints;
    List<IRInst*> pushConstantBlocks;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst); func && func->findDecoration<IREntryPointDecoration>())
            entryPoints.add(func);
        else if (as<IRGlobalParam>(inst) && isPushConstantBlock(inst))
            pushConstantBlocks.add(inst);
    }
    if (pushConstantBlocks.getCount() < 2)
        return;

    for (auto entryPoint : entryPoints)
    {
        List<IRInst*> usedBlocks;
        for (auto block : pushConstantBlocks)
        {
            auto blockEntryPoints = getReferencingEntryPoints(referencingEntryPoints, block);
            if (blockEntryPoints && blockEntryPoints->contains(entryPoint))
                usedBlocks.add(block);
        }
        if (usedBlocks.getCount() < 2)
            continue;

        Diagnostics::MultiplePushConstantBlocksInEntryPoint diagnostic;
        diagnostic.entryPoint = entryPoint;
        for (auto block : usedBlocks)
        {
            Diagnostics::MultiplePushConstantBlocksInEntryPoint::Block note;
            note.blockDescription = describePushConstantBlock(block, entryPoint);
            note.blockLocation = block->sourceLoc;
            diagnostic.blocks.add(note);
        }
        sink->diagnose(diagnostic);
    }
}

} // namespace Slang
