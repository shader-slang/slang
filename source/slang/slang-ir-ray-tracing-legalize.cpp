// slang-ir-ray-tracing-legalize.cpp
//
// Empty logical payloads may disappear from ordinary code, but native ray-tracing interfaces
// can still require storage. After specialization, replace only these interface uses with a
// one-field struct. Keep the original empty values and copies intact for type legalization to
// erase normally; neither source types nor ordinary helper signatures acquire padding.
//
// D3D uses local arguments and entry-point parameters. GLSL and SPIR-V use payload globals:
// GLSL queries their locations, while SPIR-V dispatches reference the globals directly.
// CUDA/OptiX needs no artificial storage and does not run this pass.
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


// Return the required payload operand's index, counting the opcode as operand zero, or -1 for
// an unrelated instruction. On success, isRayPayload distinguishes ray from callable data.
// Classify the actual instruction, not its containing function: helpers can do other work.
static Index getSPIRVRayTracingPayloadOperandIndex(IRSPIRVAsmInst* inst, bool& isRayPayload)
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


// State shared by one boundary-legalization pass. Physical types are canonical per payload
// role; globals retain their own identities/locations even when their physical type is shared.
struct RayTracingPayloadLegalizationContext
{
    IRModule* module;
    TargetProgram* targetProgram;
    IRStructType* emptyRayPayloadType = nullptr;
    IRStructType* emptyCallablePayloadType = nullptr;
    Dictionary<IRGlobalVar*, IRGlobalVar*> physicalGlobals;
    Dictionary<IRType*, IRStructType*> forcedStructTypes;
    Dictionary<IRType*, IRStructType*> forcedRayPayloadTypes;

