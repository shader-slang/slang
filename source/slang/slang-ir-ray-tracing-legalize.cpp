// slang-ir-ray-tracing-legalize.cpp
// Record required payload boundaries before type legalization erases their logical data.
// Physical empty payloads are created only when type legalization proves that data is absent.
#include "slang-ir-ray-tracing-legalize.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-target-program.h"
#include "slang-target.h"

#include <spirv/unified1/spirv.h>

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
        addDefaultPayloadAccessQualifiersToField(builder, field->getKey());
}

static void addRayPayloadDecorationIfNeeded(IRBuilder& builder, IRType* type)
{
    if (!type->findDecoration<IRRayPayloadDecoration>())
        builder.addRayPayloadDecoration(type);
}

// Return whether this target requires field-level ray-payload access qualifiers.
static bool requiresPayloadAccessQualifiers(TargetProgram* targetProgram)
{
    auto targetRequest = targetProgram->getTargetReq();
    if (!isD3DTarget(targetRequest))
        return false;
    auto profile = getEffectiveTargetProfile(targetRequest, targetProgram->getOptionSet());
    return profile.getFamily() == ProfileFamily::DX &&
           profile.getVersion() >= ProfileVersion::DX_6_7;
}

// Record that a parameter must have a physical representation even if its data later disappears.
// For example, the native CallShader's second parameter is required, but an ordinary helper's
// `inout Empty` parameter is not. Type legalization uses this decoration only when that parameter
// legalizes to none; it does not change the source type or propagate padding through helper calls.
static void markRequiredPayloadParam(IRParam* param, bool isRayPayload)
{
    IRBuilder builder(param);
    if (auto decoration = param->findDecoration<IRRequiredRayTracingPayloadDecoration>())
    {
        SLANG_RELEASE_ASSERT((decoration->getIsRayPayload()->getValue() != 0) == isRayPayload);
        return;
    }
    builder.addDecoration(
        param,
        kIROp_RequiredRayTracingPayloadDecoration,
        builder.getIntValue(builder.getIntType(), isRayPayload));
}

// Resolve the temporary struct-conversion markers in calls, recursing through nested blocks.
// Consider this example:
//
//     int payload = 1;
//     TraceRay(scene, flags, mask, 0, 0, 0, ray, payload);
//
// The public TraceRay API accepts any payload type, but the HLSL intrinsic requires a user struct.
// ForceVarIntoRayPayloadStructTemporarily identifies that requirement after specialization. A
// struct argument is forwarded directly; other arguments receive the existing one-field wrapper
// and copy-in/copy-out operations. Both ray cases mark the native parameter as a required payload.
// The generic ForceVarIntoStructTemporarily marker requires a struct, but does not imply a payload.
// No emptiness decision is made here: if the argument is Empty[2], its temporary struct and array
// field can both disappear during type legalization, which then supplies the physical dummy.
static void legalizeForcedStructArgumentsInChildren(IRInst* inst)
{
    for (auto child : inst->getChildren())
    {
        switch (child->getOp())
        {
        case kIROp_Block:
            legalizeForcedStructArgumentsInChildren(child);
            break;

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

                    if (isForcedRayPayloadStruct)
                    {
                        auto callee = cast<IRFunc>(call->getCallee());
                        auto param = getParamAt(callee->getFirstBlock(), i);
                        SLANG_RELEASE_ASSERT(param);
                        markRequiredPayloadParam(param, true);
                    }

                    auto forceStructArg = arg->getOperand(0);
                    auto forceStructPtrType = as<IRPtrTypeBase>(forceStructArg->getDataType());
                    SLANG_RELEASE_ASSERT(forceStructPtrType);
                    auto forceStructBaseType = forceStructPtrType->getValueType();
                    IRBuilder builder(call);
                    if (forceStructBaseType->getOp() == kIROp_StructType)
                    {
                        call->setArg(i, forceStructArg);
                        if (isForcedRayPayloadStruct)
                            addRayPayloadDecorationIfNeeded(builder, forceStructBaseType);
                        continue;
                    }

                    // A non-struct argument needs a temporary one-field struct. Copy the value in,
                    // substitute the temporary for the marker, and copy it back for mutable args.
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
                    auto dataField = builder.createStructField(
                        structType,
                        elementBufferKey,
                        forceStructBaseType);

                    builder.setInsertBefore(call);
                    auto structVar = builder.emitVar(structType);
                    builder.addNameHintDecoration(structVar, UnownedStringSlice(varNameHint));
                    builder.emitStore(
                        builder.emitFieldAddress(
                            builder.getPtrType(dataField->getFieldType()),
                            structVar,
                            dataField->getKey()),
                        builder.emitLoad(forceStructArg));

                    arg->replaceUsesWith(structVar);
                    arg->removeAndDeallocate();

                    auto calleeType = as<IRFuncType>(call->getCallee()->getDataType());
                    SLANG_RELEASE_ASSERT(calleeType && i < calleeType->getParamCount());
                    auto argType = calleeType->getParamType(i);
                    if (!isPtrLikeOrHandleType(argType))
                        continue;

                    builder.setInsertAfter(call);
                    builder.emitStore(
                        forceStructArg,
                        builder.emitLoad(builder.emitFieldAddress(
                            builder.getPtrType(dataField->getFieldType()),
                            structVar,
                            dataField->getKey())));
                }
                break;
            }
        }
    }
}

