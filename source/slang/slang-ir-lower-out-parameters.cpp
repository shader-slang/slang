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
        IRVarLayout* layout;
    };
    List<FieldInfo> fieldInfos;

    struct VarInfo
    {
        IRVar* var;
        IRParam* origParam;
        bool isInOut;
        IRVarLayout* layout;
    };
    List<VarInfo> outVars;

    // If original function returns non-void, add it to return types
    if (!as<IRVoidType>(func->getResultType()))
    {
        returnTypes.add(func->getResultType());

        auto resultKey = builder.createStructKey();
        builder.addNameHintDecoration(resultKey, UnownedStringSlice("result"));

        // Get layout for the return value if available
        IRVarLayout* resultLayout = nullptr;
        if (auto funcLayout = func->findDecoration<IRLayoutDecoration>())
        {
            if (auto entryPointLayout = as<IREntryPointLayout>(funcLayout->getLayout()))
            {
                resultLayout = entryPointLayout->getResultLayout();
            }
        }

        fieldInfos.add({resultKey, "result", func->getResultType(), resultLayout});
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

            // Get layout for parameter if available
            IRVarLayout* paramLayout = nullptr;
            if (auto layoutDecor = param->findDecoration<IRLayoutDecoration>())
            {
                paramLayout = as<IRVarLayout>(layoutDecor->getLayout());
            }

            fieldInfos.add({fieldKey, fieldName, valueType, paramLayout});

            if (outType->getOp() == kIROp_InOutType)
            {
                paramTypes.add(valueType);
            }

            outVars.add(VarInfo{nullptr, param, outType->getOp() == kIROp_InOutType, paramLayout});
        }
        else
        {
            paramTypes.add(param->getDataType());
        }
    }

    // Create new function
    auto newFunc = builder.createFunc();

    // Copy all decorations except layout (we'll handle that separately)
    for (auto decor : func->getDecorations())
    {
        if (!as<IRLayoutDecoration>(decor))
        {
            cloneDecoration(&cloneEnv, decor, newFunc, builder.getModule());
        }
    }

    // Create return struct type if we have multiple return values
    IRType* resultType;
    IRStructType* returnStruct = nullptr;
    IRVarLayout* newResultLayout = nullptr;

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
            UnownedStringSlice(nameBuilder.toString().getBuffer()));

        // Create struct type layout builder
        IRStructTypeLayout::Builder structTypeLayoutBuilder(&builder);

        // Create fields for the struct
        for (auto& fieldInfo : fieldInfos)
        {
            builder.createStructField(returnStruct, fieldInfo.key, fieldInfo.type);

            // Add layout for this field if available
            if (fieldInfo.layout)
            {
                builder.addLayoutDecoration(fieldInfo.key, fieldInfo.layout);
                structTypeLayoutBuilder.addField(fieldInfo.key, fieldInfo.layout);
            }
        }

        // Build the struct type layout
        auto typeLayout = structTypeLayoutBuilder.build();

        // Create a var layout for the struct
        IRVarLayout::Builder varLayoutBuilder(&builder, typeLayout);
        newResultLayout = varLayoutBuilder.build();

        // Add layout decoration to the struct type
        builder.addLayoutDecoration(returnStruct, newResultLayout);

        resultType = returnStruct;
    }
    else if (returnTypes.getCount() == 1)
    {
        resultType = returnTypes[0];

        // For single return value, use the original result layout if available
        if (!as<IRVoidType>(func->getResultType()))
        {
            if (auto funcLayout = func->findDecoration<IRLayoutDecoration>())
            {
                if (auto entryPointLayout = as<IREntryPointLayout>(funcLayout->getLayout()))
                {
                    newResultLayout = entryPointLayout->getResultLayout();
                }
            }
        }
        else if (outVars.getCount() == 1)
        {
            // Use the layout from the out parameter
            newResultLayout = outVars[0].layout;
        }
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
    IRVarLayout* paramsLayout = nullptr;

    // Get the original parameters layout if available
    if (auto funcLayout = func->findDecoration<IRLayoutDecoration>())
    {
        if (auto entryPointLayout = as<IREntryPointLayout>(funcLayout->getLayout()))
        {
            paramsLayout = entryPointLayout->getParamsLayout();
        }
    }

    for (auto param : func->getParams())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            if (outType->getOp() == kIROp_InOutType)
            {
                auto newParam = builder.emitParam(outType->getValueType());

                // Copy layout decoration explicitly
                if (auto layoutDecor = param->findDecoration<IRLayoutDecoration>())
                {
                    if (auto varLayout = as<IRVarLayout>(layoutDecor->getLayout()))
                    {
                        builder.addLayoutDecoration(newParam, varLayout);
                    }
                }

                // Clone all other decorations from original parameter
                for (auto decor : param->getDecorations())
                {
                    if (!as<IRLayoutDecoration>(decor))
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

            // Copy layout decoration explicitly
            if (auto layoutDecor = param->findDecoration<IRLayoutDecoration>())
            {
                if (auto varLayout = as<IRVarLayout>(layoutDecor->getLayout()))
                {
                    builder.addLayoutDecoration(newParam, varLayout);
                }
            }

            // Clone all other decorations from original parameter
            for (auto decor : param->getDecorations())
            {
                if (!as<IRLayoutDecoration>(decor))
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

        // Copy layout to the variable if available
        if (varInfo.layout)
        {
            builder.addLayoutDecoration(varInfo.var, varInfo.layout);
        }

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

    // Update entry point layout if needed
    if (auto entryPointDecor = func->findDecoration<IREntryPointDecoration>())
    {
        // Get the original entry point layout if available
        IREntryPointLayout* originalEntryPointLayout = nullptr;
        if (auto layoutDecor = func->findDecoration<IRLayoutDecoration>())
        {
            originalEntryPointLayout = as<IREntryPointLayout>(layoutDecor->getLayout());
        }

        if (originalEntryPointLayout)
        {
            // Create a new entry point layout with updated result layout
            auto newEntryPointLayout = builder.getEntryPointLayout(paramsLayout, newResultLayout);

            // Copy any additional properties from the original layout
            // (This would depend on what properties IREntryPointLayout has)

            // Add the new layout decoration to the new function
            builder.addLayoutDecoration(newFunc, newEntryPointLayout);
        }
    }

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
        "Transformed function:\n%s\n",
        dumpIRToString(newFunc->getModule()->getModuleInst(), {IRDumpOptions::Mode::Detailed, 0})
            .getBuffer());
    return newFunc;
}


} // namespace Slang
