#include "slang-ir-lower-out-parameters.h"

#include "slang-ir-clone.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

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

// Find the entry point layout information from a function
static bool findEntryPointLayoutInfo(
    IRFunc* func,
    IREntryPointLayout*& outEntryPointLayout,
    IRVarLayout*& outParamsLayout,
    IRVarLayout*& outResultLayout)
{
    for (auto decor : func->getDecorations())
    {
        if (auto layoutDecor = as<IRLayoutDecoration>(decor))
        {
            if (auto entryLayout = as<IREntryPointLayout>(layoutDecor->getLayout()))
            {
                outEntryPointLayout = entryLayout;
                if (entryLayout->getOperandCount() >= 2)
                {
                    outParamsLayout = as<IRVarLayout>(entryLayout->getOperand(0));
                    outResultLayout = as<IRVarLayout>(entryLayout->getOperand(1));
                    return true;
                }
            }
        }
    }
    return false;
}

// Find semantic decoration for the return value of a function
// Find semantic information for the return value of a function
static bool findReturnValueSemanticInfo(
    IRFunc* func,
    UnownedStringSlice& outSemanticName,
    int& outSemanticIndex)
{
    // Check for semantic on function itself
    if (auto semanticDecor = findSemanticDecoration(func))
    {
        outSemanticName = semanticDecor->getSemanticName();
        outSemanticIndex = semanticDecor->getSemanticIndex();
        return true;
    }

    // Check for semantic in entry point layout
    IREntryPointLayout* entryLayout = nullptr;
    IRVarLayout* paramsLayout = nullptr;
    IRVarLayout* resultLayout = nullptr;

    if (findEntryPointLayoutInfo(func, entryLayout, paramsLayout, resultLayout) && resultLayout)
    {
        for (UInt i = 0; i < resultLayout->getOperandCount(); i++)
        {
            auto operand = resultLayout->getOperand(i);
            if (auto semanticAttr = as<IRSystemValueSemanticAttr>(operand))
            {
                outSemanticName = semanticAttr->getName();
                outSemanticIndex = semanticAttr->getIndex();
                return true;
            }
        }
    }

    return false;
}

// Setup layout for return struct
static void setupReturnStructLayout(
    IRFunc* origFunc,
    IRBuilder& builder,
    IRVarLayout* originalResultLayout,
    List<IRStructKey*>& outKeys,
    Dictionary<IRParam*, IRStructKey*>& paramToKeyMap,
    IRVarLayout*& outResultVarLayout)
{
    // Create struct type layout for return struct
    IRStructTypeLayout::Builder returnStructLayoutBuilder(&builder);

    // Add original return value if present
    if (!as<IRVoidType>(origFunc->getResultType()) && outKeys.getCount())
    {
        // The original return value is always the first field
        if (originalResultLayout)
        {
            returnStructLayoutBuilder.addField(outKeys[0], originalResultLayout);

            // Transfer semantic information from original result layout to key if needed
            bool hasSemanticOnKey = false;
            for (auto decor : outKeys[0]->getDecorations())
            {
                if (as<IRSemanticDecoration>(decor))
                {
                    hasSemanticOnKey = true;
                    break;
                }
            }

            if (!hasSemanticOnKey)
            {
                for (UInt i = 0; i < originalResultLayout->getOperandCount(); i++)
                {
                    auto operand = originalResultLayout->getOperand(i);
                    if (auto semanticAttr = as<IRSystemValueSemanticAttr>(operand))
                    {
                        builder.addSemanticDecoration(
                            outKeys[0],
                            semanticAttr->getName(),
                            semanticAttr->getIndex());
                    }
                }
            }
        }
    }

    // Add fields to the return struct layout
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            // Skip inout params for output layout (they're handled as inputs)
            if (outType->getOp() == kIROp_InOutType)
                continue;

            if (auto paramLayout = findVarLayout(param))
            {
                if (paramToKeyMap.containsKey(param))
                {
                    auto key = paramToKeyMap[param];
                    returnStructLayoutBuilder.addField(key, paramLayout);
                }
            }
        }
    }

    auto returnStructLayout = returnStructLayoutBuilder.build();

    // Create var layout for return struct
    List<IRInst*> resultLayoutOperands;
    resultLayoutOperands.add(returnStructLayout);

    // Find and add stage information
    if (originalResultLayout)
    {
        for (UInt i = 0; i < originalResultLayout->getOperandCount(); i++)
        {
            auto operand = originalResultLayout->getOperand(i);
            if (as<IRStageAttr>(operand))
            {
                resultLayoutOperands.add(operand);
                break;
            }
        }
    }

    // Add system value semantics from out parameters
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        if (const auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            if (auto semanticDecor = findSemanticDecoration(param))
            {
                auto semanticAttr = builder.getSystemValueSemanticAttr(
                    semanticDecor->getSemanticName(),
                    semanticDecor->getSemanticIndex());
                resultLayoutOperands.add(semanticAttr);
            }
        }
    }

    outResultVarLayout = builder.getVarLayout(resultLayoutOperands);
    // Add null check before returning
    if (!outResultVarLayout)
    {
        // Create empty layout if needed
        List<IRInst*> fallbackOperands;
        fallbackOperands.add(returnStructLayout);
        outResultVarLayout = builder.getVarLayout(fallbackOperands);
    }
}

