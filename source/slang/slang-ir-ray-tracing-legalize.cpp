// slang-ir-ray-tracing-legalize.cpp
// Preserve required ray-tracing storage without changing the program's logical data types.
// First identify payload parameters and globals, then give empty ones a physical carrier. Its
// logical data field is left to ordinary type legalization; only a dummy int survives. This makes
// caller and receiver layouts independent of which other types happen to be payloads in a module.
#include "slang-ir-ray-tracing-legalize.h"

#include "slang-compiler.h"
#include "slang-ir-dce.h"
#include "slang-ir-inline.h"
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

// A boundary-only representation of an empty logical payload. The data field lets existing IR
// keep using the original type until general type legalization erases that field and its accesses.
struct EmptyPayloadCarrier
{
    IRStructType* type;
    IRStructKey* dataKey;
};

// Own the per-module carrier cache and the boundary objects that need storage. Selection is by
// use, not by type: padding Empty itself would also change Data in this valid program:
//
//     struct Empty {}
//     struct Data { Empty unused; uint value; }
//     Empty empty;
//     Data data;
//     CallShader(0, empty);
//     CallShader(1, data);
//
// A separately compiled Data receiver must have the same layout even without the first call.
struct EmptyPayloadLegalizationContext
{
    IRModule* module;
    Dictionary<IRType*, EmptyPayloadCarrier> carriers;
    HashSet<IRParam*> d3dParams;
    HashSet<IRParam*> d3dRayParams;
    HashSet<IRGlobalVar*> khronosGlobals;

    // Return a shared wrapper for this logical type without modifying it or its constructors.
    // Arrays are valid logical payloads too; the wrapper supplies the outer struct required by D3D.
    EmptyPayloadCarrier getCarrier(IRType* logicalType)
    {
        EmptyPayloadCarrier carrier;
        if (carriers.tryGetValue(logicalType, carrier))
            return carrier;

        IRBuilder builder(module);
        builder.setInsertAfter(logicalType);
        carrier.dataKey = builder.createStructKey();
        builder.addNameHintDecoration(carrier.dataKey, UnownedStringSlice("_slang_data"));
        auto dummyKey = builder.createStructKey();
        builder.addNameHintDecoration(dummyKey, UnownedStringSlice("_slang_dummy"));
        carrier.type = builder.createStructType();
        auto nameHint = logicalType->findDecoration<IRNameHintDecoration>();
        builder.addNameHintDecoration(
            carrier.type,
            nameHint ? nameHint->getName() : UnownedStringSlice("EmptyPayload"));
        builder.createStructField(carrier.type, carrier.dataKey, logicalType);
        builder.createStructField(carrier.type, dummyKey, builder.getIntType());
        carriers.add(logicalType, carrier);
        return carrier;
    }

    // Select only mutable empty payload parameters, retaining whether D3D ray qualifiers apply.
    void collectParam(IRParam* param, bool isRayPayload)
    {
        auto ptrType = as<IROutParamTypeBase>(param->getDataType());
        if (!ptrType || !isEmptyType(ptrType->getValueType()))
            return;
        d3dParams.add(param);
        if (isRayPayload)
            d3dRayParams.add(param);
    }

    // Select a decorated Khronos carrier by its logical contents, including root arrays.
    void collectGlobal(IRGlobalVar* global)
    {
        auto ptrType = cast<IRPtrTypeBase>(global->getDataType());
        if (isEmptyType(ptrType->getValueType()))
            khronosGlobals.add(global);
    }

    // Expose dispatch operands in small SPIR-V helpers that receive this payload global, such as
    // HitObject's trace wrapper. Reuse the normal intrinsic inliner's eligibility test, but leave
    // unrelated intrinsics at their normal pipeline stage. Return whether a helper was inlined.
    bool inlinePayloadIntrinsicCalls(IRGlobalVar* global)
    {
        HashSet<IRCall*> calls;
        for (auto use = global->firstUse; use; use = use->nextUse)
        {
            if (auto call = as<IRCall>(use->getUser()))
                calls.add(call);
        }
        bool changed = false;
        for (auto call : calls)
            changed |= inlineIntrinsicFunctionCall(call);
        return changed;
    }