// Mark the receiving payload from a D3D entry-point signature. Consider a separately compiled
// `[shader("miss")] void missMain(inout Empty payload)`: it has no TraceRay call whose marker could
// identify this parameter. Parameter binding classifies mutable hit/miss parameters as ray payloads
// and mutable callable parameters as callable data. Record the same roles here, before erasure
// would make their original parameter positions unavailable. Immutable hit attributes are not
// payloads. Nonempty ray-payload structs also need their existing access-qualifier normalization.
static void markD3DEntryPointPayloads(IRFunc* func)
{
    auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecor)
        return;

    bool isRayPayload;
    switch (entryPointDecor->getProfile().getStage())
    {
    case Stage::AnyHit:
    case Stage::ClosestHit:
    case Stage::Miss:
        isRayPayload = true;
        break;
    case Stage::Callable:
        isRayPayload = false;
        break;
    default:
        return;
    }

    IRBuilder builder(func);
    for (auto param : func->getParams())
    {
        auto outType = as<IROutParamTypeBase>(param->getFullType());
        if (!outType)
            continue;
        markRequiredPayloadParam(param, isRayPayload);
        if (isRayPayload)
        {
            if (auto structType = as<IRStructType>(outType->getValueType()))
                addRayPayloadDecorationIfNeeded(builder, structType);
        }
    }
}

// Fill in missing SM 6.7+ read/write qualifiers without replacing explicit qualifiers.
// Empty physical payload types are created later and receive these defaults at their creation.
static void legalizeRayPayloadAccessQualifiersForD3D(IRModule* module)
{
    List<IRStructType*> rayPayloadStructs;
    for (auto globalInst : module->getGlobalInsts())
    {
        auto structType = as<IRStructType>(globalInst);
        if (structType && structType->findDecoration<IRRayPayloadDecoration>())
            rayPayloadStructs.add(structType);
    }

    IRBuilder builder(module);
    for (auto structType : rayPayloadStructs)
        addDefaultPayloadAccessQualifiersToStruct(builder, structType);
}