    // Return a nonempty physical type without changing the source payload type. Even direct
    // SPIR-V uses a real field here, so unmodified empty-type legalization can preserve it.
    IRStructType* getEmptyPayloadType(bool isRayPayload)
    {
        auto& type = isRayPayload ? emptyRayPayloadType : emptyCallablePayloadType;
        if (type)
            return type;
        IRBuilder builder(module);
        builder.setInsertInto(module);
        type = builder.createStructType();
        builder.addNameHintDecoration(
            type,
            UnownedStringSlice(isRayPayload ? "EmptyRayPayload" : "EmptyCallablePayload"));
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, UnownedStringSlice("_slang_dummy"));
        builder.createStructField(type, key, builder.getUIntType());
        if (isRayPayload && isD3DTarget(targetProgram->getTargetReq()))
            addRayPayloadDecorationIfNeeded(builder, type);
        return type;
    }

    // Create initialized local storage for a D3D intrinsic. There is no logical value to copy
    // into or out of this variable: its field exists only to satisfy the native signature.
    IRInst* createEmptyArgument(IRCall* call, bool isRayPayload)
    {
        auto type = getEmptyPayloadType(isRayPayload);
        IRBuilder builder(call);
        builder.setInsertBefore(call);
        auto var = builder.emitVar(type);
        builder.emitStore(var, builder.emitDefaultConstruct(type));
        return var;
    }

    // Keep the native declaration's parameter and function type consistent with its argument.
    // This changes only the specialized intrinsic declaration, never a user helper signature.
    void setNativeArgument(IRCall* call, UInt index, IRInst* arg)
    {
        auto callee = cast<IRFunc>(call->getCallee());
        // These declarations are native target intrinsics, not wrapper bodies with typed uses
        // of the parameter. Target specialization has already selected their implementation.
        UnownedStringSlice definition;
        IRInst* intrinsicInst = nullptr;
        SLANG_RELEASE_ASSERT(findTargetIntrinsicDefinition(
            callee,
            targetProgram->getTargetReq()->getTargetCaps(),
            definition,
            intrinsicInst));
        auto param = getParamAt(callee->getFirstBlock(), index);
        SLANG_RELEASE_ASSERT(param);
        IRBuilder builder(module);
        auto paramType = cast<IRPtrTypeBase>(param->getDataType());
        auto valueType = cast<IRPtrTypeBase>(arg->getDataType())->getValueType();
        param->setFullType(builder.getPtrTypeWithAddressSpace(valueType, paramType));
        fixUpFuncType(callee);
        call->setArg(index, arg);
    }

    // Return the existing one-field representation for a non-struct D3D argument. Sharing the
    // type keeps every call of the same specialized native declaration type-consistent.
    IRStructType* getForcedStructType(IRType* valueType, bool isRayPayload)
    {
        auto& types = isRayPayload ? forcedRayPayloadTypes : forcedStructTypes;
        if (auto found = types.tryGetValue(valueType))
            return *found;
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto type = builder.createStructType();
        builder.addNameHintDecoration(
            type,
            UnownedStringSlice(isRayPayload ? "RayPayload_t" : "ForceVarIntoStructTemporarily_t"));
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, UnownedStringSlice("data"));
        builder.createStructField(type, key, valueType);
        if (isRayPayload)
            addRayPayloadDecorationIfNeeded(builder, type);
        types.add(valueType, type);
        return type;
    }

    // Resolve the standard library's struct-only markers at the native call. Consider:
    //
    //     void helper(inout Empty p) { before(); TraceRay(..., p); after(); }
    //
    // TraceRay's HLSL arm forwards p through ForceVarIntoRayPayloadStructTemporarily to the
    // native intrinsic. Only that argument becomes a dummy; helper's p and body stay intact.
    // Nonempty scalars retain the existing wrapper and copy-in/copy-out behavior.
    void legalizeD3DCall(IRCall* call)
    {
        if (getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::CallShader)
        {
            SLANG_RELEASE_ASSERT(call->getArgCount() == 2);
            auto ptrType = cast<IRPtrTypeBase>(call->getArg(1)->getDataType());
            if (isEmptyType(ptrType->getValueType()))
                setNativeArgument(call, 1, createEmptyArgument(call, false));
        }

        for (UInt i = 0; i < call->getArgCount(); ++i)
        {
            auto marker = call->getArg(i);
            const bool isRayPayload =
                marker->getOp() == kIROp_ForceVarIntoRayPayloadStructTemporarily;
            if (!isRayPayload && marker->getOp() != kIROp_ForceVarIntoStructTemporarily)
                continue;

            auto logical = marker->getOperand(0);
            auto valueType = cast<IRPtrTypeBase>(logical->getDataType())->getValueType();
            IRBuilder builder(call);
            builder.setInsertBefore(call);
            if (isRayPayload && isEmptyType(valueType))
            {
                setNativeArgument(call, i, createEmptyArgument(call, true));
            }
            else if (auto structType = as<IRStructType>(valueType))
            {
                setNativeArgument(call, i, logical);
                if (isRayPayload)
                    addRayPayloadDecorationIfNeeded(builder, structType);
            }
            else
            {
                auto type = getForcedStructType(valueType, isRayPayload);
                auto field = *type->getFields().begin();
                auto var = builder.emitVar(type);
                builder.addNameHintDecoration(
                    var,
                    UnownedStringSlice(
                        isRayPayload ? "rayPayload" : "forceVarIntoStructTemporarily"));
                auto data =
                    builder.emitFieldAddress(builder.getPtrType(valueType), var, field->getKey());
                builder.emitStore(data, builder.emitLoad(logical));
                setNativeArgument(call, i, var);
                builder.setInsertAfter(call);
                builder.emitStore(logical, builder.emitLoad(data));
            }
        }
    }

    // Separate a decorated empty global's physical interface from its logical copies.
    // CallShader's GLSL arm contains:
    //
    //     static Empty p;
    //     p = payload;
    //     executeCallable(index, getPayloadLocation(p));
    //     payload = p;
    //
    // Move the binding metadata to a new physical global, but leave both copies using p.
    // Replacing all uses of p would instead mix Empty and Dummy types in those assignments.
    void separateEmptyGlobal(IRGlobalVar* logical)
    {
        auto rayDecor = logical->findDecoration<IRVulkanRayPayloadDecoration>();
        auto rayInDecor = logical->findDecoration<IRVulkanRayPayloadInDecoration>();
        bool isRayPayload = rayDecor || rayInDecor;
        if (!isRayPayload && !logical->findDecoration<IRVulkanCallablePayloadDecoration>() &&
            !logical->findDecoration<IRVulkanCallablePayloadInDecoration>())
            return;
        auto ptrType = logical->getDataType();
        if (!isEmptyType(ptrType->getValueType()))
            return;

        auto type = getEmptyPayloadType(isRayPayload);
        IRBuilder builder(module);
        builder.setInsertBefore(logical);
        auto physical = builder.createGlobalVar(type);
        physical->setFullType(builder.getPtrTypeWithAddressSpace(type, ptrType));
        logical->transferDecorationsTo(physical);
        physicalGlobals.add(logical, physical);

        // Entry-point dependencies describe the interface, not the vanished logical data.
        traverseUses(
            logical,
            [&](IRUse* use)
            {
                if (use->getUser()->getOp() == kIROp_DependsOnDecoration)
                    use->set(physical);
            });
    }

    // Reuse an explicitly bound global when present. A raw dispatch inside a user helper may
    // instead reference an empty parameter/local, which needs only fresh outgoing ABI storage.
    IRGlobalVar* getPhysicalGlobal(IRInst* logical, bool isRayPayload)
    {
        if (auto global = as<IRGlobalVar>(logical))
        {
            if (auto found = physicalGlobals.tryGetValue(global))
                return *found;
        }
        IRBuilder builder(module);
        builder.setInsertInto(module);
        auto physical = builder.createGlobalVar(getEmptyPayloadType(isRayPayload));
        if (isRayPayload)
            builder.addVulkanRayPayloadDecoration(physical, -1);
        else
            builder.addVulkanCallablePayloadDecoration(physical, -1);
        return physical;
    }

    // Replace only a Khronos dispatch's interface operand. In GLSL that operand belongs to
    // the location query, not to executeCallableEXT/traceRayEXT, which take integer locations.
    void legalizeKhronosInstruction(IRInst* inst)
    {
        if (inst->getOp() == kIROp_GetVulkanRayTracingPayloadLocation)
        {
            auto global = cast<IRGlobalVar>(inst->getOperand(0));
            if (auto found = physicalGlobals.tryGetValue(global))
                inst->setOperand(0, *found);
            return;
        }

        auto asmInst = as<IRSPIRVAsmInst>(inst);
        if (!asmInst)
            return;
        bool isRayPayload = false;
        auto index = getSPIRVRayTracingPayloadOperandIndex(asmInst, isRayPayload);
        if (index < 0)
            return;
        auto operand = as<IRSPIRVAsmOperandInst>(asmInst->getOperand(index));
        // Location-based assembly operands (__rayPayloadFromLocation and
        // __rayCallableFromLocation) are resolved later against the decorated globals. Their
        // bindings already belong to physical storage after separateEmptyGlobal, so only direct
        // value operands need rewriting here.
        if (!operand)
            return;
        auto logical = operand->getValue();
        auto ptrType = as<IRPtrTypeBase>(logical->getDataType());
        if (!ptrType || !isEmptyType(ptrType->getValueType()))
            return;
        auto physical = getPhysicalGlobal(logical, isRayPayload);
        IRBuilder builder(asmInst);
        builder.setInsertBefore(asmInst);
        asmInst->setOperand(index, builder.emitSPIRVAsmOperandInst(physical));
    }

    // Visit instruction bodies without replacing or inlining the functions containing them.
    void legalizeInstructions(IRInst* parent)
    {
        for (auto inst : parent->getChildren())
        {
            if (isD3DTarget(targetProgram->getTargetReq()))
            {
                if (auto call = as<IRCall>(inst))
                    legalizeD3DCall(call);
            }
            else
                legalizeKhronosInstruction(inst);
            legalizeInstructions(inst);
        }
    }

    // Preserve receiving interfaces independently of callers, including separately compiled
    // shaders. Consider:
    //
    //     [shader("miss")] void missMain(inout Empty p) { helper(p); }
    //
    // D3D receives a physical parameter, while helper still receives a logical Empty local.
    // Khronos receives a decorated global pinned to the entry point; its old parameters and
    // their uses can disappear unchanged. Multiple Khronos payload parameters form one incoming
    // interface, so no padding is introduced alongside any nonempty payload parameter.
    void legalizeEntryPoint(IRFunc* func)
    {
        auto entryPoint = func->findDecoration<IREntryPointDecoration>();
        if (!entryPoint)
            return;
        bool isRayPayload;
        switch (entryPoint->getProfile().getStage())
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

        List<IRParam*> payloadParams;
        bool allEmpty = true;
        for (auto param : func->getParams())
        {
            auto ptrType = as<IROutParamTypeBase>(param->getDataType());
            if (!ptrType)
                continue;
            payloadParams.add(param);
            allEmpty &= isEmptyType(ptrType->getValueType());
            if (isRayPayload && isD3DTarget(targetProgram->getTargetReq()))
            {
                if (auto type = as<IRStructType>(ptrType->getValueType()))
                {
                    IRBuilder builder(module);
                    addRayPayloadDecorationIfNeeded(builder, type);
                }
            }
        }
        if (payloadParams.getCount() == 0 || !allEmpty)
            return;

        auto type = getEmptyPayloadType(isRayPayload);
        IRBuilder builder(module);
        if (isKhronosTarget(targetProgram->getTargetReq()))
        {
            builder.setInsertBefore(func);
            auto physical = builder.createGlobalVar(type);
            builder.addNameHintDecoration(physical, UnownedStringSlice("incomingEmptyPayload"));
            if (isRayPayload)
                builder.addVulkanRayPayloadInDecoration(physical, 0);
            else
                builder.addDecoration(
                    physical,
                    kIROp_VulkanCallablePayloadInDecoration,
                    builder.getIntValue(builder.getIntType(), 0));
            builder.addDependsOnDecoration(func, physical);
        }
        else
        {
            auto param = payloadParams[0];
            auto ptrType = cast<IRPtrTypeBase>(param->getDataType());
            builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
            auto logical = builder.emitVar(ptrType->getValueType());
            param->replaceUsesWith(logical);
            param->setFullType(builder.getPtrTypeWithAddressSpace(type, ptrType));
            fixUpFuncType(func);

            // An entry point can also have ordinary call sites on D3D: fixEntryPointCallsites
            // separates their callable bodies later in the pipeline. Supply the physical
            // argument now to keep those calls well typed; the body still uses its logical local.
            Index paramIndex = 0;
            for (auto p : func->getParams())
            {
                if (p == param)
                    break;
                ++paramIndex;
            }
            traverseUses(
                func,
                [&](IRUse* use)
                {
                    if (auto call = as<IRCall>(use->getUser()))
                    {
                        if (call->getCallee() == func)
                            call->setArg(paramIndex, createEmptyArgument(call, isRayPayload));
                    }
                });
        }
    }
};

