#include "slang-ir-synthesize-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

static void _collectStageInputOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectStageInputOperations(child, operations);
        if (as<IRStructuralRayTracingStageInputOperation>(child))
            operations.add(child);
    }
}

void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module)
{
    List<IRInst*> operations;
    _collectStageInputOperations(module->getModuleInst(), operations);

    IRBuilder builder(module);
    for (auto operation : operations)
    {
        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
        builder.setInsertBefore(operation);

        List<IRInst*> arguments;
        for (UInt i = 1; i < operation->getOperandCount(); ++i)
            arguments.add(operation->getOperand(i));

        auto call = builder.emitCallInst(
            operation->getDataType(),
            stageInputOperation->getFallback(),
            arguments.getCount(),
            arguments.getBuffer());
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }
}

} // namespace Slang
