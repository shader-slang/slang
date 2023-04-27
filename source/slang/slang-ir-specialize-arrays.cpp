// slang-ir-specialize-arrays.cpp
#include "slang-ir-specialize-arrays.h"

#include "slang-ir-specialize-function-call.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct ArrayParameterSpecializationCondition : FunctionCallSpecializeCondition
{
    // This pass is intended to specialize functions
    // with struct parameters that has array fields
    // to avoid performance problems for GLSL targets.

    // Returns true if `type` is an `IRStructType` with array-typed fields.
    bool isStructTypeWithArray(IRType* type)
    {
        if (auto structType = as<IRStructType>(type))
        {
            for (auto field : structType->getFields())
            {
                if (const auto arrayType = as<IRArrayType>(field->getFieldType()))
                {
                    return true;
                }
                if (auto subStructType = as<IRStructType>(field->getFieldType()))
                {
                    if (isStructTypeWithArray(subStructType))
                        return true;
                }
            }
        }
        return false;
    }

    bool doesParamWantSpecialization(IRParam* param, IRInst* arg)
    {
        SLANG_UNUSED(arg);
        return isStructTypeWithArray(param->getDataType());
    }
};

void specializeArrayParameters(
    CodeGenContext* codeGenContext,
    IRModule*       module)
{
    ArrayParameterSpecializationCondition condition;
    specializeFunctionCalls(codeGenContext, module, &condition);
}

} // namesapce Slang