// Setup layout for function parameters
static IRVarLayout* setupParamsLayout(
    IRFunc* origFunc,
    IRBuilder& builder,
    IRVarLayout* originalParamsLayout,
    Dictionary<IRParam*, IRParam*>& origToNewParamMap)
{
    // Extract original struct type layout for parameters
    auto origParamsStructLayout = as<IRStructTypeLayout>(originalParamsLayout->getTypeLayout());
    if (!origParamsStructLayout)
        return nullptr;

    // Create builder for new params layout
    IRStructTypeLayout::Builder paramsLayoutBuilder(&builder);

    // Create mapping from original param to field key/layout
    Dictionary<IRParam*, IRStructFieldLayoutAttr*> paramToFieldAttrMap;

    // Process field layout attributes from original struct layout
    IRParam* param = origFunc->getFirstParam();
    for (auto fieldAttr : origParamsStructLayout->getFieldLayoutAttrs())
    {
        if (!param)
            break;

        paramToFieldAttrMap[param] = fieldAttr;
        param = param->getNextParam();
    }

    // Add fields to new params layout and add decorations to parameters
    for (auto param = origFunc->getFirstParam(); param; param = param->getNextParam())
    {
        // Skip pure out parameters
        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            if (outType->getOp() != kIROp_InOutType)
                continue;
        }

        // Get field layout attribute for this parameter
        if (!paramToFieldAttrMap.containsKey(param))
            continue;

        auto fieldAttr = paramToFieldAttrMap[param];
        auto key = fieldAttr->getFieldKey();
        auto layout = fieldAttr->getLayout();

        // Add field to new params layout
        paramsLayoutBuilder.addField(key, layout);

        // Add layout decoration to new parameter if there's a mapping
        if (origToNewParamMap.containsKey(param))
        {
            auto newParam = origToNewParamMap[param];
            builder.addLayoutDecoration(newParam, layout);
        }
    }

    auto paramsTypeLayout = paramsLayoutBuilder.build();

    // Create var layout for parameters
    List<IRInst*> paramsLayoutOperands;
    paramsLayoutOperands.add(paramsTypeLayout);

    // Copy additional var layout attributes
    for (UInt i = 1; i < originalParamsLayout->getOperandCount(); i++)
    {
        paramsLayoutOperands.add(originalParamsLayout->getOperand(i));
    }

    return builder.getVarLayout(paramsLayoutOperands);
}

// Complete helper to set up entry point layout
static void setupEntryPointLayout(
    IRFunc* origFunc,
    IRFunc* newFunc,
    List<IRStructKey*>& outKeys,
    Dictionary<IRParam*, IRStructKey*>& paramToKeyMap,
    Dictionary<IRParam*, IRParam*>& origToNewParamMap,
    IRBuilder& builder)
{
    // Find the original entry point layout
    IREntryPointLayout* originalEntryPointLayout = nullptr;
    IRVarLayout* originalParamsLayout = nullptr;
    IRVarLayout* originalResultLayout = nullptr;

    if (!findEntryPointLayoutInfo(
            origFunc,
            originalEntryPointLayout,
            originalParamsLayout,
            originalResultLayout))
        return;

    // Setup layouts for the return struct and parameters
    IRVarLayout* resultVarLayout = nullptr;
    setupReturnStructLayout(
        origFunc,
        builder,
        originalResultLayout,
        outKeys,
        paramToKeyMap,
        resultVarLayout);

    IRVarLayout* paramsVarLayout =
        setupParamsLayout(origFunc, builder, originalParamsLayout, origToNewParamMap);

    if (!resultVarLayout || !paramsVarLayout)
        return;

    // Create new entry point layout
    auto entryPointLayout = builder.getEntryPointLayout(paramsVarLayout, resultVarLayout);

    // Add layout decoration to the function
    builder.addLayoutDecoration(newFunc, entryPointLayout);
}

// Structure to hold parameter information
struct ParamInfo
{
    IRParam* origParam;       // Original parameter
    IRType* valueType;        // Parameter value type (without out/inout wrapper)
    bool isOut;               // Is an out parameter
    bool isInOut;             // Is an inout parameter
    IRParam* newParam;        // New param (nullptr for pure out params)
    IRVar* outVar;            // Out variable (nullptr for non-out params)
    IRStructKey* outFieldKey; // Field key (for out params)
};

