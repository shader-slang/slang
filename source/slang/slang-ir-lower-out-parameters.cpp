#include "slang-ir-lower-out-parameters.h"

#include "slang-ir-clone.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink*)
{
    IRBuilder builder(func->getModule());
    IRCloneEnv cloneEnv;

    // Collect types for the new function
    List<IRType*> returnTypes;
    List<IRType*> paramTypes;

    struct VarInfo
    {
        IRVar* var;
        IRParam* origParam;
        bool isInOut;
    };
    List<VarInfo> outVars;

    // If original function returns non-void, add it to tuple types
    if (!as<IRVoidType>(func->getResultType()))
        returnTypes.add(func->getResultType());

    // Process parameters
    for (auto param : func->getParams())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            returnTypes.add(outType->getValueType());

            if (outType->getOp() == kIROp_InOutType)
            {
                paramTypes.add(outType->getValueType());
            }

            outVars.add(VarInfo{nullptr, param, outType->getOp() == kIROp_InOutType});
        }
        else
        {
            paramTypes.add(param->getDataType());
        }
    }

    // Create new function
    auto newFunc = builder.createFunc();

    // Copy all decorations except name hint
    for (auto decor : func->getDecorations())
    {
        cloneDecoration(&cloneEnv, decor, newFunc, builder.getModule());
    }

    // Determine result type
    IRType* resultType;
    if (returnTypes.getCount() > 1)
    {
        resultType = builder.getTupleType(returnTypes);
    }
    else if (returnTypes.getCount() == 1)
    {
        resultType = returnTypes[0];
    }
    else
    {
        resultType = builder.getVoidType();
    }

    auto funcType = builder.getFuncType(paramTypes, resultType);
    newFunc->setFullType(funcType);

    auto firstBlock = builder.createBlock();
    newFunc->addBlock(firstBlock);
    builder.setInsertInto(firstBlock);

    // Create parameters and track them
    List<IRParam*> newParams;
    for (auto param : func->getParams())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            if (outType->getOp() == kIROp_InOutType)
            {
                auto newParam = builder.emitParam(outType->getValueType());
                if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
                    builder.addNameHintDecoration(newParam, nameHint->getName());
                newParams.add(newParam);
            }
        }
        else
        {
            auto newParam = builder.emitParam(param->getDataType());
            if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
                builder.addNameHintDecoration(newParam, nameHint->getName());
            newParams.add(newParam);
        }
    }

    // Create vars for out/inout parameters
    for (auto& varInfo : outVars)
    {
        auto outType = as<IROutTypeBase>(varInfo.origParam->getDataType());
        varInfo.var = builder.emitVar(outType->getValueType());

        if (varInfo.isInOut)
        {
            for (auto newParam : newParams)
            {
                if (auto nameHint = varInfo.origParam->findDecoration<IRNameHintDecoration>())
                {
                    if (auto newNameHint = newParam->findDecoration<IRNameHintDecoration>())
                    {
                        if (nameHint->getName() == newNameHint->getName())
                        {
                            builder.emitStore(varInfo.var, newParam);
                            break;
                        }
                    }
                }
            }
        }
    }

    // Build call to original function
    List<IRInst*> args;
    int newParamIndex = 0;
    int outVarIndex = 0;

    for (auto param : func->getParams())
    {
        if (as<IROutTypeBase>(param->getDataType()))
        {
            args.add(outVars[outVarIndex++].var);
        }
        else
        {
            args.add(newParams[newParamIndex++]);
        }
    }

    IRCall* callResult = builder.emitCallInst(func->getResultType(), func, args);

    // If original function has only one use, inline it
    int useCount = 0;
    for (auto use = func->firstUse; use; use = use->nextUse)
    {
        useCount++;
    }
    if (useCount == 1)
    {
        inlineCall(callResult);
    }

    // Construct return tuple
    List<IRInst*> tupleValues;

    if (!as<IRVoidType>(func->getResultType()))
    {
        tupleValues.add(callResult);
    }

    for (auto& varInfo : outVars)
    {
        tupleValues.add(builder.emitLoad(varInfo.var));
    }

    IRInst* returnValue;
    if (tupleValues.getCount() > 1)
    {
        returnValue = builder.emitMakeTuple(tupleValues);
    }
    else if (tupleValues.getCount() == 1)
    {
        returnValue = tupleValues[0];
    }
    else
    {
        returnValue = nullptr;
    }

    builder.emitReturn(returnValue);

    return newFunc;
}

} // namespace Slang
