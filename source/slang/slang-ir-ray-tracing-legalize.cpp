// slang-ir-ray-tracing-legalize.cpp
// Keep required ray-tracing payload arguments and variables when their source type is empty.
// HLSL/DXIL need a payload argument and receiving parameter; GLSL/SPIR-V need a payload variable.
// We give these parameters and variables a wrapper containing the original value and a dummy int,
// without adding fields to the user's type. General type legalization then removes the empty
// value and keeps the dummy int. CUDA/OptiX permits the empty argument to disappear and needs no
// wrapper. The target policy at the end of this file selects these actions.
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

// An internal wrapper used in place of an empty payload at a ray-tracing call or entry point.
// For a source type T, `type` describes `struct { T _slang_data; int _slang_dummy; }`, and
// `dataKey` identifies its _slang_data field. Shader code still reads and writes T through that
// field. Later type legalization removes _slang_data because T is empty, leaving just _slang_dummy.
struct EmptyPayloadCarrier
{
    IRStructType* type;
    IRStructKey* dataKey;
};

// Find the empty payload parameters and variables that must survive, then change just those
// parameters and variables to use EmptyPayloadCarrier wrappers. Do not change the source types.
// Consider this example:
//
//     struct Empty {}
//     struct Data { Empty unused; uint value; }
//     [shader("raygeneration")]
//     void rgen()
//     {
//         Empty empty;
//         Data data = { empty, 37 };
//         CallShader(0, empty);
//         CallShader(1, data);
//     }
//     [shader("callable")]
//     void callee(inout Data data) { data.value += 5; }
//
// Adding a dummy field to Empty itself would also add storage to Data.unused. Compiling callee
// alone would not see CallShader(0, empty), so its Data would have a different layout from rgen's.
// Instead, wrap only the first call's payload and leave both Empty and Data unchanged. After type
// legalization, the relevant generated HLSL has this shape (names simplified):
//
//     struct EmptyCarrier { int _slang_dummy; }
//     struct Data { uint value; }
//     EmptyCarrier carrier = { 0 };
//     Data data = { 37 };
//     CallShader(0, carrier);
//     CallShader(1, data);
//
// Data now contains one uint in both rgen and a separately compiled callee. The sets below record
// the parameters/globals to rewrite; the cache lets them reuse one wrapper for each source type.
struct EmptyPayloadLegalizationContext
{
    IRModule* module;
    // Reuse wrappers without adding fields to the original types used as dictionary keys.
    Dictionary<IRType*, EmptyPayloadCarrier> carriers;
    // HLSL/DXIL intrinsic or entry-point parameters whose empty value needs a wrapper.
    HashSet<IRParam*> d3dParams;
    // The subset used for TraceRay/HitObject or hit/miss entry points, which need ray qualifiers.
    HashSet<IRParam*> d3dRayParams;
    // GLSL/SPIR-V payload globals. Each shader invocation has its own instance of these variables.
    HashSet<IRGlobalVar*> khronosGlobals;

    // Return the wrapper for an empty source type, creating it on first use. For example, an
    // Empty[2] payload becomes `struct { Empty _slang_data[2]; int _slang_dummy; }`. The array
    // field disappears during type legalization, leaving one int rather than padding each element.
    // Keeping _slang_data until then lets us redirect existing loads/stores without changing
    // their value types. The original type and its constructors are not modified.
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

    // Record a D3D payload parameter for materializeParam if it is an out/inout empty value.
    // Callers establish its payload role from a ray-tracing intrinsic or entry-point signature;
    // this method only checks whether its data will disappear. isRayPayload distinguishes ray
    // payloads from callable data so only ray wrappers receive payload access qualifiers.
    void collectParam(IRParam* param, bool isRayPayload)
    {
        auto ptrType = as<IROutParamTypeBase>(param->getDataType());
        if (!ptrType || !isEmptyType(ptrType->getValueType()))
            return;
        d3dParams.add(param);
        if (isRayPayload)
            d3dRayParams.add(param);
    }

