// slang-ir-hlsl-legalize.cpp
#include "slang-ir-hlsl-legalize.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util-hlsl.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static String getBarrierFlagValueString(uint32_t flagVal)
{
    StringBuilder sb;
    sb << "0x" << String(flagVal, 16);
    return sb.produceString();
}

static void validateBarrierFlagsForHLSLInst(IRInst* inst, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_GetEnumBarrierMemoryTypeFlags:
        {
            auto intLit = cast<IRIntLit>(getBarrierFlagValueInst(inst->getOperand(0)));
            auto rawFlagVal = getIntVal(intLit);
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
            auto intLit = cast<IRIntLit>(getBarrierFlagValueInst(inst->getOperand(0)));
            auto rawFlagVal = getIntVal(intLit);
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

static void validateBarrierFlagsForHLSLFunc(IRFunc* func, DiagnosticSink* sink)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
            validateBarrierFlagsForHLSLInst(inst, sink);
    }
}

void validateBarrierFlagsForHLSL(IRModule* module, DiagnosticSink* sink)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        switch (globalInst->getOp())
        {
        case kIROp_GetEnumBarrierMemoryTypeFlags:
        case kIROp_GetEnumBarrierSemanticFlags:
            validateBarrierFlagsForHLSLInst(globalInst, sink);
            break;
        case kIROp_Func:
            validateBarrierFlagsForHLSLFunc(as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            if (auto innerFunc = as<IRFunc>(findGenericReturnVal(as<IRGeneric>(globalInst))))
                validateBarrierFlagsForHLSLFunc(innerFunc, sink);
            break;
        default:
            break;
        }
    }
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

// A struct is "empty" when it has no fields. Type legalization erases such a struct (it legalizes
// to `LegalType::Flavor::none`), which is what deletes empty ray-payload / callable-data structs.
// The padding passes below all key on this single predicate so the collectors and the padder's
// precondition stay in sync. (A struct whose fields *all* themselves legalize to `none` also
// legalizes to `none` yet is not empty by this test — see the callable-data passes for that
// documented limitation.)
static bool isEmptyStruct(IRStructType* structType)
{
    return !structType->getFields().getFirst();
}

// Add `type` to `set` if it is an empty struct. Centralizes the "needs padding?" decision the
// padding passes share; skipping non-empty structs also makes the passes idempotent (an
// already-padded struct is non-empty and so is not re-collected on a re-run).
static void addIfEmptyStruct(IRType* type, HashSet<IRStructType*>& set)
{
    if (auto structType = as<IRStructType>(type); structType && isEmptyStruct(structType))
        set.add(structType);
}

// Give an empty struct a legal one-field physical layout by adding an `int _slang_dummy` field and
// rewriting every `MakeStruct` of the type to supply a zero for it. This is a mechanical transform
// that lets the struct survive type legalization, which would otherwise erase an empty struct; the
// target-specific reason each caller needs that survival lives at the call sites.
// `addPayloadAccessQualifiers` is set only for ray payloads, whose fields must carry HLSL payload
// access qualifiers at SM 6.7+; callable data is a plain `inout` and must not receive them.
static void padEmptyStructWithDummyField(
    IRBuilder& builder,
    IRStructType* structType,
    bool addPayloadAccessQualifiers)
{
    // Padding a struct that already has fields would silently corrupt its layout — out-of-contract
    // input, so fail loudly even in release. All current callers filter via `isEmptyStruct`, so
    // this is unreachable today; it guards a future caller added without that filter.
    SLANG_RELEASE_ASSERT(isEmptyStruct(structType));

    // Insert the key BEFORE the struct type so it is defined before being referenced.
    builder.setInsertBefore(structType);
    auto dummyKey = builder.createStructKey();
    builder.addNameHintDecoration(dummyKey, UnownedStringSlice("_slang_dummy"));

    if (addPayloadAccessQualifiers)
        addDefaultPayloadAccessQualifiersToField(builder, dummyKey);

    builder.createStructField(structType, dummyKey, builder.getIntType());

    // The (now non-empty) struct's `MakeStruct`s must supply a value for the new field. Collect
    // first, then mutate: `replaceUsesWith`/`removeAndDeallocate` would invalidate the use walk.
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
            addIfEmptyStruct(ptrType->getValueType(), emptyRayPayloadStructs);
            continue;
        }

        // Check if this struct has ray payload decoration
        auto rayPayloadDec = structType->findDecoration<IRRayPayloadDecoration>();
        auto vulkanRayPayloadDec = structType->findDecoration<IRVulkanRayPayloadDecoration>();
        bool isRayPayload = rayPayloadDec != nullptr || vulkanRayPayloadDec != nullptr;

        if (!isRayPayload)
            continue;

        addIfEmptyStruct(structType, emptyRayPayloadStructs);
    }

    // Now process the collected structs. Ray payload fields require stage access qualifiers.
    for (auto structType : emptyRayPayloadStructs)
    {
        padEmptyStructWithDummyField(builder, structType, /*addPayloadAccessQualifiers*/ true);
    }
}

