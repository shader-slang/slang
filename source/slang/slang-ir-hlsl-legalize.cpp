// slang-ir-hlsl-legalize.cpp
#include "slang-ir-hlsl-legalize.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

#include <functional>

namespace Slang
{

static void addDefaultPayloadAccessQualifiersToField(IRBuilder& builder, IRStructKey* fieldKey)
{
    const bool hasReadAccess = fieldKey->findDecoration<IRStageReadAccessDecoration>() != nullptr;
    const bool hasWriteAccess = fieldKey->findDecoration<IRStageWriteAccessDecoration>() != nullptr;
    if (hasReadAccess && hasWriteAccess)
        return;

    IRInst* stageNames[] = {
        builder.getStringValue(UnownedStringSlice("caller")),
        builder.getStringValue(UnownedStringSlice("anyhit")),
        builder.getStringValue(UnownedStringSlice("closesthit")),
        builder.getStringValue(UnownedStringSlice("miss")),
    };

    if (!hasReadAccess)
    {
        builder.addDecoration(
            fieldKey,
            kIROp_StageReadAccessDecoration,
            stageNames,
            SLANG_COUNT_OF(stageNames));
    }

    if (!hasWriteAccess)
    {
        builder.addDecoration(
            fieldKey,
            kIROp_StageWriteAccessDecoration,
            stageNames,
            SLANG_COUNT_OF(stageNames));
    }
}

static void addDefaultPayloadAccessQualifiersToStruct(IRBuilder& builder, IRStructType* structType)
{
    for (auto field : structType->getFields())
    {
        addDefaultPayloadAccessQualifiersToField(builder, field->getKey());
    }
}

static void addRayPayloadDecorationIfNeeded(IRBuilder& builder, IRType* type)
{
    if (!type->findDecoration<IRRayPayloadDecoration>())
        builder.addRayPayloadDecoration(type);
}

static bool isPrimitiveIDSystemValueParam(IRParam* param)
{
    if (auto semanticDecor = param->findDecoration<IRSemanticDecoration>())
    {
        if (semanticDecor->getSemanticName().caseInsensitiveEquals(toSlice("sv_primitiveid")))
            return true;
    }

    auto layoutDecor = param->findDecoration<IRLayoutDecoration>();
    if (!layoutDecor)
        return false;

    auto varLayout = as<IRVarLayout>(layoutDecor->getLayout());
    if (!varLayout)
        return false;

    auto systemValueAttr = varLayout->findSystemValueSemanticAttr();
    if (!systemValueAttr)
        return false;

    return systemValueAttr->getName().caseInsensitiveEquals(toSlice("sv_primitiveid"));
}

static IRFunc* getRayTracingPrimitiveIndexFunc(IRModule* module, IRFunc*& primitiveIndexFunc)
{
    if (primitiveIndexFunc)
        return primitiveIndexFunc;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    primitiveIndexFunc = builder.createFunc();
    builder.setDataType(primitiveIndexFunc, builder.getFuncType(0, nullptr, builder.getUIntType()));
    builder.addTargetIntrinsicDecoration(
        primitiveIndexFunc,
        CapabilitySet(CapabilityName::hlsl),
        UnownedTerminatedStringSlice("PrimitiveIndex"));
    builder.addTargetIntrinsicDecoration(
        primitiveIndexFunc,
        CapabilitySet(CapabilityName::cuda),
        UnownedTerminatedStringSlice("optixGetPrimitiveIndex"));
    return primitiveIndexFunc;
}

void searchChildrenForForceVarIntoStructTemporarily(IRModule* module, IRInst* inst)
{
    for (auto child : inst->getChildren())
    {
        switch (child->getOp())
        {
        case kIROp_Block:
            {
                searchChildrenForForceVarIntoStructTemporarily(module, child);
                break;
            }
        case kIROp_Call:
            {
                auto call = as<IRCall>(child);
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    auto arg = call->getArg(i);
                    const bool isForcedStruct = arg->getOp() == kIROp_ForceVarIntoStructTemporarily;
                    const bool isForcedRayPayloadStruct =
                        arg->getOp() == kIROp_ForceVarIntoRayPayloadStructTemporarily;
                    if (!(isForcedStruct || isForcedRayPayloadStruct))
                        continue;
                    auto forceStructArg = arg->getOperand(0);
                    auto forceStructBaseType =
                        (IRType*)(forceStructArg->getDataType()->getOperand(0));
                    IRBuilder builder(call);
                    if (forceStructBaseType->getOp() == kIROp_StructType)
                    {
                        call->setArg(i, arg->getOperand(0));
                        if (isForcedRayPayloadStruct)
                        {
                            addRayPayloadDecorationIfNeeded(builder, forceStructBaseType);
                            addDefaultPayloadAccessQualifiersToStruct(
                                builder,
                                cast<IRStructType>(forceStructBaseType));
                        }
                        continue;
                    }

                    // When `__forceVarIntoStructTemporarily` is called with a non-struct type
                    // parameter, we create a temporary struct and copy the parameter into the
                    // struct. This struct is then subsituted for the return of
                    // `__forceVarIntoStructTemporarily`. Optionally, if
                    // `__forceVarIntoStructTemporarily` is a parameter to a side effect type
                    // (`ref`, `out`, `inout`) we copy the struct back into our original non-struct
                    // parameter.

                    const auto typeNameHint = isForcedRayPayloadStruct
                                                  ? "RayPayload_t"
                                                  : "ForceVarIntoStructTemporarily_t";
                    const auto varNameHint =
                        isForcedRayPayloadStruct ? "rayPayload" : "forceVarIntoStructTemporarily";

                    builder.setInsertBefore(call->getCallee());
                    auto structType = builder.createStructType();
                    builder.addNameHintDecoration(structType, UnownedStringSlice(typeNameHint));
                    if (isForcedRayPayloadStruct)
                        addRayPayloadDecorationIfNeeded(builder, structType);

                    auto elementBufferKey = builder.createStructKey();
                    builder.addNameHintDecoration(elementBufferKey, UnownedStringSlice("data"));
                    if (isForcedRayPayloadStruct)
                    {
                        addDefaultPayloadAccessQualifiersToField(builder, elementBufferKey);
                    }
                    auto _dataField = builder.createStructField(
                        structType,
                        elementBufferKey,
                        forceStructBaseType);

                    builder.setInsertBefore(call);
                    auto structVar = builder.emitVar(structType);
                    builder.addNameHintDecoration(structVar, UnownedStringSlice(varNameHint));
                    builder.emitStore(
                        builder.emitFieldAddress(
                            builder.getPtrType(_dataField->getFieldType()),
                            structVar,
                            _dataField->getKey()),
                        builder.emitLoad(forceStructArg));

                    arg->replaceUsesWith(structVar);
                    arg->removeAndDeallocate();

                    auto argType = call->getCallee()->getDataType()->getOperand(i + 1);
                    if (!isPtrLikeOrHandleType(argType))
                        continue;

                    builder.setInsertAfter(call);
                    builder.emitStore(
                        forceStructArg,
                        builder.emitFieldAddress(
                            builder.getPtrType(_dataField->getFieldType()),
                            structVar,
                            _dataField->getKey()));
                }
                break;
            }
        }
    }
}