// Analyze parameters and collect information
List<ParamInfo> collectParameterInfo(
    IRFunc* func,
    IRBuilder& builder,
    List<IRStructKey*>& outKeys,
    Dictionary<IRParam*, IRStructKey*>& paramToKeyMap)
{
    List<ParamInfo> paramInfos;

    for (auto param = func->getFirstParam(); param; param = param->getNextParam())
    {
        ParamInfo info;
        info.origParam = param;
        info.newParam = nullptr;
        info.outVar = nullptr;
        info.outFieldKey = nullptr;

        if (auto outType = as<IROutTypeBase>(param->getDataType()))
        {
            // Handle out/inout parameter
            info.valueType = outType->getValueType();
            info.isOut = true;
            info.isInOut = (outType->getOp() == kIROp_InOutType);

            // Create field key for out parameter
            String fieldName = "param";
            if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
                fieldName = String(nameHint->getName());

            auto fieldKey = builder.createStructKey();
            builder.addNameHintDecoration(fieldKey, UnownedStringSlice(fieldName.getBuffer()));

            // Transfer semantic decorations
            transferSemanticDecorations(param, fieldKey, builder);

            // Store field key for layout
            info.outFieldKey = fieldKey;
            outKeys.add(fieldKey);
            paramToKeyMap[param] = fieldKey;
        }
        else
        {
            // Regular parameter
            info.valueType = param->getDataType();
            info.isOut = false;
            info.isInOut = false;
        }

        paramInfos.add(info);
    }

    return paramInfos;
}

// Create a result key for non-void return types
IRStructKey* createResultKey(IRFunc* func, IRBuilder& builder, List<IRStructKey*>& outKeys)
{
    if (as<IRVoidType>(func->getResultType()))
        return nullptr;

    IRStructKey* resultKey = builder.createStructKey();
    builder.addNameHintDecoration(resultKey, UnownedStringSlice("result"));

    // Transfer semantic decoration from function return value to struct key
    UnownedStringSlice semanticName;
    int semanticIndex = 0;
    if (findReturnValueSemanticInfo(func, semanticName, semanticIndex))
    {
        builder.addSemanticDecoration(resultKey, semanticName, semanticIndex);
    }

    outKeys.add(resultKey);
    return resultKey;
}

// Determine if we need to transform the function
bool needsTransformation(const List<ParamInfo>& paramInfos, bool alwaysUseReturnStruct)
{
    if (alwaysUseReturnStruct)
        return true;

    for (auto& info : paramInfos)
    {
        if (info.isOut && !info.isInOut)
            return true;
    }

    return false;
}

// Create the return type for the new function
IRType* createReturnType(
    IRFunc* func,
    IRBuilder& builder,
    IRStructKey* resultKey,
    const List<IRStructKey*>& outKeys,
    const List<ParamInfo>& paramInfos,
    bool alwaysUseReturnStruct,
    IRStructType*& returnStruct)
{
    // Determine if we need a struct return type
    bool needsStructReturn = alwaysUseReturnStruct ||
                             (resultKey != nullptr && outKeys.getCount()) || outKeys.getCount() > 1;

    if (needsStructReturn)
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
        if (resultKey)
        {
            builder.createStructField(returnStruct, resultKey, func->getResultType());
        }

        for (auto& info : paramInfos)
        {
            if (info.isOut && info.outFieldKey)
            {
                builder.createStructField(returnStruct, info.outFieldKey, info.valueType);
            }
        }

        return returnStruct;
    }
    else if (outKeys.getCount())
    {
        // Find the first out parameter's type
        for (auto& info : paramInfos)
        {
            if (info.isOut)
            {
                return info.valueType;
            }
        }
    }

    // Default case
    return builder.getVoidType();
}

// Create parameters for the new function
void createNewParameters(
    IRBuilder& builder,
    List<ParamInfo>& paramInfos,
    IRCloneEnv& cloneEnv,
    Dictionary<IRParam*, IRParam*>& origToNewParamMap)
{
    for (auto& info : paramInfos)
    {
        if (!info.isOut || info.isInOut)
        {
            // Create parameter
            auto newParam = builder.emitParam(info.valueType);

            // Copy decorations (except layout which is handled later)
            for (auto decor : info.origParam->getDecorations())
            {
                if (!as<IRLayoutDecoration>(decor))
                {
                    cloneDecoration(&cloneEnv, decor, newParam, builder.getModule());
                }
            }

            info.newParam = newParam;
            origToNewParamMap[info.origParam] = newParam;
        }

        if (info.isOut)
        {
            // Create out variable
            auto var = builder.emitVar(info.valueType);
            info.outVar = var;

            // Initialize inout variables from parameters
            if (info.isInOut && info.newParam)
            {
                builder.emitStore(var, info.newParam);
            }
        }
    }
}

