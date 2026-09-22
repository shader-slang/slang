// slang-ir-ray-tracing-legalize.cpp
#include "slang-ir-ray-tracing-legalize.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-target-program.h"
#include "slang-target.h"

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

// An empty struct has no fields and legalizes to `LegalType::Flavor::none`. A struct whose fields
// all legalize to `none` has the same eventual representation, but is deliberately not classified
// as empty here; handling that recursive case requires a separate type-legalization design.
static bool isEmptyStruct(IRStructType* structType)
{
    return !structType->getFields().getFirst();
}

static void addIfEmptyStruct(IRType* type, HashSet<IRStructType*>& set)
{
    auto structType = as<IRStructType>(type);
    if (structType && isEmptyStruct(structType))
        set.add(structType);
}

// Give an empty struct a legal one-field physical layout and update its constructors. The target
// policy and semantic collectors decide which structs need a physical representation; this helper
// only performs the shared mechanical rewrite.
static void padEmptyStructWithDummyField(IRBuilder& builder, IRStructType* structType)
{
    SLANG_RELEASE_ASSERT(isEmptyStruct(structType));

    // Insert the key before the struct type so it is defined before being referenced.
    builder.setInsertBefore(structType);
    auto dummyKey = builder.createStructKey();
    builder.addNameHintDecoration(dummyKey, UnownedStringSlice("_slang_dummy"));
    builder.createStructField(structType, dummyKey, builder.getIntType());

    // Collect first because replacing and removing a constructor invalidates the use walk.
    List<IRInst*> makeStructsToUpdate;
    for (auto use = structType->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (user->getOp() == kIROp_MakeStruct && user->getDataType() == structType)
            makeStructsToUpdate.add(user);
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
                        builder.emitFieldAddress(
                            builder.getPtrType(dataField->getFieldType()),
                            structVar,
                            dataField->getKey()));
                }
                break;
            }
        }
    }
}

static void legalizeD3DForcedStructArguments(IRModule* module)
{
    // Both marker opcodes are part of the same HLSL ray-tracing support path. The ray-payload
    // variant also records semantic payload identity; retaining the generic variant here preserves
    // the established lowering contract for callers of the adjacent core-module helper.
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (func)
            legalizeForcedStructArgumentsInChildren(func);
    }
}

static void collectEmptyRayPayloadStructs(
    IRModule* module,
    HashSet<IRStructType*>& emptyRayPayloadStructs)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto structType = as<IRStructType>(globalInst))
        {
            if (structType->findDecoration<IRRayPayloadDecoration>() ||
                structType->findDecoration<IRVulkanRayPayloadDecoration>())
            {
                addIfEmptyStruct(structType, emptyRayPayloadStructs);
            }
            continue;
        }

        // Built-ins such as `__spirvTraceRayHitObjectEXT` put the Vulkan payload decoration on a
        // global variable rather than on its struct type.
        auto globalVar = as<IRGlobalVar>(globalInst);
        if (!globalVar || !globalVar->findDecoration<IRVulkanRayPayloadDecoration>())
            continue;
        auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
        SLANG_RELEASE_ASSERT(ptrType);
        addIfEmptyStruct(ptrType->getValueType(), emptyRayPayloadStructs);
    }
}

static bool isCallShaderCall(IRCall* call)
{
    return getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::CallShader;
}

static void collectEmptyD3DCallableDataStructs(
    IRModule* module,
    HashSet<IRStructType*>& emptyCallableDataStructs)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;

        // A D3D callable entry point has a fixed-shape mutable callable-data parameter.
        auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
        if (entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Callable)
        {
            for (auto param : func->getParams())
            {
                if (auto outType = as<IROutParamTypeBase>(param->getFullType()))
                    addIfEmptyStruct(outType->getValueType(), emptyCallableDataStructs);
            }
        }

        // A CallShader payload is its second, pointer-typed argument. KnownBuiltin gives this
        // target-neutral IR pass a stable identity independent of the eventual intrinsic spelling.
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto call = as<IRCall>(inst);
                if (!call || !isCallShaderCall(call))
                    continue;
                SLANG_RELEASE_ASSERT(call->getArgCount() == 2);
                auto ptrType = as<IRPtrTypeBase>(call->getArg(1)->getDataType());
                SLANG_RELEASE_ASSERT(ptrType);
                addIfEmptyStruct(ptrType->getValueType(), emptyCallableDataStructs);
            }
        }
    }
}