    // Retype a D3D boundary parameter and adapt all calls to the same intrinsic signature. The
    // standard-library intrinsic has a GenericAsm body that implicitly consumes its parameters;
    // entry points instead access the logical data field in their ordinary shader code.
    void materializeParam(IRParam* param)
    {
        auto func = getParentFunc(param);
        auto paramIndex = getParamIndexInBlock(param);
        auto oldPtrType = cast<IRPtrTypeBase>(param->getDataType());
        auto logicalType = oldPtrType->getValueType();
        auto carrier = getCarrier(logicalType);
        IRBuilder builder(module);
        if (d3dRayParams.contains(param))
            addRayPayloadDecorationIfNeeded(builder, carrier.type);

        List<IRUse*> oldUses;
        for (auto use = param->firstUse; use; use = use->nextUse)
            oldUses.add(use);
        param->setFullType(builder.getPtrType(carrier.type, oldPtrType));
        if (oldUses.getCount())
        {
            builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
            auto dataAddress = builder.emitFieldAddress(oldPtrType, param, carrier.dataKey);
            for (auto use : oldUses)
                use->set(dataAddress);
        }

        // The instantiated intrinsic may be called from several shaders. Updating only the call
        // that identified it would leave the other calls with a mismatched parameter type.
        List<IRCall*> calls;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            if (call && call->getCallee() == func)
                calls.add(call);
        }
        for (auto call : calls)
        {
            auto arg = call->getArg(paramIndex);
            builder.setInsertBefore(call);
            auto carrierVar = builder.emitVar(carrier.type);
            IRInst* fields[] = {
                builder.emitLoad(arg),
                builder.getIntValue(builder.getIntType(), 0)};
            builder.emitStore(carrierVar, builder.emitMakeStruct(carrier.type, 2, fields));
            call->setArg(paramIndex, carrierVar);
            builder.setInsertAfter(call);
            auto data = builder.emitFieldAddress(
                builder.getPtrType(logicalType),
                carrierVar,
                carrier.dataKey);
            builder.emitStore(arg, builder.emitLoad(data));
        }
        fixUpFuncType(func);
    }

    // Give a Khronos payload global physical storage. Dispatch operands and location queries
    // must still name that global, while source-level loads/stores use its empty logical field.
    // Payload-helper inlining exposes SPIR-V dispatch operands before this rewrite.
    void materializeGlobal(IRGlobalVar* global)
    {
        auto oldPtrType = cast<IRPtrTypeBase>(global->getDataType());
        auto carrier = getCarrier(oldPtrType->getValueType());
        List<IRUse*> logicalUses;
        List<IRInst*> asmOperands;
        for (auto use = global->firstUse; use; use = use->nextUse)
        {
            switch (use->getUser()->getOp())
            {
            case kIROp_SPIRVAsmOperandInst:
                asmOperands.add(use->getUser());
                break;
            case kIROp_GetVulkanRayTracingPayloadLocation:
            case kIROp_DependsOnDecoration:
                break;
            default:
                logicalUses.add(use);
                break;
            }
        }
        IRBuilder builder(module);
        global->setFullType(builder.getPtrType(carrier.type, oldPtrType));
        // An asm operand is a typed reference to its value. Keep both sides consistent or the
        // operand's stale empty pointer type would itself legalize to none inside the asm block.
        for (auto operand : asmOperands)
            operand->setFullType(global->getFullType());
        for (auto use : logicalUses)
        {
            builder.setInsertBefore(use->getUser());
            use->set(builder.emitFieldAddress(oldPtrType, global, carrier.dataKey));
        }
    }
};

