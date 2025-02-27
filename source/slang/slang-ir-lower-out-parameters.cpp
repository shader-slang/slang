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

    // Keep track of field information for struct
    struct FieldInfo
    {
        IRStructKey* key;
        String name;
        IRType* type;
    };
    List<FieldInfo> fieldInfos;

    struct VarInfo
    {
        IRVar* var;
        IRParam* origParam;
        bool isInOut;
    };
    List<VarInfo> outVars;

    // If original function returns non-void, add it to return types
    if (!as<IRVoidType>(func->getResultType()))
    {
        returnTypes.add(func->getResultType());

        auto resultKey = builder.createStructKey();
        builder.addNameHintDecoration(resultKey, String("result").getUnownedSlice());
        fieldInfos.add({resultKey, "result", func->getResultType()});
    }

    // Process parameters
    for (auto param : func->getParams())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            auto valueType = outType->getValueType();
            returnTypes.add(valueType);

            // Get parameter name for field name
            String fieldName = "param";
            if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
                fieldName = String(nameHint->getName());

            auto fieldKey = builder.createStructKey();
            builder.addNameHintDecoration(
                fieldKey,
                builder.getStringValue(fieldName.getUnownedSlice()));
            fieldInfos.add({fieldKey, fieldName, valueType});

            if (outType->getOp() == kIROp_InOutType)
            {
                paramTypes.add(valueType);
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

    // Copy all decorations
    for (auto decor : func->getDecorations())
    {
        cloneDecoration(&cloneEnv, decor, newFunc, builder.getModule());
    }

    // Create return struct type if we have multiple return values
    IRType* resultType;
    IRStructType* returnStruct = nullptr;

    if (fieldInfos.getCount() > 1)
    {
        returnStruct = builder.createStructType();

        // Create name for struct
        StringBuilder nameBuilder;
        if (auto nameHint = func->findDecoration<IRNameHintDecoration>())
            nameBuilder << nameHint->getName() << "_Result";
        else
            nameBuilder << "Function_Result";

        builder.addNameHintDecoration(
            returnStruct,
            builder.getStringValue(nameBuilder.toString().getUnownedSlice()));

        // Create fields for the struct
        for (auto& fieldInfo : fieldInfos)
        {
            builder.createStructField(returnStruct, fieldInfo.key, fieldInfo.type);
        }

        resultType = returnStruct;
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
                // Clone all decorations from original parameter
                for (auto decor : param->getDecorations())
                {
                    cloneDecoration(&cloneEnv, decor, newParam, builder.getModule());
                }
                newParams.add(newParam);
            }
        }
        else
        {
            auto newParam = builder.emitParam(param->getDataType());
            // Clone all decorations from original parameter
            for (auto decor : param->getDecorations())
            {
                cloneDecoration(&cloneEnv, decor, newParam, builder.getModule());
            }
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

    // Construct return value
    IRInst* returnValue = nullptr;

    if (returnStruct)
    {
        // Collect field values in order
        List<IRInst*> fieldValues;

        // Add original return value if non-void
        if (!as<IRVoidType>(func->getResultType()))
        {
            fieldValues.add(callResult);
        }

        // Add out parameter values
        for (auto& varInfo : outVars)
        {
            fieldValues.add(builder.emitLoad(varInfo.var));
        }

        // Create struct with all field values
        returnValue = builder.emitMakeStruct(returnStruct, fieldValues);
    }
    else if (returnTypes.getCount() == 1)
    {
        // Single return value
        if (!as<IRVoidType>(func->getResultType()))
        {
            returnValue = callResult;
        }
        else if (outVars.getCount() == 1)
        {
            returnValue = builder.emitLoad(outVars[0].var);
        }
    }

    builder.emitReturn(returnValue);

    fprintf(
        stderr,
        "Original function:\n%s\n",
        dumpIRToString(func, {IRDumpOptions::Mode::Detailed, 0}).getBuffer());
    fprintf(
        stderr,
        "Transformed function:\n%s\n",
        dumpIRToString(newFunc, {IRDumpOptions::Mode::Detailed, 0}).getBuffer());
    fprintf(
        stderr,
        "Transformed function type:\n%s\n",
        dumpIRToString(returnStruct, {IRDumpOptions::Mode::Detailed, 0}).getBuffer());
    return newFunc;
}


} // namespace Slang
