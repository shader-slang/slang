#include "slang-ir-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

// Ordinary type annotations and source-operation decorations retain the information needed by
// structural ray-tracing lowering through module serialization, linking, and specialization.

namespace Slang
{

bool hasStructuralRayTracingStageInput(IRFunc* func)
{
    for (auto paramType : func->getDataType()->getParamTypes())
    {
        auto inputType = getResolvedInstForDecorations(unwrapAttributedType(paramType));
        auto builtin = inputType->findDecoration<IRKnownBuiltinDecoration>();
        if (builtin && getStructuralRayTracingStageInputKind(builtin->getName()) != Stage::Unknown)
            return true;
    }
    return false;
}

void addStructuralRayTracingSourceOperation(
    IRBuilder& builder,
    IRFunc* func,
    StructuralRayTracingSourceOperationKind kind)
{
    SLANG_RELEASE_ASSERT(func && kind == StructuralRayTracingSourceOperationKind::CallShader);

    if (auto existing = func->findDecoration<IRStructuralRayTracingSourceOperationDecoration>())
    {
        auto existingKind = as<IRIntLit>(existing->getOperationKind());
        SLANG_RELEASE_ASSERT(existingKind && existingKind->getValue() == IRIntegerValue(kind));
        return;
    }

    builder.addDecoration(
        func,
        kIROp_StructuralRayTracingSourceOperationDecoration,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(kind)));
}

} // namespace Slang
