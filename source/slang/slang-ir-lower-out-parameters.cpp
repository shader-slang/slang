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

            // Copy semantic information
            for (auto decor : param->getDecorations())
            {
                if (auto semanticDecor = as<IRSemanticDecoration>(decor))
                {
                    // Clone the semantic decoration to the new struct key
                    builder.addSemanticDecoration(
                        fieldKey,
                        semanticDecor->getSemanticName(),
                        semanticDecor->getSemanticIndex());
                }
            }

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

            // Skip fields that don't come from parameters
            if (!fieldInfo.origParam)
                continue;

            // Transfer semantic decorations from the original parameter to the struct key
            for (auto decor : fieldInfo.origParam->getDecorations())
            {
                if (auto semanticDecor = as<IRSemanticDecoration>(decor))
                {
                    // Add semantic decoration to the struct key in the new struct
                    builder.addSemanticDecoration(
                        fieldInfo.key,
                        semanticDecor->getSemanticName(),
                        semanticDecor->getSemanticIndex());
                }
            }
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
                auto newParam = builder.emitParam(param->getDataType());

                // For regular parameters, copy ALL decorations including semantics and layout
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

    if (returnStruct)
    {
        // Find the original entry point layout and extract components
        IREntryPointLayout* originalEntryPointLayout = nullptr;
        IRVarLayout* originalParamsLayout = nullptr;
        IRVarLayout* originalResultLayout = nullptr;

        for (auto decor : func->getDecorations())
        {
            if (auto layoutDecor = as<IRLayoutDecoration>(decor))
            {
                if (auto entryLayout = as<IREntryPointLayout>(layoutDecor->getLayout()))
                {
                    originalEntryPointLayout = entryLayout;
                    if (entryLayout->getOperandCount() >= 2)
                    {
                        originalParamsLayout = as<IRVarLayout>(entryLayout->getOperand(0));
                        originalResultLayout = as<IRVarLayout>(entryLayout->getOperand(1));
                    }
                    break;
                }
            }
        }

        if (originalEntryPointLayout && originalParamsLayout)
        {
            // Create struct type layout for return struct
            IRStructTypeLayout::Builder returnStructLayoutBuilder(&builder);

            // Add fields to the return struct layout
            for (auto& fieldInfo : fieldInfos)
            {
                if (!fieldInfo.origParam)
                    continue;

                // Find the parameter's original layout
                IRVarLayout* paramLayout = nullptr;
                for (auto decor : fieldInfo.origParam->getDecorations())
                {
                    if (auto layoutDecor = as<IRLayoutDecoration>(decor))
                    {
                        paramLayout = as<IRVarLayout>(layoutDecor->getLayout());
                        break;
                    }
                }

                if (paramLayout)
                {
                    returnStructLayoutBuilder.addField(fieldInfo.key, paramLayout);
                }
            }

            auto returnStructLayout = returnStructLayoutBuilder.build();

            // Create var layout for return struct with semantics
            List<IRInst*> resultLayoutOperands;
            resultLayoutOperands.add(returnStructLayout);

            // Find and add stage information
            IRInst* stageAttr = nullptr;
            if (originalResultLayout)
            {
                for (UInt i = 0; i < originalResultLayout->getOperandCount(); i++)
                {
                    auto operand = originalResultLayout->getOperand(i);
                    if (as<IRStageAttr>(operand))
                    {
                        stageAttr = operand;
                        resultLayoutOperands.add(stageAttr);
                        break;
                    }
                }
            }

            // Add system value semantics from the out parameters
            for (auto& fieldInfo : fieldInfos)
            {
                if (!fieldInfo.origParam)
                    continue;

                for (auto decor : fieldInfo.origParam->getDecorations())
                {
                    if (auto semanticDecor = as<IRSemanticDecoration>(decor))
                    {
                        auto semanticAttr = builder.getSystemValueSemanticAttr(
                            semanticDecor->getSemanticName(),
                            semanticDecor->getSemanticIndex());
                        resultLayoutOperands.add(semanticAttr);
                    }
                }
            }

            // Create the return value layout
            auto resultVarLayout = builder.getVarLayout(resultLayoutOperands);

            // Handle input parameters layout preservation
            // Extract struct type layout for input parameters
            auto origParamsStructLayout =
                as<IRStructTypeLayout>(originalParamsLayout->getTypeLayout());
            if (!origParamsStructLayout)
                return newFunc;

            // Create new params structure layout
            IRStructTypeLayout::Builder paramsLayoutBuilder(&builder);

            // Track which parameters are regular inputs vs out parameters
            int fieldIndex = 0;
            int newParamIndex = 0;

            // Iterate through original parameters
            for (auto param : func->getParams())
            {
                if (auto outType = as<IROutTypeBase>(param->getDataType()))
                {
                    // Skip out parameters, they're now in the return struct
                    if (outType->getOp() != kIROp_InOutType)
                    {
                        fieldIndex++;
                        continue;
                    }

                    // InOut parameters need appropriate layout info transferred
                    // They appear as regular parameters in the new function
                }

                // Handle regular inputs (including inout parameters that become inputs)
                if (fieldIndex < origParamsStructLayout->getFieldCount())
                {
                    auto fieldLayoutAttr =
                        origParamsStructLayout->getFieldLayoutAttrs()[fieldIndex];
                    auto key = fieldLayoutAttr->getFieldKey();
                    auto layout = fieldLayoutAttr->getLayout();

                    // Add field to new params layout
                    paramsLayoutBuilder.addField(key, layout);

                    // Also add layout decoration to new parameter
                    builder.addLayoutDecoration(newParams[newParamIndex], layout);

                    // Add semantic decoration to parameter if present
                    for (auto decor : param->getDecorations())
                    {
                        if (auto semanticDecor = as<IRSemanticDecoration>(decor))
                        {
                            builder.addSemanticDecoration(
                                newParams[newParamIndex],
                                semanticDecor->getSemanticName(),
                                semanticDecor->getSemanticIndex());
                        }
                    }
                }

                newParamIndex++;
                fieldIndex++;
            }

            auto paramsTypeLayout = paramsLayoutBuilder.build();

            // Create var layout for parameters
            List<IRInst*> paramsLayoutOperands;
            paramsLayoutOperands.add(paramsTypeLayout);

            // Copy any additional var layout attributes (beyond type layout)
            for (UInt i = 1; i < originalParamsLayout->getOperandCount(); i++)
            {
                paramsLayoutOperands.add(originalParamsLayout->getOperand(i));
            }

            auto paramsVarLayout = builder.getVarLayout(paramsLayoutOperands);

            // Create new entry point layout
            auto entryPointLayout = builder.getEntryPointLayout(paramsVarLayout, resultVarLayout);

            // Add layout decoration to the function
            builder.addLayoutDecoration(newFunc, entryPointLayout);
        }
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