// Resolve the temporary markers in `inst`'s calls, recursing through nested blocks. Consider this
// example:
//
//     int payload = 1;
//     TraceRay(scene, flags, mask, 0, 0, 0, ray, payload);
//
// The public TraceRay API accepts any `payload_t`, but the HLSL intrinsic requires its payload to
// be a user struct. The specialized wrapper therefore passes `payload` through
// ForceVarIntoRayPayloadStructTemporarily. If `payload_t` is a nonempty struct, this pass forwards
// the original variable and marks its type with `IRRayPayloadDecoration` so later payload passes
// can find it. Otherwise, as in the example, it creates a local `struct { int data; }`, copies the
// value in, passes the wrapper, and copies the field back after the call when the intrinsic
// parameter is mutable. ForceVarIntoStructTemporarily performs the same two rewrites for other
// struct-only parameters without adding ray-payload semantics. Specialization must expose the
// concrete argument type before this choice, and no marker may remain when HLSL emission begins.
// Empty structs and arrays instead select the intrinsic parameter for boundary-only
// materialization.
static void legalizeForcedStructArgumentsInChildren(
    IRInst* inst,
    EmptyPayloadLegalizationContext& context)
{
    for (auto child : inst->getChildren())
    {
        switch (child->getOp())
        {
        case kIROp_Block:
            legalizeForcedStructArgumentsInChildren(child, context);
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
                    if (isEmptyType(forceStructBaseType))
                    {
                        // Empty data needs a boundary-only carrier, even when it is an array.
                        // Select the formal parameter so every call to this instantiated
                        // intrinsic is adapted together when its signature changes.
                        call->setArg(i, forceStructArg);
                        auto callee = cast<IRFunc>(call->getCallee());
                        UInt paramIndex = 0;
                        for (auto param : callee->getParams())
                        {
                            if (paramIndex++ == i)
                            {
                                context.collectParam(param, isForcedRayPayloadStruct);
                                break;
                            }
                        }
                        continue;
                    }
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

// Mark payload types inferred from a D3D entry-point signature. For example, a separately compiled
// `[shader("miss")] void missMain(inout EmptyOuter payload)` has no TraceRay call whose temporary
// marker could identify EmptyOuter as a payload. Parameter binding treats mutable parameters of
// hit and miss shaders as ray payloads; record that same role here so collection and SM 6.7 access
// qualifiers also cover these entry points without requiring an explicit [raypayload] attribute.
static void markD3DEntryPointRayPayloadTypes(IRFunc* func, EmptyPayloadLegalizationContext& context)
{
    auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecor)
        return;
    switch (entryPointDecor->getProfile().getStage())
    {
    case Stage::AnyHit:
    case Stage::ClosestHit:
    case Stage::Miss:
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
        context.collectParam(param, true);
        auto structType = as<IRStructType>(outType->getValueType());
        if (structType)
            addRayPayloadDecorationIfNeeded(builder, structType);
    }
}

static void prepareD3DRayTracingPayloads(IRModule* module, EmptyPayloadLegalizationContext& context)
{
    // Establish payload roles from both caller arguments and receiving entry-point parameters
    // before collection or access-qualifier normalization inspects the types.
    // Both marker opcodes are part of the same HLSL ray-tracing support path. The ray-payload
    // variant also records semantic payload identity; retaining the generic variant here preserves
    // the established lowering contract for callers of the adjacent core-module helper.
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;
        legalizeForcedStructArgumentsInChildren(func, context);
        markD3DEntryPointRayPayloadTypes(func, context);
    }
}

// Collect an empty Khronos ray-payload global by its semantic decoration. For example, the wrapper
// that calls __spirvTraceRayHitObjectEXT declares `[__vulkanRayPayload] static T p;` and passes p
// to the intrinsic. Lowering attaches IRVulkanRayPayloadDecoration to that global, so the
// global-variable check below finds its payload type without checking the intrinsic's name or
// looking for a call.
static void collectIfEmptyKhronosRayPayload(
    IRInst* globalInst,
    EmptyPayloadLegalizationContext& context)
{
    auto globalVar = as<IRGlobalVar>(globalInst);
    if (!globalVar || (!globalVar->findDecoration<IRVulkanRayPayloadDecoration>() &&
                       !globalVar->findDecoration<IRVulkanRayPayloadInDecoration>()))
        return;
    context.collectGlobal(globalVar);
}

// Collect the empty D3D callable-data parameters of a single function. A D3D callable
// entry point has a fixed-shape mutable callable-data parameter, and a `CallShader` payload is the
// second, pointer-typed argument of the call; `KnownBuiltin` gives this target-neutral IR pass a
// stable identity for `CallShader` independent of the eventual intrinsic spelling.
static void collectIfEmptyD3DCallableData(
    IRInst* globalInst,
    EmptyPayloadLegalizationContext& context)
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
            context.collectParam(param, false);
    }

    // Specialization leaves the HLSL CallShader intrinsic as a concrete function. Select its
    // payload parameter, then adapt all calls to that signature during materialization.
    if (getBuiltinFuncEnum(func) == KnownBuiltinDeclName::CallShader)
    {
        UInt paramIndex = 0;
        for (auto param : func->getParams())
        {
            if (paramIndex++ == 1)
                context.collectParam(param, false);
        }
        SLANG_RELEASE_ASSERT(paramIndex == 2);
    }
}