// Return true if `call` invokes the HLSL `CallShader` target intrinsic. Recognition uses the
// canonical `findTargetIntrinsicDefinition` (which matches both the pre-link
// `[targetIntrinsic("CallShader")]` decoration and the post-link `IRGenericAsm("CallShader")` body
// the callee becomes), then compares the returned definition against the intrinsic's HLSL codegen
// string `"CallShader"`. The coupling to that string is intentional: on the D3D path — the only
// path that reaches this pass — the definition IS the HLSL `__intrinsic_asm` text. It deliberately
// does NOT match the CUDA arm (whose definition is `optixDirectCall<...>`, a different string);
// covering CUDA needs a target-agnostic identity (`KnownBuiltinDeclName`) and is tracked
// separately.
static bool isCallShaderCall(IRCall* call, CapabilitySet const& targetCaps)
{
    auto callee = getResolvedInstForDecorations(call->getCallee());
    UnownedStringSlice definition;
    IRInst* intrinsicInst = nullptr;
    if (!findTargetIntrinsicDefinition(callee, targetCaps, definition, intrinsicInst))
        return false;
    return definition == UnownedStringSlice("CallShader");
}

void legalizeEmptyCallableDataPayloadsForHLSL(IRModule* module, CapabilitySet targetCaps)
{
    // DXC requires a callable entry point to declare exactly one argument parameter, and a
    // `CallShader(index, payload)` to pass exactly two arguments. An empty callable-data struct
    // legalizes to `LegalType::Flavor::none`, so `legalizeResourceTypes` removes both the callable
    // parameter (leaving a zero-parameter callable) and the `CallShader` payload argument (leaving
    // `CallShader(index)`) — DXC rejects both. Pad the empty struct so it survives, as
    // `legalizeEmptyRayPayloadsForHLSL` does for ray payloads.
    IRBuilder builder(module);

    // On the D3D path the callable-data struct carries no decoration to key on, so identify it
    // structurally at its two use sites. Collect first, because padding inserts new global
    // instructions (struct keys) that would invalidate a live `getGlobalInsts()` walk.
    //
    // Limitation: `isEmptyStruct` keys on zero fields, but the erasure it guards against triggers
    // whenever the struct legalizes to `none` — which also happens for a struct whose fields *all*
    // legalize to `none` (e.g. one holding only empty structs). Such a struct has a nonzero field
    // count, so it is not padded and the original abort recurs. This matches the pre-existing
    // behavior of `legalizeEmptyRayPayloadsForHLSL` and is left as a follow-up.
    HashSet<IRStructType*> emptyCallableDataStructs;
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;

        // Use site 1: the callable entry point's own data, a varying parameter. On D3D — the only
        // target this pass runs for — DXC requires callable data to be `inout`; it is lowered to an
        // `IROutParamTypeBase` here, so scanning those parameters covers it. (`IROutParamTypeBase`
        // spans both `out` and `inout`, the two mutable forms SPIR-V/CUDA additionally allow.)
        auto entryPointDecor = func->findDecoration<IREntryPointDecoration>();
        if (entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Callable)
        {
            for (auto param : func->getParams())
            {
                if (auto outType = as<IROutParamTypeBase>(param->getFullType()))
                    addIfEmptyStruct(outType->getValueType(), emptyCallableDataStructs);
            }
        }

        // Use site 2: the payload argument of a `CallShader` call. It is passed by `inout`, so the
        // argument is the address (a pointer) of the caller's payload local; the struct is its
        // pointee type.
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto call = as<IRCall>(inst);
                if (!call || call->getArgCount() < 2)
                    continue;
                if (!isCallShaderCall(call, targetCaps))
                    continue;
                if (auto ptrType = as<IRPtrTypeBase>(call->getArg(1)->getDataType()))
                    addIfEmptyStruct(ptrType->getValueType(), emptyCallableDataStructs);
            }
        }
    }

    // Unlike a ray payload, a callable-data struct is a plain `inout` parameter with no
    // `[raypayload]` attribute, so the dummy field must NOT carry payload access qualifiers; DXC
    // rejects `read()/write()` qualifiers on callable data.
    for (auto structType : emptyCallableDataStructs)
    {
        padEmptyStructWithDummyField(builder, structType, /*addPayloadAccessQualifiers*/ false);
    }
}