void legalizeNonStructParameterToStructForHLSL(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        // Only process functions - at this stage generics are already resolved,
        // and the search only handles Block and Call children.
        if (globalInst->getOp() != kIROp_Func)
            continue;
        searchChildrenForForceVarIntoStructTemporarily(module, globalInst);
    }
}

void legalizeEmptyRayPayloadsForHLSL(IRModule* module)
{
    // DXIL/HLSL with NVAPI requires non-empty ray payload structs because
    // the NvInvokeHitObject macro expects a Payload argument.
    IRBuilder builder(module);

    // First, collect all empty ray payload structs to process.
    // We must collect first because the processing phase inserts new global
    // instructions (struct keys, string values) which would invalidate the iterator.
    HashSet<IRStructType*> emptyRayPayloadStructs;

    for (auto globalInst : module->getGlobalInsts())
    {
        auto structType = as<IRStructType>(globalInst);
        if (!structType)
        {
            // Also check global variables with IRVulkanRayPayloadDecoration.
            // These arise from [__vulkanRayPayload] parameters in built-in functions
            // (e.g. __spirvTraceRayHitObjectEXT) where the decoration is on the
            // variable rather than the struct type itself.
            auto globalVar = as<IRGlobalVar>(globalInst);
            if (!globalVar)
                continue;
            if (!globalVar->findDecoration<IRVulkanRayPayloadDecoration>())
                continue;
            auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
            if (!ptrType)
                continue;
            structType = as<IRStructType>(ptrType->getValueType());
            if (!structType)
                continue;
            // Check if the struct is empty
            if (structType->getFields().begin() != structType->getFields().end())
                continue;
            emptyRayPayloadStructs.add(structType);
            continue;
        }

        // Check if this struct has ray payload decoration
        auto rayPayloadDec = structType->findDecoration<IRRayPayloadDecoration>();
        auto vulkanRayPayloadDec = structType->findDecoration<IRVulkanRayPayloadDecoration>();
        bool isRayPayload = rayPayloadDec != nullptr || vulkanRayPayloadDec != nullptr;

        if (!isRayPayload)
            continue;

        // Check if the struct is empty (has no fields)
        if (structType->getFields().begin() != structType->getFields().end())
            continue;

        emptyRayPayloadStructs.add(structType);
    }

    // Now process the collected structs
    for (auto structType : emptyRayPayloadStructs)
    {
        // Add a dummy field to the empty ray payload struct
        // Insert the key BEFORE the struct type so it's defined before being referenced
        builder.setInsertBefore(structType);
        auto dummyKey = builder.createStructKey();
        builder.addNameHintDecoration(dummyKey, UnownedStringSlice("_slang_dummy"));

        // Add stage access decorations that ray payload fields require
        addDefaultPayloadAccessQualifiersToField(builder, dummyKey);

        builder.createStructField(structType, dummyKey, builder.getIntType());

        // Now find and update all makeStruct instructions that create this struct type.
        List<IRInst*> makeStructsToUpdate;
        for (auto use = structType->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (user->getOp() == kIROp_MakeStruct && user->getDataType() == structType)
            {
                makeStructsToUpdate.add(user);
            }
        }

        for (auto makeStructInst : makeStructsToUpdate)
        {
            builder.setInsertBefore(makeStructInst);
            auto defaultValue = builder.getIntValue(builder.getIntType(), 0);
            auto newMakeStruct = builder.emitMakeStruct(structType, 1, &defaultValue);
            makeStructInst->replaceUsesWith(newMakeStruct);
            makeStructInst->removeAndDeallocate();
        }
    }
}