    // Record a GLSL/SPIR-V payload global for materializeGlobal if its value is empty. The caller
    // has already checked its ray-payload or callable-data decoration. An Empty[2] variable needs
    // the same treatment as an Empty variable: both lose all their data during type legalization.
    void collectGlobal(IRGlobalVar* global)
    {
        auto ptrType = cast<IRPtrTypeBase>(global->getDataType());
        if (isEmptyType(ptrType->getValueType()))
            khronosGlobals.add(global);
    }

    // Inline eligible assembly helper calls that take this empty payload global. Return true if
    // any call was inlined, so the caller can remove the now-unused helper definitions.
    //
    // For example, HitObject.TraceRay passes its thread-local payload variable p to
    // __spirvTraceRayHitObjectEXT(..., p). That helper's assembly refers to its payload parameter,
    // not directly to p. If materializeGlobal redirected this ordinary call argument to
    // p._slang_data, type legalization would erase the argument and the assembly operand with it.
    // Inlining first makes the assembly refer directly to p. materializeGlobal can then keep
    // that reference on the wrapper while redirecting only shader loads/stores to p._slang_data.
    //
    // Reuse the existing intrinsic inliner's eligibility rules for these calls only. Other
    // intrinsics still run through the normal, later module-wide inlining pass.
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

    // Replace an empty HLSL/DXIL payload parameter with its wrapper and update every call to it.
    // For example, CallShader(0, empty) is temporarily rewritten as:
    //
    //     EmptyCarrier carrier = { empty, 0 };
    //     CallShader(0, carrier);
    //     empty = carrier._slang_data;
    //
    // The intrinsic's parameter changes from `inout Empty` to `inout EmptyCarrier` too. Its
    // GenericAsm body emits the target CallShader using that parameter. A receiving entry point,
    // such as `void callableMain(inout Empty data)`, gets the same parameter type; existing shader
    // accesses to data are redirected to data._slang_data. These copies and field accesses remain
    // well-typed until general type legalization removes them along with the empty data field.
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

    // Change an empty GLSL/SPIR-V payload global to use its wrapper, keeping dispatch instructions
    // attached to the variable itself. Consider CallShader(0, data) with empty source data. The
    // standard-library implementation uses a thread-local global p, with operations like:
    //
    //     p = data;
    //     OpExecuteCallableKHR 0 &p;
    //     data = p;
    //
    // After changing p's type to EmptyCarrier, rewrite this to:
    //
    //     p._slang_data = data;
    //     OpExecuteCallableKHR 0 &p;
    //     data = p._slang_data;
    //
    // Type legalization erases the empty assignments but leaves p with one int, so the dispatch
    // still has a payload variable. GLSL's payload-location query must likewise keep referring to
    // p, not p._slang_data. inlinePayloadIntrinsicCalls must first expose any SPIR-V assembly
    // hidden inside eligible helper calls, so we can distinguish it from ordinary data accesses.
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

// Prepare HLSL/DXIL ray-payload arguments and receiving parameters before empty types are removed.
// For example, TraceRay(..., payload) leaves a ForceVarIntoRayPayloadStructTemporarily marker on
// its payload argument: legalizeForcedStructArgumentsInChildren wraps a scalar payload in a struct,
// marks an existing nonempty struct as a ray payload, or records an empty parameter for wrapping.
// A separately compiled `void missMain(inout Empty payload)` has no TraceRay call, so
// markD3DEntryPointRayPayloadTypes identifies the payload from the shader stage and parameter.
// Both paths also identify the types that later need D3D payload access qualifiers.
static void prepareD3DRayTracingPayloads(IRModule* module, EmptyPayloadLegalizationContext& context)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;
        legalizeForcedStructArgumentsInChildren(func, context);
        markD3DEntryPointRayPayloadTypes(func, context);
    }
}

// Record empty GLSL/SPIR-V ray-payload variables, on both the sending and receiving sides.
// For example, the HitObject.TraceRay implementation declares `[__vulkanRayPayload] static T p;`
// and passes p to __spirvTraceRayHitObjectEXT. Lowering attaches IRVulkanRayPayloadDecoration to
// that global thread-local variable. A receiving hit/miss shader uses the corresponding
// IRVulkanRayPayloadInDecoration. These decorations identify which variables need a payload
// wrapper; there is no need to recognize the names of the functions that use them.
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