void legalizeEmptyCallableDataPayloadsForVulkan(IRModule* module)
{
    // On the Vulkan targets (SPIR-V and GLSL), a `CallShader` payload is a module-scope
    // `[__vulkanCallablePayload]` global — the `static Payload p` in `CallShader`'s `spirv`/`glsl`
    // arms — whose address feeds the callable dispatch. An empty payload struct legalizes to
    // `LegalType::Flavor::none`, which erases the global's value type. On SPIR-V that leaves
    // `OpExecuteCallableKHR ... &p` with a non-simple operand and type legalization aborts with
    // "non-simple operand(s)!"; on GLSL the erased `p` feeds `__callablePayloadLocation(p)` and
    // hits the same abort via a different instruction. Pad the empty struct so a real Callable Data
    // variable survives. This mirrors the global-var branch of `legalizeEmptyRayPayloadsForHLSL`;
    // keying on the global (rather than the `CallShader` call) means the fix does not depend on the
    // intrinsic call surviving un-inlined at this point. Callable data is a plain `inout`, not a
    // `[raypayload]`, so the dummy field carries no payload access qualifiers.
    //
    // Both the outgoing `[__vulkanCallablePayload]` and the incoming `[__vulkanCallablePayloadIn]`
    // decorations are matched. The outgoing form backs a `CallShader` caller (above) and is the one
    // exercised by the tests. The incoming form (lowered from `VulkanCallablePayloadInAttribute`)
    // marks incoming callable data; matching it is defensive — any module-scope global carrying
    // that decoration with an empty struct would legalize to `none` and abort the same way, so
    // padding it is the same correct fix. (A callable entry point's own empty payload parameter
    // materializes no such global on this path — it compiles to a valid `CallableKHR` entry point
    // with no variable — so this is not the entry-point case.)
    IRBuilder builder(module);

    HashSet<IRStructType*> emptyCallablePayloadStructs;
    for (auto globalInst : module->getGlobalInsts())
    {
        auto globalVar = as<IRGlobalVar>(globalInst);
        if (!globalVar)
            continue;
        if (!globalVar->findDecoration<IRVulkanCallablePayloadDecoration>() &&
            !globalVar->findDecoration<IRVulkanCallablePayloadInDecoration>())
            continue;
        auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
        if (!ptrType)
            continue;
        addIfEmptyStruct(ptrType->getValueType(), emptyCallablePayloadStructs);
    }

    for (auto structType : emptyCallablePayloadStructs)
    {
        padEmptyStructWithDummyField(builder, structType, /*addPayloadAccessQualifiers*/ false);
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
