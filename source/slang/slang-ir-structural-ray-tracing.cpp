#include "slang-ir-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

// Ordinary type annotations identify structural stage inputs through module serialization,
// linking, and specialization.

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

} // namespace Slang
