#include "slang-ir-lower-out-parameters.h"

#include "slang-ir-clone.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Helper functions for decoration handling
static IRVarLayout* findVarLayout_(IRInst* inst)
{
    for (auto decor : inst->getDecorations())
    {
        if (auto layoutDecor = as<IRLayoutDecoration>(decor))
        {
            return as<IRVarLayout>(layoutDecor->getLayout());
        }
    }
    return nullptr;
}

static IRSemanticDecoration* findSemanticDecoration(IRInst* inst)
{
    for (auto decor : inst->getDecorations())
    {
        if (auto semanticDecor = as<IRSemanticDecoration>(decor))
        {
            return semanticDecor;
        }
    }
    return nullptr;
}

static void transferSemanticDecorations(IRInst* source, IRInst* target, IRBuilder& builder)
{
    for (auto decor : source->getDecorations())
    {
        if (auto semanticDecor = as<IRSemanticDecoration>(decor))
        {
            builder.addSemanticDecoration(
                target,
                semanticDecor->getSemanticName(),
                semanticDecor->getSemanticIndex());
        }
    }
}

// Helper to set up entry point layout

static void setupEntryPointLayout(
    IRFunc* origFunc,
    IRFunc* newFunc,
    IRStructType* returnStruct,
    List<IRInst*>& fieldKeys,
    Dictionary<IRParam*, int>& outParamMap,
    List<IRParam*>& newParams,
    Dictionary<IRParam*, IRParam*>& origToNewParamMap,
    IRBuilder& builder)
{
    // Find the original entry point layout
    IREntryPointLayout* originalEntryPointLayout = nullptr;
    IRVarLayout* originalParamsLayout = nullptr;
    IRVarLayout* originalResultLayout = nullptr;

    for (auto decor : origFunc->getDecorations())
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

    if (!originalEntryPointLayout || !originalParamsLayout)
        return;

    // Extract original struct type layout for parameters
    auto origParamsStructLayout = as<IRStructTypeLayout>(originalParamsLayout->getTypeLayout());
    if (!origParamsStructLayout)
        return;

    // =======================================================================
    // Create struct type layout for return struct
    // =======================================================================
    IRStructTypeLayout::Builder returnStructLayoutBuilder(&builder);

    // Add fields to the return struct layout
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            // Skip inout params for output layout (they're handled as inputs)
            if (outType->getOp() == kIROp_InOutType)
                continue;

            IRVarLayout* paramLayout = findVarLayout(param);
            if (paramLayout && outParamMap.containsKey(param))
            {
                int index = outParamMap[param];
                returnStructLayoutBuilder.addField(fieldKeys[index], paramLayout);
            }
        }
    }

    // Add original return value if present
    if (!as<IRVoidType>(origFunc->getResultType()) && fieldKeys.getCount() > 0)
    {
        // Assume first field key is for the original return value
        if (originalResultLayout)
        {
            returnStructLayoutBuilder.addField(fieldKeys[0], originalResultLayout);
        }
    }

    auto returnStructLayout = returnStructLayoutBuilder.build();

    // =======================================================================
    // Create var layout for return struct
    // =======================================================================
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

    // Add system value semantics from out parameters
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            auto semanticDecor = findSemanticDecoration(param);
            if (semanticDecor)
            {
                auto semanticAttr = builder.getSystemValueSemanticAttr(
                    semanticDecor->getSemanticName(),
                    semanticDecor->getSemanticIndex());
                resultLayoutOperands.add(semanticAttr);
            }
        }
    }

    auto resultVarLayout = builder.getVarLayout(resultLayoutOperands);

    // =======================================================================
    // Create params layout for input parameters
    // =======================================================================
    IRStructTypeLayout::Builder paramsLayoutBuilder(&builder);

    // Build key-to-layout mapping from original params layout
    Dictionary<IRInst*, IRVarLayout*> keyToLayoutMap;
    for (auto fieldAttr : origParamsStructLayout->getFieldLayoutAttrs())
    {
        keyToLayoutMap[fieldAttr->getFieldKey()] = fieldAttr->getLayout();
    }

    // Map param-to-key from original layout
    Dictionary<IRParam*, IRInst*> paramToKeyMap;

    // Create a temporary mapping of parameter index to field
    int paramIndex = 0;
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        if (paramIndex < int(origParamsStructLayout->getFieldLayoutAttrs().getCount()))
        {
            auto fieldAttr = origParamsStructLayout->getFieldLayoutAttrs()[paramIndex];
            paramToKeyMap[param] = fieldAttr->getFieldKey();
        }
        paramIndex++;
    }

    // Process original parameters to add fields to new params layout
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        // Skip pure out parameters
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            if (outType->getOp() != kIROp_InOutType)
                continue;
        }

        // Get layout for this parameter
        IRVarLayout* paramLayout = findVarLayout(param);
        if (!paramLayout)
            continue;

        // Get the key for this parameter
        if (!paramToKeyMap.containsKey(param))
            continue;

        auto key = paramToKeyMap[param];

        // Add field to new params layout
        paramsLayoutBuilder.addField(key, paramLayout);

        // Add layout decoration to new parameter if there's a mapping
        if (origToNewParamMap.containsKey(param))
        {
            auto newParam = origToNewParamMap[param];
            builder.addLayoutDecoration(newParam, paramLayout);

            // Ensure semantic decoration is present
            IRSemanticDecoration* origSemantic = findSemanticDecoration(param);
            IRSemanticDecoration* newSemantic = findSemanticDecoration(newParam);

            if (origSemantic && !newSemantic)
            {
                builder.addSemanticDecoration(
                    newParam,
                    origSemantic->getSemanticName(),
                    origSemantic->getSemanticIndex());
            }
        }
    }

    auto paramsTypeLayout = paramsLayoutBuilder.build();

    // =======================================================================
    // Create var layout for parameters
    // =======================================================================
    List<IRInst*> paramsLayoutOperands;
    paramsLayoutOperands.add(paramsTypeLayout);

    // Copy additional var layout attributes
    for (UInt i = 1; i < originalParamsLayout->getOperandCount(); i++)
    {
        paramsLayoutOperands.add(originalParamsLayout->getOperand(i));
    }

    auto paramsVarLayout = builder.getVarLayout(paramsLayoutOperands);

    // =======================================================================
    // Create entry point layout and apply to function
    // =======================================================================
    auto entryPointLayout = builder.getEntryPointLayout(paramsVarLayout, resultVarLayout);
    builder.addLayoutDecoration(newFunc, entryPointLayout);
}