static void collectEmptyKhronosCallableDataStructs(
    IRModule* module,
    HashSet<IRStructType*>& emptyCallableDataStructs)
{
    // Vulkan-style CallShader lowering stores callable data in a decorated module-scope global.
    // The intrinsic call may already have been inlined by this point, so the global is the
    // canonical surviving carrier to inspect.
    for (auto globalInst : module->getGlobalInsts())
    {
        auto globalVar = as<IRGlobalVar>(globalInst);
        if (!globalVar)
            continue;
        if (!globalVar->findDecoration<IRVulkanCallablePayloadDecoration>() &&
            !globalVar->findDecoration<IRVulkanCallablePayloadInDecoration>())
        {
            continue;
        }
        auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
        SLANG_RELEASE_ASSERT(ptrType);
        addIfEmptyStruct(ptrType->getValueType(), emptyCallableDataStructs);
    }
}

static void padEmptyStructs(IRModule* module, HashSet<IRStructType*> const& emptyStructs)
{
    IRBuilder builder(module);
    for (auto structType : emptyStructs)
        padEmptyStructWithDummyField(builder, structType);
}

static void legalizeRayPayloadAccessQualifiersForD3D(IRModule* module)
{
    // A hit-shader-only translation unit has no TraceRay call and therefore no forced-payload
    // marker to visit. Sweep every semantic ray-payload struct so those separately compiled
    // stages receive the same SM 6.7 access-qualifier normalization as a ray-generation module.
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

struct RayTracingPayloadLegalizationPolicy
{
    bool legalizeD3DForcedStructArguments = false;
    bool materializeEmptyRayPayloads = false;
    bool materializeEmptyD3DCallableData = false;
    bool materializeEmptyKhronosCallableData = false;
    bool normalizeD3DPayloadAccessQualifiers = false;
};

static RayTracingPayloadLegalizationPolicy getRayTracingPayloadLegalizationPolicy(
    TargetProgram* targetProgram)
{
    RayTracingPayloadLegalizationPolicy policy;
    auto targetRequest = targetProgram->getTargetReq();
    const auto target = targetRequest->getTarget();

    if (isD3DTarget(targetRequest))
    {
        policy.legalizeD3DForcedStructArguments = true;
        policy.materializeEmptyRayPayloads = true;
        policy.materializeEmptyD3DCallableData = true;

        auto profile = getEffectiveTargetProfile(targetRequest, targetProgram->getOptionSet());
        policy.normalizeD3DPayloadAccessQualifiers = profile.getFamily() == ProfileFamily::DX &&
                                                     profile.getVersion() >= ProfileVersion::DX_6_7;
    }
    else if (isKhronosTarget(targetRequest))
    {
        // SPIR-V output needs a physical ray-payload object. Both SPIR-V and GLSL lower CallShader
        // through a decorated module-scope callable-data object.
        policy.materializeEmptyRayPayloads = isSPIRV(target);
        policy.materializeEmptyKhronosCallableData = true;
    }

    // CUDA/OptiX is intentionally absent. Its callable ABI is variadic, so ordinary empty-type
    // legalization may erase an empty callable-data argument and parameter.
    return policy;
}

void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram)
{
    const auto policy = getRayTracingPayloadLegalizationPolicy(targetProgram);

    // Resolve the frontend marker before collecting ray payloads. For an unannotated empty struct,
    // this step is what applies IRRayPayloadDecoration; collecting first would miss the struct and
    // allow type legalization to erase it.
    if (policy.legalizeD3DForcedStructArguments)
        legalizeD3DForcedStructArguments(module);

    if (policy.materializeEmptyRayPayloads)
    {
        HashSet<IRStructType*> emptyRayPayloadStructs;
        collectEmptyRayPayloadStructs(module, emptyRayPayloadStructs);
        padEmptyStructs(module, emptyRayPayloadStructs);
    }

    if (policy.materializeEmptyD3DCallableData || policy.materializeEmptyKhronosCallableData)
    {
        HashSet<IRStructType*> emptyCallableDataStructs;
        if (policy.materializeEmptyD3DCallableData)
            collectEmptyD3DCallableDataStructs(module, emptyCallableDataStructs);
        if (policy.materializeEmptyKhronosCallableData)
            collectEmptyKhronosCallableDataStructs(module, emptyCallableDataStructs);
        padEmptyStructs(module, emptyCallableDataStructs);
    }

    if (policy.normalizeD3DPayloadAccessQualifiers)
        legalizeRayPayloadAccessQualifiersForD3D(module);
}

} // namespace Slang