void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram)
{
    // Khronos payload globals already carry their ray/callable role decorations. CUDA/OptiX
    // permits an empty callable argument to disappear, so neither target needs D3D slot markers.
    if (!isD3DTarget(targetProgram->getTargetReq()))
        return;

    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;

        legalizeForcedStructArgumentsInChildren(func);
        markD3DEntryPointPayloads(func);

        // The HLSL target arm is a native intrinsic, not an ordinary helper body. KnownBuiltin
        // identifies its mandatory second argument without relying on the emitted spelling.
        if (getBuiltinFuncEnum(func) == KnownBuiltinDeclName::CallShader)
        {
            auto funcType = func->getDataType();
            SLANG_RELEASE_ASSERT(funcType->getParamCount() == 2);
            auto payloadParam = getParamAt(func->getFirstBlock(), 1);
            SLANG_RELEASE_ASSERT(payloadParam);
            markRequiredPayloadParam(payloadParam, false);
        }
    }

    if (requiresPayloadAccessQualifiers(targetProgram))
        legalizeRayPayloadAccessQualifiersForD3D(module);
}

IRStructType* createEmptyRayTracingPayloadType(
    IRModule* module,
    TargetProgram* targetProgram,
    bool isRayPayload)
{
    auto targetRequest = targetProgram->getTargetReq();
    SLANG_RELEASE_ASSERT(isD3DTarget(targetRequest) || isKhronosTarget(targetRequest));

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto type = builder.createStructType();
    builder.addNameHintDecoration(
        type,
        UnownedStringSlice(isRayPayload ? "EmptyRayPayload" : "EmptyCallablePayload"));
    builder.addDecoration(
        type,
        kIROp_EmptyRayTracingPayloadDecoration,
        builder.getIntValue(builder.getIntType(), isRayPayload));

    // Direct SPIR-V permits OpTypeStruct with no members. GLSL does not, including when it is
    // used as an intermediate for SPIR-V, and D3D receiving shaders require a nonempty payload.
    // The decoration identifies this target-required representation to subsequent legalization
    // passes: unlike the original logical Empty type, it must not be erased again.
    if (!targetProgram->shouldEmitSPIRVDirectly())
    {
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, UnownedStringSlice("_slang_dummy"));
        builder.createStructField(type, key, builder.getIntType());
    }

    if (isRayPayload && isD3DTarget(targetRequest))
    {
        addRayPayloadDecorationIfNeeded(builder, type);
        if (requiresPayloadAccessQualifiers(targetProgram))
            addDefaultPayloadAccessQualifiersToStruct(builder, type);
    }
    return type;
}

Index getSPIRVRayTracingPayloadOperandIndex(IRSPIRVAsmInst* inst, bool& isRayPayload)
{
    // __truncate is an assembly pseudo-instruction with no numeric SPIR-V opcode.
    if (inst->getOpcodeOperand()->getOp() == kIROp_SPIRVAsmOperandTruncate)
        return -1;

    Index payloadIndex;
    isRayPayload = true;
    switch (inst->getOpcodeOperandWord())
    {
    case SpvOpTraceRayKHR:
        payloadIndex = 11;
        break;
    case SpvOpExecuteCallableKHR:
        isRayPayload = false;
        payloadIndex = 2;
        break;
    case SpvOpTraceRayMotionNV:
    case SpvOpHitObjectTraceRayNV:
    case SpvOpHitObjectTraceRayEXT:
    case SpvOpHitObjectTraceReorderExecuteEXT:
        payloadIndex = 12;
        break;
    case SpvOpHitObjectTraceRayMotionNV:
    case SpvOpHitObjectTraceRayMotionEXT:
    case SpvOpHitObjectTraceMotionReorderExecuteEXT:
        payloadIndex = 13;
        break;
    case SpvOpHitObjectExecuteShaderNV:
    case SpvOpHitObjectExecuteShaderEXT:
    case SpvOpHitObjectReorderExecuteShaderEXT:
        payloadIndex = 2;
        break;
    default:
        return -1;
    }

    // The IR includes the opcode as operand zero. These instructions have no result operands;
    // some reorder operations allow optional Hint/Bits operands after their mandatory payload.
    SLANG_RELEASE_ASSERT(inst->getOperandCount() >= UInt(payloadIndex + 1));
    return payloadIndex;
}

} // namespace Slang
