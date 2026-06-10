// slang-ir-hlsl-legalize.cpp
#include "slang-ir-hlsl-legalize.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-type-system-shared.h"

#include <functional>

namespace Slang
{

static bool tryGetBarrierFlagValue(IRInst* inst, IRIntegerValue& outValue)
{
    if (auto intLit = as<IRIntLit>(getBarrierFlagValueInst(inst)))
    {
        outValue = getIntVal(intLit);
        return true;
    }
    return false;
}

static bool tryGetBarrierFlagValueOrDiagnose(
    IRInst* inst,
    DiagnosticSink* sink,
    IRIntegerValue& outValue)
{
    if (tryGetBarrierFlagValue(inst->getOperand(0), outValue))
        return true;

    sink->diagnose(Diagnostics::NeedCompileTimeConstant{.location = inst->sourceLoc});
    return false;
}

static String getBarrierFlagValueString(uint32_t flagVal)
{
    StringBuilder sb;
    sb << "0x" << String(flagVal, 16);
    return sb.produceString();
}

static bool isValidBarrierMemoryTypeFlags(uint32_t flagVal)
{
    const uint32_t knownFlags =
        BarrierMemoryTypeFlags::UavMemory | BarrierMemoryTypeFlags::GroupSharedMemory |
        BarrierMemoryTypeFlags::NodeInputMemory | BarrierMemoryTypeFlags::NodeOutputMemory;
    return flagVal == BarrierMemoryTypeFlags::AllMemory ||
           (flagVal != 0 && (flagVal & ~knownFlags) == 0);
}

static bool isValidBarrierSemanticFlags(uint32_t flagVal)
{
    const uint32_t knownFlags = BarrierSemanticFlags::GroupSync | BarrierSemanticFlags::GroupScope |
                                BarrierSemanticFlags::DeviceScope;
    return flagVal == BarrierSemanticFlags::Reorder || (flagVal & ~knownFlags) == 0;
}

static void validateBarrierFlagsForHLSLInst(IRInst* inst, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_GetEnumBarrierMemoryTypeFlags:
        {
            IRIntegerValue rawFlagVal = 0;
            if (!tryGetBarrierFlagValueOrDiagnose(inst, sink, rawFlagVal))
                break;

            auto flagVal = (uint32_t)rawFlagVal;
            if (!isValidBarrierMemoryTypeFlags(flagVal))
            {
                sink->diagnose(Diagnostics::InvalidBarrierMemoryTypeFlagsValue{
                    .value = getBarrierFlagValueString(flagVal),
                    .location = inst->sourceLoc});
            }
            break;
        }
    case kIROp_GetEnumBarrierSemanticFlags:
        {
            IRIntegerValue rawFlagVal = 0;
            if (!tryGetBarrierFlagValueOrDiagnose(inst, sink, rawFlagVal))
                break;

            auto flagVal = (uint32_t)rawFlagVal;
            if (!isValidBarrierSemanticFlags(flagVal))
            {
                sink->diagnose(Diagnostics::InvalidBarrierSemanticFlagsValue{
                    .value = getBarrierFlagValueString(flagVal),
                    .location = inst->sourceLoc});
            }
            break;
        }
    default:
        break;
    }

    for (auto child : inst->getChildren())
        validateBarrierFlagsForHLSLInst(child, sink);
}

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink)
{
    validateBarrierFlagsForHLSLInst(module->getModuleInst(), sink);
}

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

void legalizeRayPayloadAccessQualifiersForHLSL(IRModule* module)
{
    // Walk every `[raypayload]` struct in the module and fill in any missing per-side
    // PAQs. This is a structural pass keyed on `IRRayPayloadDecoration`, rather than a
    // call-site fixup, because the call-site PAQ fill in
    // `searchChildrenForForceVarIntoStructTemporarily` only fires when the frontend wraps
    // a payload argument with `__forceVarIntoRayPayloadStructTemporarily`, which it does
    // only around `TraceRay` / `HitObject::TraceRay` / `HitObject::Invoke` payload args.
    // A hit-shader-only translation unit (typical for per-stage-compiled, runtime-linked
    // shader libraries) has no such call, so a user-authored struct with one-sided PAQ
    // would keep its one-sided PAQ and be rejected by DXC at SM 6.7+.
    // Collect first: filling a struct's PAQs reaches `builder.getStringValue(...)` and
    // adds decorations, which inserts new global instructions and would invalidate a
    // live `getGlobalInsts()` walk (the same hazard documented in
    // `legalizeEmptyRayPayloadsForHLSL`).
    List<IRStructType*> rayPayloadStructs;
    for (auto globalInst : module->getGlobalInsts())
    {
        auto structType = as<IRStructType>(globalInst);
        if (!structType)
            continue;
        if (!structType->findDecoration<IRRayPayloadDecoration>())
            continue;
        rayPayloadStructs.add(structType);
    }

    IRBuilder builder(module);
    for (auto structType : rayPayloadStructs)
    {
        addDefaultPayloadAccessQualifiersToStruct(builder, structType);
    }
}

} // namespace Slang
