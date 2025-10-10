// slang-ir-specialize-arrays.cpp
#include "slang-ir-specialize-arrays.h"

#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir.h"

namespace Slang
{

struct ArrayParameterSpecializationCondition : FunctionCallSpecializeCondition
{
    // This pass is intended to specialize functions
    // with unsized array parameter called with a sized-array argument.
    //

    bool doesParamWantSpecialization(IRParam* param, IRInst* arg, IRCall* callInst)
    {
        SLANG_UNUSED(param);
        SLANG_UNUSED(arg);
        SLANG_UNUSED(callInst);
        return false;
    }

    bool doesParamTypeWantSpecialization(IRParam* param, IRInst* arg)
    {
        auto paramType = param->getDataType();
        auto argType = arg->getDataType();
        if (auto outTypeBase = as<IROutParamTypeBase>(paramType))
        {
            paramType = outTypeBase->getValueType();
            IRBuilder builder(paramType);
            argType = tryGetPointedToType(&builder, argType);
            SLANG_ASSERT(argType);
        }
        else if (auto refType = as<IRRefParamType>(paramType))
        {
            paramType = refType->getValueType();
            IRBuilder builder(paramType);
            argType = tryGetPointedToType(&builder, argType);
            SLANG_ASSERT(argType);
        }
        else if (auto constRefType = as<IRBorrowInParamType>(paramType))
        {
            paramType = constRefType->getValueType();
            IRBuilder builder(paramType);
            argType = tryGetPointedToType(&builder, argType);
            SLANG_ASSERT(argType);
        }
        auto arrayType = as<IRUnsizedArrayType>(paramType);
        if (!arrayType)
            return false;
        auto argArrayType = as<IRArrayType>(argType);
        if (!argArrayType)
            return false;
        if (as<IRIntLit>(argArrayType->getElementCount()))
        {
            return true;
        }
        return false;
    }

    CodeGenContext* codeGenContext = nullptr;
};

void specializeArrayParameters(CodeGenContext* codeGenContext, IRModule* module)
{
    ArrayParameterSpecializationCondition condition;
    condition.codeGenContext = codeGenContext;
    specializeFunctionCalls(codeGenContext, module, &condition);
}

} // namespace Slang