// Collect an empty Khronos callable-data global. Vulkan-style
// `CallShader` lowering stores callable data in a decorated module-scope global; the intrinsic call
// may already have been inlined by this point, so the global is the canonical surviving carrier.
static void collectIfEmptyKhronosCallableData(
    IRInst* globalInst,
    EmptyPayloadLegalizationContext& context)
{
    auto globalVar = as<IRGlobalVar>(globalInst);
    if (!globalVar)
        return;
    if (!globalVar->findDecoration<IRVulkanCallablePayloadDecoration>() &&
        !globalVar->findDecoration<IRVulkanCallablePayloadInDecoration>())
    {
        return;
    }
    context.collectGlobal(globalVar);
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

// Select the physical boundary forms required by a target and its shader model.
struct RayTracingPayloadLegalizationPolicy
{
    bool prepareD3DRayTracingPayloads = false;
    bool materializeEmptyKhronosRayPayloads = false;
    bool materializeEmptyD3DCallableData = false;
    bool materializeEmptyKhronosCallableData = false;
    bool inlineSPIRVPayloadIntrinsics = false;
    bool normalizeD3DPayloadAccessQualifiers = false;
};

static RayTracingPayloadLegalizationPolicy getRayTracingPayloadLegalizationPolicy(
    TargetProgram* targetProgram)
{
    RayTracingPayloadLegalizationPolicy policy;
    auto targetRequest = targetProgram->getTargetReq();

    if (isD3DTarget(targetRequest))
    {
        policy.prepareD3DRayTracingPayloads = true;
        policy.materializeEmptyD3DCallableData = true;

        auto profile = getEffectiveTargetProfile(targetRequest, targetProgram->getOptionSet());
        policy.normalizeD3DPayloadAccessQualifiers = profile.getFamily() == ProfileFamily::DX &&
                                                     profile.getVersion() >= ProfileVersion::DX_6_7;
    }
    else if (isKhronosTarget(targetRequest))
    {
        // Both SPIR-V and GLSL lower ray payloads and CallShader callable data through decorated
        // module-scope objects that must keep a physical representation.
        policy.materializeEmptyKhronosRayPayloads = true;
        policy.materializeEmptyKhronosCallableData = true;
        policy.inlineSPIRVPayloadIntrinsics = isSPIRV(targetRequest->getTarget());
    }

    // CUDA/OptiX is intentionally absent. Its callable ABI is variadic, so ordinary empty-type
    // legalization may erase an empty callable-data argument and parameter.
    return policy;
}

// Collect boundary objects in a single global walk. Separate ray and callable collectors share
// one context so repeated uses of a logical type receive the same boundary representation.
static void collectEmptyPayloads(
    IRModule* module,
    RayTracingPayloadLegalizationPolicy const& policy,
    EmptyPayloadLegalizationContext& context)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (policy.materializeEmptyKhronosRayPayloads)
            collectIfEmptyKhronosRayPayload(globalInst, context);
        if (policy.materializeEmptyD3DCallableData)
            collectIfEmptyD3DCallableData(globalInst, context);
        if (policy.materializeEmptyKhronosCallableData)
            collectIfEmptyKhronosCallableData(globalInst, context);
    }
}

void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram)
{
    const auto policy = getRayTracingPayloadLegalizationPolicy(targetProgram);
    EmptyPayloadLegalizationContext context;
    context.module = module;

    // Identify D3D ray-payload boundaries from both call-site markers and entry-point parameters,
    // including unannotated payload types and independently compiled receiving shaders.
    if (policy.prepareD3DRayTracingPayloads)
        prepareD3DRayTracingPayloads(module, context);

    if (policy.materializeEmptyKhronosRayPayloads || policy.materializeEmptyD3DCallableData ||
        policy.materializeEmptyKhronosCallableData)
    {
        collectEmptyPayloads(module, policy, context);
        for (auto param : context.d3dParams)
            context.materializeParam(param);
        bool inlinedPayloadIntrinsic = false;
        for (auto global : context.khronosGlobals)
        {
            if (policy.inlineSPIRVPayloadIntrinsics)
                inlinedPayloadIntrinsic |= context.inlinePayloadIntrinsicCalls(global);
            context.materializeGlobal(global);
        }
        if (inlinedPayloadIntrinsic)
        {
            // Remove obsolete helper definitions: their old empty formal parameters would
            // still be visited by type legalization. Do this after all selected globals have
            // been rewritten, since DCE can also remove globals that no longer have uses.
            eliminateDeadCode(module);
        }
    }

    if (policy.normalizeD3DPayloadAccessQualifiers)
        legalizeRayPayloadAccessQualifiersForD3D(module);
}

} // namespace Slang