// Record empty GLSL/SPIR-V callable-data variables. For example, CallShader(0, data) uses the
// standard-library implementation that declares `[__vulkanCallablePayload] static T p;`, copies
// data into p, dispatches the callable shader with p, and copies p back to data. After inlining,
// the shader contains those operations directly: there need not be a CallShader function call
// left to find. The global thread-local variable p still has IRVulkanCallablePayloadDecoration,
// so we find it by that decoration. Receiving callable shaders use the matching
// IRVulkanCallablePayloadInDecoration and need the same wrapper if their variable survives here.
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

// Fill in missing SM 6.7+ read/write qualifiers on fields of D3D ray-payload structs. Visit all
// marked structs, not just types found at TraceRay calls: a separately compiled hit/miss shader
// also needs the qualifiers. This runs after wrapper creation so the dummy field is covered too.
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

// Choose which ray-tracing rewrites run for the output target. D3D means HLSL/DXIL; Khronos means
// GLSL/SPIR-V. D3D passes payloads as arguments/parameters, whereas Khronos dispatch instructions
// use decorated global thread-local variables. The flags separate finding those different forms,
// preserving empty payloads, and adding D3D field qualifiers. They are not optimization options.
struct RayTracingPayloadLegalizationPolicy
{
    // HLSL/DXIL: resolve the struct-conversion markers on TraceRay/HitObject arguments and inspect
    // hit/miss entry-point parameters. For `int p; TraceRay(..., p)`, create a struct containing p
    // because the target intrinsic requires a struct. For an empty p, record its parameter for
    // the dummy-field wrapper instead. Also mark ray-payload types for the qualifier step below.
    bool prepareD3DRayTracingPayloads = false;

    // GLSL/SPIR-V: find decorated outgoing/incoming ray-payload variables and wrap empty ones.
    // For `Empty p; TraceRay(..., p)`, the library uses a [__vulkanRayPayload] variable. Keep that
    // variable with one dummy int so OpTraceRayKHR (or GLSL's payload-location query) can name it
    // even after the source Empty value is erased. HitObject uses the same variable decorations.
    bool materializeEmptyKhronosRayPayloads = false;

    // HLSL/DXIL: wrap the empty payload parameter of CallShader and of callable entry points,
    // updating the intrinsic's callers too. `CallShader(0, empty)` must still have two arguments,
    // and `[shader("callable")] void f(inout Empty data)` must still have its data parameter.
    // Both sides end up using a struct with one dummy int, even when compiled separately.
    bool materializeEmptyD3DCallableData = false;

    // GLSL/SPIR-V: find decorated outgoing/incoming callable-data variables and wrap empty ones.
    // CallShader(0, empty) lowers through a [__vulkanCallablePayload] variable, not a fixed D3D
    // call signature. OpExecuteCallableKHR and GLSL's payload-location query still need that
    // variable. A receiving variable, when present, must have the same one-int representation.
    bool materializeEmptyKhronosCallableData = false;

    // SPIR-V targets: inline eligible assembly helpers taking a selected empty payload
    // variable before rewriting its uses. For example, __spirvTraceRayHitObjectEXT(..., p) hides
    // the dispatch operand inside its body. Inlining makes the assembly refer directly to p,
    // so materializeGlobal keeps the wrapper as that operand instead of its erasable data field.
    // GLSL uses a payload-location query directly and does not need this SPIR-V-specific step.
    bool inlineSPIRVPayloadIntrinsics = false;

    // D3D shader model 6.7+: fill in missing field-level payload access qualifiers, preserving
    // explicit qualifiers. For example, a ray wrapper's new _slang_dummy field needs the default
    // read(caller, anyhit, closesthit, miss) and write(caller, anyhit, closesthit, miss)
    // annotations. This applies to ray payloads, not callable data, and runs after wrappers have
    // been created.
    bool normalizeD3DPayloadAccessQualifiers = false;
};

// Enable the rewrites needed by this target; leave unsupported or unnecessary actions disabled.
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
