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

// Check whether the struct has no fields before type legalization. This misses transitively empty
// carriers: for `struct Outer { Empty inner; }`, where Empty has no fields, legalization removes
// inner and then Outer. Such a payload still loses its required argument or parameter on D3D,
// or its required global on Khronos. Fixing that requires preserving the carrier when its fields
// legalize to `none`; this immediate-field check does not establish that it will survive.
static bool isZeroFieldStruct(IRStructType* structType)
{
    return !structType->getFields().getFirst();
}

static void addIfEmptyStruct(IRType* type, HashSet<IRStructType*>& set)
{
    auto structType = as<IRStructType>(type);
    if (structType && isZeroFieldStruct(structType))
        set.add(structType);
}

// Give a zero-field struct a legal one-field physical layout and update its constructors. The
// target policy and semantic collectors decide which structs need a physical representation; this
// helper only performs the shared mechanical rewrite.
static void padEmptyStructWithDummyField(IRBuilder& builder, IRStructType* structType)
{
    SLANG_RELEASE_ASSERT(isZeroFieldStruct(structType));

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

// Resolve the temporary markers in `inst`'s calls, recursing through nested blocks. Consider this
// example:
//
//     int payload = 1;
//     TraceRay(scene, flags, mask, 0, 0, 0, ray, payload);
//
// The public TraceRay API accepts any `payload_t`, but the HLSL intrinsic requires its payload to
// be a user struct. The specialized wrapper therefore passes `payload` through
// ForceVarIntoRayPayloadStructTemporarily. If `payload_t` is already a struct, this pass forwards
// the original variable and marks its type with `IRRayPayloadDecoration` so later payload passes
// can find it. Otherwise, as in the example, it creates a local `struct { int data; }`, copies the
// value in, passes the wrapper, and copies the field back after the call when the intrinsic
// parameter is mutable. ForceVarIntoStructTemporarily performs the same two rewrites for other
// struct-only parameters without adding ray-payload semantics. Specialization must expose the
// concrete argument type before this choice, and no marker may remain when HLSL emission begins.
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

// Collect the empty ray-payload struct reachable from a single global inst, if any. Identify ray
// payloads by their IR decorations: either the struct type is decorated, or it is the pointee type
// of a decorated global variable. For example, the standard-library wrapper that calls
// __spirvTraceRayHitObjectEXT declares `[__vulkanRayPayload] static T p;` and passes `p` to the
// intrinsic. Lowering attaches IRVulkanRayPayloadDecoration to that global, so the global-variable
// check below finds its payload type without checking the intrinsic's name or looking for a call.
static void collectIfEmptyRayPayload(
    IRInst* globalInst,
    HashSet<IRStructType*>& emptyRayPayloadStructs)
{
    if (auto structType = as<IRStructType>(globalInst))
    {
        if (structType->findDecoration<IRRayPayloadDecoration>() ||
            structType->findDecoration<IRVulkanRayPayloadDecoration>())
        {
            addIfEmptyStruct(structType, emptyRayPayloadStructs);
        }
        return;
    }

    auto globalVar = as<IRGlobalVar>(globalInst);
    if (!globalVar || !globalVar->findDecoration<IRVulkanRayPayloadDecoration>())
        return;
    auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
    SLANG_RELEASE_ASSERT(ptrType);
    addIfEmptyStruct(ptrType->getValueType(), emptyRayPayloadStructs);
}

static bool isCallShaderCall(IRCall* call)
{
    return getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::CallShader;
}

// Collect the empty D3D callable-data structs reachable from a single global inst. A D3D callable
// entry point has a fixed-shape mutable callable-data parameter, and a `CallShader` payload is the
// second, pointer-typed argument of the call; `KnownBuiltin` gives this target-neutral IR pass a
// stable identity for `CallShader` independent of the eventual intrinsic spelling.
static void collectIfEmptyD3DCallableData(
    IRInst* globalInst,
    HashSet<IRStructType*>& emptyCallableDataStructs)
{
    auto func = as<IRFunc>(globalInst);
    if (!func)
        return;

    // Find the callable entry point's mutable data parameter, for example `data` in
    // `[shader("callable")] void callableMain(inout Data data)`. This parameter receives the
    // second argument of CallShader; the shader index is not a parameter of the entry point.
    auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
    if (entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Callable)
    {
        for (auto param : func->getParams())
        {
            if (auto outType = as<IROutParamTypeBase>(param->getFullType()))
                addIfEmptyStruct(outType->getValueType(), emptyCallableDataStructs);
        }
    }

    // Find the caller's payload type in `CallShader(shaderIndex, data)`. Its second argument is
    // the pointer to `data`, whose pointee struct must survive type legalization.
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

// Collect the empty Khronos callable-data struct reachable from a single global inst. Vulkan-style
// `CallShader` lowering stores callable data in a decorated module-scope global; the intrinsic call
// may already have been inlined by this point, so the global is the canonical surviving carrier.
static void collectIfEmptyKhronosCallableData(
    IRInst* globalInst,
    HashSet<IRStructType*>& emptyCallableDataStructs)
{
    auto globalVar = as<IRGlobalVar>(globalInst);
    if (!globalVar)
        return;
    if (!globalVar->findDecoration<IRVulkanCallablePayloadDecoration>() &&
        !globalVar->findDecoration<IRVulkanCallablePayloadInDecoration>())
    {
        return;
    }
    auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
    SLANG_RELEASE_ASSERT(ptrType);
    addIfEmptyStruct(ptrType->getValueType(), emptyCallableDataStructs);
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
        // Both SPIR-V and GLSL lower ray payloads and CallShader callable data through decorated
        // module-scope objects that must keep a physical representation.
        policy.materializeEmptyRayPayloads = true;
        policy.materializeEmptyKhronosCallableData = true;
    }

    // CUDA/OptiX is intentionally absent. Its callable ABI is variadic, so ordinary empty-type
    // legalization may erase an empty callable-data argument and parameter.
    return policy;
}

// Collect every empty payload struct the policy asks for in a single walk of the module's global
// insts. The payload kinds inspect different global-inst shapes, so we dispatch each inst to the
// per-kind collectors the policy enables instead of walking the global list once per kind. All
// three collectors feed one shared set: a struct reused as both a ray payload and callable data
// must be padded exactly once, and the set deduplicates it (padding it from two sets would trip
// `padEmptyStructWithDummyField`'s empty-struct assert on the second pass).
static void collectEmptyPayloadStructs(
    IRModule* module,
    RayTracingPayloadLegalizationPolicy const& policy,
    HashSet<IRStructType*>& emptyPayloadStructs)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (policy.materializeEmptyRayPayloads)
            collectIfEmptyRayPayload(globalInst, emptyPayloadStructs);
        if (policy.materializeEmptyD3DCallableData)
            collectIfEmptyD3DCallableData(globalInst, emptyPayloadStructs);
        if (policy.materializeEmptyKhronosCallableData)
            collectIfEmptyKhronosCallableData(globalInst, emptyPayloadStructs);
    }
}

void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram)
{
    const auto policy = getRayTracingPayloadLegalizationPolicy(targetProgram);

    // Resolve the frontend marker before collecting ray payloads. For an unannotated empty struct,
    // this step is what applies IRRayPayloadDecoration; collecting first would miss the struct and
    // allow type legalization to erase it.
    if (policy.legalizeD3DForcedStructArguments)
        legalizeD3DForcedStructArguments(module);

    if (policy.materializeEmptyRayPayloads || policy.materializeEmptyD3DCallableData ||
        policy.materializeEmptyKhronosCallableData)
    {
        HashSet<IRStructType*> emptyPayloadStructs;
        collectEmptyPayloadStructs(module, policy, emptyPayloadStructs);
        padEmptyStructs(module, emptyPayloadStructs);
    }

    if (policy.normalizeD3DPayloadAccessQualifiers)
        legalizeRayPayloadAccessQualifiersForD3D(module);
}

} // namespace Slang