// Build the call to the original function
IRCall* buildOriginalFunctionCall(
    IRFunc* func,
    IRBuilder& builder,
    const List<ParamInfo>& paramInfos)
{
    List<IRInst*> args;
    for (auto& info : paramInfos)
    {
        if (info.isOut)
        {
            args.add(info.outVar);
        }
        else
        {
            args.add(info.newParam);
        }
    }

    // Call the original function
    return builder.emitCallInst(func->getResultType(), func, args);
}

// Construct the return value for the new function
IRInst* constructReturnValue(
    IRBuilder& builder,
    IRCall* callResult,
    IRStructKey* resultKey,
    IRStructType* returnStruct,
    const List<IRStructKey*>& outKeys,
    const List<ParamInfo>& paramInfos)
{
    if (returnStruct)
    {
        // Collect field values in order
        List<IRInst*> fieldValues;

        // Add original return value if non-void
        if (resultKey)
        {
            fieldValues.add(callResult);
        }

        // Add out parameter values
        for (auto& info : paramInfos)
        {
            if (info.isOut && info.outVar)
            {
                fieldValues.add(builder.emitLoad(info.outVar));
            }
        }

        // Create struct with all field values
        return builder.emitMakeStruct(returnStruct, fieldValues);
    }
    else if (outKeys.getCount())
    {
        // Single return value
        if (resultKey)
        {
            return callResult;
        }
        else
        {
            // Get the out var from the first out parameter
            for (auto& info : paramInfos)
            {
                if (info.isOut && info.outVar)
                {
                    return builder.emitLoad(info.outVar);
                }
            }
        }
    }

    return nullptr;
}

// Transfer decorations from original to new function
void transferFunctionDecorations(
    IRFunc* func,
    IRFunc* newFunc,
    IRBuilder& builder,
    IRCloneEnv& cloneEnv)
{
    // Copy all decorations except layout
    for (auto decor : func->getDecorations())
    {
        if (!as<IRLayoutDecoration>(decor))
        {
            cloneDecoration(&cloneEnv, decor, newFunc, builder.getModule());
        }
    }

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
}

// Handle cleanup of original function if needed
void handleOriginalFunction(IRFunc* func, IRCall* callResult)
{
    // Count uses of original function
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

        func->removeAndDeallocate();
    }
}

// Main function that orchestrates the transformation
IRFunc* lowerOutParameters(
    IRFunc* func,
    [[maybe_unused]] DiagnosticSink* sink,
    bool alwaysUseReturnStruct)
{
    IRBuilder builder(func->getModule());
    IRCloneEnv cloneEnv;

    // Data structures for tracking parameter information
    List<IRStructKey*> outKeys;
    Dictionary<IRParam*, IRStructKey*> paramToKeyMap;
    Dictionary<IRParam*, IRParam*> origToNewParamMap;

    // Create result key for non-void return types
    IRStructKey* resultKey = createResultKey(func, builder, outKeys);

    // Collect parameter information
    List<ParamInfo> paramInfos = collectParameterInfo(func, builder, outKeys, paramToKeyMap);

    // Check if transformation is needed
    if (!needsTransformation(paramInfos, alwaysUseReturnStruct))
        return func;

    // Create new function
    auto newFunc = builder.createFunc();

    // Transfer decorations to new function
    transferFunctionDecorations(func, newFunc, builder, cloneEnv);

    // Create return type
    IRStructType* returnStruct = nullptr;
    IRType* resultType = createReturnType(
        func,
        builder,
        resultKey,
        outKeys,
        paramInfos,
        alwaysUseReturnStruct,
        returnStruct);

    // Collect parameter types for new function
    List<IRType*> newParamTypes;
    for (auto& info : paramInfos)
    {
        if (!info.isOut || info.isInOut)
        {
            newParamTypes.add(info.valueType);
        }
    }

    // Set function type
    auto funcType = builder.getFuncType(newParamTypes, resultType);
    newFunc->setFullType(funcType);

    // Create function body
    auto firstBlock = builder.createBlock();
    newFunc->addBlock(firstBlock);
    builder.setInsertInto(firstBlock);

    // Create parameters and variables
    createNewParameters(builder, paramInfos, cloneEnv, origToNewParamMap);

    // Build call to original function
    IRCall* callResult = buildOriginalFunctionCall(func, builder, paramInfos);

    // Construct return value
    IRInst* returnValue =
        constructReturnValue(builder, callResult, resultKey, returnStruct, outKeys, paramInfos);

    builder.emitReturn(returnValue);

    // Set up layout information
    if (returnStruct)
    {
        setupEntryPointLayout(func, newFunc, outKeys, paramToKeyMap, origToNewParamMap, builder);
    }

    // Handle cleanup of original function
    handleOriginalFunction(func, callResult);

    return newFunc;
}


} // namespace Slang
