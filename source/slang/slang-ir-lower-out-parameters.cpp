#include "slang-ir-lower-out-parameters.h"

#include "slang-ir-clone.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink, bool alwaysUseReturnStruct)
{
    SourceManager s;
    DiagnosticSinkWriter w(sink);
    dumpIR(func->getModule(), {}, "MODULE BEFORE", &s, &w);

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
        IRParam* origParam; // Track the original parameter for this field
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
        builder.addNameHintDecoration(resultKey, UnownedStringSlice("result"));
        fieldInfos.add({resultKey, "result", func->getResultType(), nullptr});
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
            builder.addNameHintDecoration(fieldKey, UnownedStringSlice(fieldName.getBuffer()));
            fieldInfos.add({fieldKey, fieldName, valueType, param});

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

    // Nothing to do
    if (outVars.getCount() == 0)
        return func;

    // Create new function
    auto newFunc = builder.createFunc();

    // Copy all decorations except layout
    for (auto decor : func->getDecorations())
    {
        if (!as<IRLayoutDecoration>(decor) && !as<IRSemanticDecoration>(decor))
        {
            cloneDecoration(&cloneEnv, decor, newFunc, builder.getModule());
        }
    }

    // Create return struct type if we have multiple return values
    IRType* resultType;
    IRStructType* returnStruct = nullptr;

    if (fieldInfos.getCount() > 1 || alwaysUseReturnStruct)
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
            UnownedStringSlice(nameBuilder.toString().getBuffer()));

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

                // Clone all decorations except layout and semantic from original parameter
                for (auto decor : param->getDecorations())
                {
                    if (!as<IRLayoutDecoration>(decor) && !as<IRSemanticDecoration>(decor))
                    {
                        cloneDecoration(&cloneEnv, decor, newParam, builder.getModule());
                    }
                }

                newParams.add(newParam);
            }
        }
        else
        {
            auto newParam = builder.emitParam(param->getDataType());

            // Clone all decorations except layout and semantic from original parameter
            for (auto decor : param->getDecorations())
            {
                if (!as<IRLayoutDecoration>(decor) && !as<IRSemanticDecoration>(decor))
                {
                    cloneDecoration(&cloneEnv, decor, newParam, builder.getModule());
                }
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
            // Find the corresponding parameter by direct pointer comparison
            int newParamIndex = 0;
            for (auto param : func->getParams())
            {
                if (!as<IROutTypeBase>(param->getDataType()) ||
                    as<IROutTypeBase>(param->getDataType())->getOp() != kIROp_InOutType)
                {
                    newParamIndex++;
                    continue;
                }

                if (param == varInfo.origParam)
                {
                    builder.emitStore(varInfo.var, newParams[newParamIndex]);
                    break;
                }
                newParamIndex++;
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
        useCount++;
    if (useCount == 1)
    {
        inlineCall(callResult);

        // Remove keepAlive and entryPoint decorations from old function
        List<IRDecoration*> decorationsToRemove;
        for (auto decor : func->getDecorations())
        {
            if (as<IRKeepAliveDecoration>(decor) || as<IREntryPointDecoration>(decor))
            {
                decorationsToRemove.add(decor);
            }
        }

        // Remove them after iteration is complete
        for (auto decor : decorationsToRemove)
        {
            decor->removeFromParent();
        }
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

    // Transfer entry point decoration if present
    if (auto entryPointDecor = func->findDecoration<IREntryPointDecoration>())
    {
        builder.addEntryPointDecoration(
            newFunc,
            entryPointDecor->getProfile(),
            entryPointDecor->getName()->getStringSlice(),
            entryPointDecor->getModuleName()->getStringSlice());
    }

    // Add keepAlive decoration to ensure the new function is preserved
    builder.addKeepAliveDecoration(newFunc);


    if (useCount == 1)
        func->removeAndDeallocate();
    dumpIR(newFunc->getModule(), {}, "MODULE AFTER", &s, &w);
    // fprintf(
    //     stderr,
    //     "Original function:\n%s\n",
    //     dumpIRToString(func, {IRDumpOptions::Mode::Detailed, 0}).getBuffer());
    // fprintf(
    //     stderr,
    //     "Transformed function:\n%s\n",
    //     dumpIRToString(newFunc, {IRDumpOptions::Mode::Detailed, 0}).getBuffer());

    return newFunc;
}

} // namespace Slang