void legalizeRayTracingPrimitiveIDParam(IRModule* module)
{
    IRFunc* primitiveIndexFunc = nullptr;
    List<IRFunc*> entryPointsToProcess;

    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;

        auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
        if (!entryPointDecor)
            continue;

        switch (entryPointDecor->getProfile().getStage())
        {
        case Stage::Intersection:
        case Stage::AnyHit:
        case Stage::ClosestHit:
            break;
        default:
            continue;
        }

        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            continue;

        entryPointsToProcess.add(func);
    }

    for (auto func : entryPointsToProcess)
    {
        auto firstBlock = func->getFirstBlock();

        IRBuilder builder(module);
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());

        bool modifiedFuncType = false;
        for (auto param = firstBlock->getFirstParam(); param;)
        {
            auto nextParam = param->getNextParam();

            if (!isPrimitiveIDSystemValueParam(param))
            {
                param = nextParam;
                continue;
            }

            if (param->hasUses())
            {
                auto primitiveIndexCall = builder.emitCallInst(
                    builder.getUIntType(),
                    getRayTracingPrimitiveIndexFunc(module, primitiveIndexFunc),
                    0,
                    nullptr);

                IRInst* replacement = primitiveIndexCall;
                auto paramType = param->getFullType();
                auto valueType = paramType;
                if (auto borrowInParamType = as<IRBorrowInParamType>(valueType))
                    valueType = borrowInParamType->getValueType();

                if (valueType != builder.getUIntType())
                    replacement = builder.emitCast(valueType, primitiveIndexCall);

                traverseUses(
                    param,
                    [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        if (auto load = as<IRLoad>(user))
                        {
                            if (load->getPtr() == param)
                            {
                                load->replaceUsesWith(replacement);
                                load->removeAndDeallocate();
                                return;
                            }
                        }

                        builder.replaceOperand(use, replacement);
                    });
            }

            param->removeAndDeallocate();
            modifiedFuncType = true;
            param = nextParam;
        }

        if (modifiedFuncType)
            fixUpFuncType(func);
    }
}

} // namespace Slang