IRFunc* lowerOutParameters(IRFunc* func, DiagnosticSink* sink, bool alwaysUseReturnStruct)
{
#ifdef SLANG_DEBUG
    SourceManager s;
    DiagnosticSinkWriter w(sink);
    dumpIR(func->getModule(), {}, "MODULE BEFORE", &s, &w);
#endif

    IRBuilder builder(func->getModule());
    IRCloneEnv cloneEnv;

    // Parameter classification
    struct ParamInfo
    {
        IRType* type;       // Parameter type
        bool isOut;         // Is out parameter
        bool isInOut;       // Is inout parameter
        IRParam* origParam; // Original parameter
        int fieldIndex;     // Index in field list (-1 if not an out parameter)
    };
    List<ParamInfo> paramInfos;

    // Field information for struct return
    struct FieldInfo
    {
        IRStructKey* key;
        String name;
        IRType* type;
        IRParam* origParam; // nullptr for original return value
    };
    List<FieldInfo> fieldInfos;

    // Track struct keys for layout
    List<IRInst*> fieldKeys;

    // Map from original parameter to field index
    Dictionary<IRParam*, int> outParamFieldMap;

    // If original function returns non-void, add it to return struct
    if (!as<IRVoidType>(func->getResultType()))
    {
        auto resultKey = builder.createStructKey();
        builder.addNameHintDecoration(resultKey, UnownedStringSlice("result"));
        fieldInfos.add({resultKey, "result", func->getResultType(), nullptr});
        fieldKeys.add(resultKey);
    }

    // Process parameters
    for (auto param : func->getParams())
    {
        ParamInfo info;
        info.origParam = param;
        info.fieldIndex = -1;

        if (const auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            auto valueType = outType->getValueType();
            info.type = valueType;
            info.isOut = true;
            info.isInOut = (outType->getOp() == kIROp_InOutType);

            // Get parameter name for field
            String fieldName = "param";
            if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
                fieldName = String(nameHint->getName());

            auto fieldKey = builder.createStructKey();
            builder.addNameHintDecoration(fieldKey, UnownedStringSlice(fieldName.getBuffer()));

            // Transfer semantic decorations to the key
            transferSemanticDecorations(param, fieldKey, builder);

            // Add to field list
            info.fieldIndex = fieldInfos.getCount();
            fieldInfos.add({fieldKey, fieldName, valueType, param});
            fieldKeys.add(fieldKey);
            outParamFieldMap[param] = fieldKeys.getCount() - 1;
        }
        else
        {
            info.type = param->getDataType();
            info.isOut = false;
            info.isInOut = false;
        }

        paramInfos.add(info);
    }

    // Nothing to do if no out parameters
    bool hasOutParams = false;
    for (auto& info : paramInfos)
    {
        if (info.isOut && !info.isInOut)
        {
            hasOutParams = true;
            break;
        }
    }

    if (!hasOutParams && !alwaysUseReturnStruct)
        return func;

    // Create new function
    auto newFunc = builder.createFunc();

    // Copy all decorations except layout
    for (auto decor : func->getDecorations())
    {
        if (!as<IRLayoutDecoration>(decor))
        {
            cloneDecoration(&cloneEnv, decor, newFunc, builder.getModule());
        }
    }

    // Create return type
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
    else if (fieldInfos.getCount() == 1)
    {
        resultType = fieldInfos[0].type;
    }
    else
    {
        resultType = builder.getVoidType();
    }

    // Collect parameter types for new function
    List<IRType*> newParamTypes;
    for (auto& info : paramInfos)
    {
        if (!info.isOut || info.isInOut)
        {
            newParamTypes.add(info.type);
        }
    }

    auto funcType = builder.getFuncType(newParamTypes, resultType);
    newFunc->setFullType(funcType);

    auto firstBlock = builder.createBlock();
    newFunc->addBlock(firstBlock);
    builder.setInsertInto(firstBlock);

    // Create parameters for new function
    List<IRParam*> newParams;
    List<IRVar*> outVars;
    Dictionary<IRParam*, IRParam*> origToNewParamMap; // Track mapping

    for (auto& info : paramInfos)
    {
        if (!info.isOut || info.isInOut)
        {
            // Create parameter
            auto newParam = builder.emitParam(info.type);

            // Copy ALL decorations except layout
            for (auto decor : info.origParam->getDecorations())
            {
                if (!as<IRLayoutDecoration>(decor))
                {
                    cloneDecoration(&cloneEnv, decor, newParam, builder.getModule());
                }
            }

            newParams.add(newParam);
            origToNewParamMap[info.origParam] = newParam; // Store mapping
        }


        if (info.isOut)
        {
            // Create variable for out parameter
            auto var = builder.emitVar(info.type);
            outVars.add(var);

            // Initialize inout variables from parameters
            if (info.isInOut)
            {
                int paramIndex = newParams.getCount() - 1;
                builder.emitStore(var, newParams[paramIndex]);
            }
        }
        else
        {
            outVars.add(nullptr);
        }
    }

    // Build call to original function
    List<IRInst*> args;
    int regularParamIndex = 0;

    for (int i = 0; i < paramInfos.getCount(); i++)
    {
        if (paramInfos[i].isOut)
        {
            args.add(outVars[i]);
        }
        else
        {
            args.add(newParams[regularParamIndex++]);
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

        // Remove decorations from old function
        List<IRDecoration*> decorationsToRemove;
        for (auto decor : func->getDecorations())
        {
            if (as<IRKeepAliveDecoration>(decor) || as<IREntryPointDecoration>(decor))
            {
                decorationsToRemove.add(decor);
            }
        }

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
        for (int i = 0; i < paramInfos.getCount(); i++)
        {
            if (paramInfos[i].isOut)
            {
                fieldValues.add(builder.emitLoad(outVars[i]));
            }
        }

        // Create struct with all field values
        returnValue = builder.emitMakeStruct(returnStruct, fieldValues);
    }
    else if (fieldInfos.getCount() == 1)
    {
        // Single return value
        if (!as<IRVoidType>(func->getResultType()))
        {
            returnValue = callResult;
        }
        else
        {
            // Find the out parameter
            for (int i = 0; i < paramInfos.getCount(); i++)
            {
                if (paramInfos[i].isOut)
                {
                    returnValue = builder.emitLoad(outVars[i]);
                    break;
                }
            }
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

    // Set up layout information
    if (returnStruct)
    {
        setupEntryPointLayout(
            func,
            newFunc,
            returnStruct,
            fieldKeys,
            outParamFieldMap,
            newParams,
            origToNewParamMap, // Pass the parameter mapping
            builder);
    }

    if (useCount == 1)
        func->removeAndDeallocate();

#ifdef SLANG_DEBUG
    dumpIR(newFunc->getModule(), {}, "MODULE AFTER", &s, &w);
#endif

    return newFunc;
}

} // namespace Slang
