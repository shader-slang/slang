#include "slang-ir-check-shader-parameter-type.h"

#include "slang-ir-util.h"

namespace Slang
{
void checkForInvalidShaderParameterTypeForMetal(IRModule* module, DiagnosticSink* sink)
{
    HashSet<IRInst*> workListSet;
    List<IRInst*> workList;
    for (auto inst : module->getGlobalInsts())
    {
        if (inst->getOp() == kIROp_ParameterBlockType)
        {
            auto type = inst->getOperand(0);
            if (workListSet.add(type))
                workList.add(type);
            // Diagnose an error on `ParameterBlock<ConstantBuffer<T>>`.
            if (type->getOp() == kIROp_ConstantBufferType)
            {
                bool foundUseSite = false;
                for (auto use = inst->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (user->sourceLoc.isValid())
                    {
                        sink->diagnose(
                            user,
                            Diagnostics::constantBufferInParameterBlockNotAllowedOnMetal);
                        foundUseSite = true;
                        break;
                    }
                }
                if (!foundUseSite)
                    sink->diagnose(
                        inst,
                        Diagnostics::constantBufferInParameterBlockNotAllowedOnMetal);
            }
        }
    }
    // Diagnose an error any any struct fields whose type is `ConstantBuffer<T>` if the
    // struct is used inside a `ParameterBlock`.
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto type = workList[i];
        if (auto structType = as<IRStructType>(type))
        {
            for (auto field : structType->getFields())
            {
                auto fieldType = field->getFieldType();
                if (fieldType->getOp() == kIROp_ConstantBufferType)
                {
                    sink->diagnose(
                        field->getKey(),
                        Diagnostics::constantBufferInParameterBlockNotAllowedOnMetal);
                }
                if (workListSet.add(fieldType))
                    workList.add(fieldType);
            }
        }
    }
}

void checkForInvalidShaderParameterType(
    TargetRequest* target,
    IRModule* module,
    DiagnosticSink* sink)
{
    if (isMetalTarget(target))
        checkForInvalidShaderParameterTypeForMetal(module, sink);
}
} // namespace Slang