void legalizeRayTracingPayloads(IRModule* module, TargetProgram* targetProgram)
{
    auto target = targetProgram->getTargetReq();
    if (!isD3DTarget(target) && !isKhronosTarget(target))
        return;

    RayTracingPayloadLegalizationContext context;
    context.module = module;
    context.targetProgram = targetProgram;

    // Snapshot globals before inserting physical types/variables. Binding separation precedes
    // instruction rewriting so location queries can refer to the original-to-physical mapping.
    List<IRGlobalVar*> globals;
    List<IRFunc*> funcs;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto global = as<IRGlobalVar>(inst))
            globals.add(global);
        if (auto func = as<IRFunc>(inst))
            funcs.add(func);
    }
    if (isKhronosTarget(target))
        for (auto global : globals)
            context.separateEmptyGlobal(global);

    for (auto func : funcs)
    {
        context.legalizeInstructions(func);
        context.legalizeEntryPoint(func);
    }

    // Normalize qualifiers after creating physical ray types as well as marking nonempty
    // source ray types. Explicit user qualifiers remain unchanged.
    if (requiresPayloadAccessQualifiers(targetProgram))
    {
        IRBuilder builder(module);
        for (auto inst : module->getGlobalInsts())
        {
            if (auto type = as<IRStructType>(inst))
                if (type->findDecoration<IRRayPayloadDecoration>())
                    addDefaultPayloadAccessQualifiersToStruct(builder, type);
        }
    }
}

} // namespace Slang
