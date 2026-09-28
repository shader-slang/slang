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
// These positions follow the operand order in external/spirv-headers/include/spirv/unified1/
// spirv.core.grammar.json. IRSPIRVAsmInst stores generic operands, not a named payload operand;
// their common IdRef kind cannot distinguish the payload from the other arguments. The opcode
// determines that role. None of these opcodes has result-type/result-ID operands to skip.
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
        // Accel, RayFlags, CullMask, SBTOffset, SBTStride, MissIndex, Origin, TMin, Direction,
        // TMax, Payload: ten arguments precede Payload, plus operand zero for the opcode.
        payloadIndex = 11;
        break;
    case SpvOpExecuteCallableKHR:
        // SBTIndex, CallableData: the callable data follows the shader index.
        isRayPayload = false;
        payloadIndex = 2;
        break;
    case SpvOpTraceRayMotionNV:
    case SpvOpHitObjectTraceRayNV:
    case SpvOpHitObjectTraceRayEXT:
    case SpvOpHitObjectTraceReorderExecuteEXT:
        // TraceRayMotionNV adds Time before Payload in the TraceRayKHR sequence. The hit-object
        // forms instead prepend HitObject before Accel. Either adds one operand before Payload.
        payloadIndex = 12;
        break;
    case SpvOpHitObjectTraceRayMotionNV:
    case SpvOpHitObjectTraceRayMotionEXT:
    case SpvOpHitObjectTraceMotionReorderExecuteEXT:
        // These forms add both HitObject before Accel and Time before Payload.
        payloadIndex = 13;
        break;
    case SpvOpHitObjectExecuteShaderNV:
    case SpvOpHitObjectExecuteShaderEXT:
    case SpvOpHitObjectReorderExecuteShaderEXT:
        // HitObject, Payload: shader execution takes the payload immediately after HitObject.
        payloadIndex = 2;
        break;
    default:
        return -1;
    }

    // The three EXT reorder forms allow optional Hint/Bits operands after Payload. Its fixed
    // position still applies when they are present; getOperandCount() - 1 would select a hint.
    SLANG_RELEASE_ASSERT(inst->getOperandCount() >= UInt(payloadIndex + 1));
    return payloadIndex;
}


// State shared by one boundary-legalization pass. Physical types are canonical per payload
// role; globals retain their own identities/locations even when their physical type is shared.
struct RayTracingPayloadLegalizationContext
{
    IRModule* module;
    TargetProgram* targetProgram;
    IRStructType* dummyRayPayloadType = nullptr;
    IRStructType* dummyCallablePayloadType = nullptr;
    Dictionary<IRGlobalVar*, IRGlobalVar*> physicalGlobals;
    Dictionary<IRType*, IRStructType*> forcedStructTypes;
    Dictionary<IRType*, IRStructType*> forcedRayPayloadTypes;

    // Return the cached dummy struct for this payload role, creating it on first use. Its single
    // uint field provides physical storage without changing the original empty source type.
    // Even direct SPIR-V uses a real field so ordinary empty-type legalization preserves it.
    IRStructType* getOrCreateDummyPayloadType(bool isRayPayload)
    {
        auto& type = isRayPayload ? dummyRayPayloadType : dummyCallablePayloadType;
        if (type)
            return type;
        IRBuilder builder(module);
        builder.setInsertInto(module);
        type = builder.createStructType();
        builder.addNameHintDecoration(
            type,
            UnownedStringSlice(isRayPayload ? "DummyRayPayload" : "DummyCallablePayload"));
        auto key = builder.createStructKey();
        builder.addNameHintDecoration(key, UnownedStringSlice("_slang_dummy"));
        builder.createStructField(type, key, builder.getUIntType());
        if (isRayPayload && isD3DTarget(targetProgram->getTargetReq()))
            addRayPayloadDecorationIfNeeded(builder, type);
        return type;
    }

    // Create initialized local storage for a D3D intrinsic's empty argument.
    // Consider this example:
    //
    //     struct Empty {};
    //     void helper(inout Empty p) { CallShader(0, p); }
    //
    // Ordinary type legalization can erase p, but native HLSL CallShader still requires its
    // second argument. Immediately before that call, this function inserts the following IR
    // (shown schematically), using the one-uint type from getOrCreateDummyPayloadType:
    //
    //     dummy = var DummyCallablePayload;
    //     store(dummy, makeStruct(DummyCallablePayload, 0u));
    //
    // It returns dummy to the caller, which uses replaceNativeCallArgumentAndUpdateSignature to
    // produce CallShader(0, dummy). TraceRay uses the same sequence with DummyRayPayload. No source
    // data needs copying from p or back to p: only the native call uses dummy, and helper keeps its
    // Empty parameter until normal type legalization erases it. Initialization supplies a defined
    // value for the artificial field passed to the native inout parameter; emitDefaultConstruct
    // builds the struct from its zero-initialized uint field.
    IRInst* createDummyPayloadArgument(IRCall* call, bool isRayPayload)
    {
        auto type = getOrCreateDummyPayloadType(isRayPayload);
        IRBuilder builder(call);
        builder.setInsertBefore(call);
        auto var = builder.emitVar(type);
        builder.emitStore(var, builder.emitDefaultConstruct(type));
        return var;
    }

    // Replace one native-call argument and update the corresponding parameter and function
    // type. This changes only the specialized intrinsic declaration, never a user helper.
    //
    // Consider the native CallShader declaration selected for an Empty payload. Supplying the
    // variable from createDummyPayloadArgument changes the call schematically from
    //
    //     void CallShaderMain(uint index, inout Empty payload);
    //     logical = var Empty;
    //     call CallShaderMain(0, logical);
    //
    // to
    //
    //     void CallShaderMain(uint index, inout DummyCallablePayload payload);
    //     logical = var Empty;
    //     dummy = var DummyCallablePayload;
    //     store(dummy, makeStruct(DummyCallablePayload, 0u));
    //     call CallShaderMain(0, dummy);
    //
    // Here logical is addressable storage for an Empty value: the IR var produces the pointer
    // passed to the inout parameter. dummy is the separate storage created by the caller, and
    // CallShaderMain denotes the selected native intrinsic declaration in both examples.
    //
    // Replacing only the argument would leave an Empty parameter that type legalization can
    // erase, and the call would disagree with its declaration. Preserve the parameter's pointer
    // kind/address space, change its value type, then rebuild the IRFunc's type. These IRParams
    // describe a target intrinsic's signature; they are not an ordinary helper body whose typed
    // uses would also need rewriting. The callers adapt every call to that specialization using
    // shared physical types, so repeated calls finish with the same declaration and argument type.
    void replaceNativeCallArgumentAndUpdateSignature(IRCall* call, UInt index, IRInst* arg)
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

    // Return the cached one-field struct for a non-struct D3D argument, creating it on first use.
    // Consider this example:
    //
    //     uint p = 7;
    //     TraceRay(scene, flags, mask, contribution, multiplier, missIndex, ray, p);
    //     consume(p);
    //
    // Slang accepts a scalar here, but the native HLSL operation requires a struct. For uint
    // this function creates the module-level IR type corresponding to
    //
    //     [raypayload] struct RayPayload_t { uint data; };
    //
    // legalizeD3DCall then uses it schematically as follows:
    //
    //     temporary = var RayPayload_t;
    //     field = fieldAddress(temporary, data);
    //     store(field, load(p));
    //     nativeTraceRay(..., temporary);
    //     store(p, load(field));
    //
    // Unlike an empty dummy, data carries the real input and output. Other struct-only markers
    // use ForceVarIntoStructTemporarily_t without ray-payload decoration. The cache is keyed by
    // source value type and separated by that role: every call to a specialized native function
    // must use the same struct type, not a fresh nominally distinct struct for each call.
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

    // Adapt native D3D calls to their struct-storage requirements before type legalization.
    // D3D requires nonempty struct storage for ray/callable payloads. An empty source struct
    // cannot supply that storage, and the following type legalization would erase its values
    // and parameters, leaving the native operation without its required payload argument.
    // Consider this example:
    //
    //     struct Empty {};
    //     void helper(inout Empty p)
    //     {
    //         before();
    //         CallShader(0, p);
    //         TraceRay(scene, flags, mask, contribution, multiplier, missIndex, ray, p);
    //         after();
    //     }
    //
    // Changing helper's parameter to a dummy struct would also require changing its callers,
    // which may forward the empty value through arbitrarily many other helpers. Instead, create
    // dummy storage next to each native intrinsic and pass it only to that operation, updating
    // the matching native declaration's signature. Empty data needs no copy-in/copy-out. The
    // ordinary helper chain keeps its original types until normal legalization erases its empty
    // arguments; before(), after(), and all other work in those helpers remain intact.
    //
    // CallShader's HLSL arm is a native intrinsic identified by KnownBuiltinDeclName::CallShader;
    // it has no struct-only marker. Its empty second argument gets a DummyCallablePayload local.
    // TraceRay's HLSL arm instead passes p through ForceVarIntoRayPayloadStructTemporarily, which
    // identifies the argument needing a DummyRayPayload local. Both use
    // replaceNativeCallArgumentAndUpdateSignature to keep the native call and signature consistent.
    // Nonempty structs pass through unchanged; nonempty scalars behind a struct-only marker keep
    // their real values through the wrapper and copy-in/copy-out sequence shown above.
    void legalizeD3DCall(IRCall* call)
    {
        if (getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::CallShader)
        {
            SLANG_RELEASE_ASSERT(call->getArgCount() == 2);
            auto ptrType = cast<IRPtrTypeBase>(call->getArg(1)->getDataType());
            if (isEmptyType(ptrType->getValueType()))
                replaceNativeCallArgumentAndUpdateSignature(
                    call,
                    1,
                    createDummyPayloadArgument(call, false));
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
                replaceNativeCallArgumentAndUpdateSignature(
                    call,
                    i,
                    createDummyPayloadArgument(call, true));
            }
            else if (auto structType = as<IRStructType>(valueType))
            {
                replaceNativeCallArgumentAndUpdateSignature(call, i, logical);
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
                replaceNativeCallArgumentAndUpdateSignature(call, i, var);
                builder.setInsertAfter(call);
                builder.emitStore(logical, builder.emitLoad(data));
            }
        }
    }

    // Prepare a decorated empty global's physical interface before instruction rewriting.
    // Transfer its decorations and DependsOn references to dummy storage and record the
    // logical-to-physical mapping. Leave ordinary loads/stores using the logical global so
    // their types stay consistent. See legalizeKhronosInstruction for the complete transformation.
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

        auto type = getOrCreateDummyPayloadType(isRayPayload);
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
        auto physical = builder.createGlobalVar(getOrCreateDummyPayloadType(isRayPayload));
        if (isRayPayload)
            builder.addVulkanRayPayloadDecoration(physical, -1);
        else
            builder.addVulkanCallablePayloadDecoration(physical, -1);
        return physical;
    }

    // Adapt the payload operands of native Khronos ray-tracing instructions. In GLSL the native
    // dispatch takes an integer location, so its associated query must reference a surviving
    // payload global. In direct SPIR-V the dispatch instead references the payload storage
    // itself. Either reference would lose its storage if an empty logical value were erased.
    //
    // Consider a CallShader(0, payload) call, where payload is storage for an Empty value. The
    // standard library's GLSL arm stages that value through a global. Before this pass, its IR
    // is schematically as follows. The -1 decoration operand requests automatic location
    // assignment, which assignRayPayloadHitObjectAttributeLocations performs later:
    //
    //     [VulkanCallablePayload(-1)] logical = globalVar Empty;
    //     store(logical, load(payload));
    //     location = getVulkanRayTracingPayloadLocation(logical);
    //     executeCallable(0, location);
    //     store(payload, load(logical));
    //
    // The pass driver first calls separateEmptyGlobal to create physical storage, move binding
    // decorations and DependsOn references to it, and record logical -> physical. This instruction
    // visit then redirects the location query using that mapping. Together these steps produce:
    //
    //     logical = globalVar Empty;
    //     [VulkanCallablePayload(-1)] physical = globalVar DummyCallablePayload;
    //     store(logical, load(payload));
    //     location = getVulkanRayTracingPayloadLocation(physical);
    //     executeCallable(0, location);
    //     store(payload, load(logical));
    //
    // Both copies remain correctly typed Empty-to-Empty until normal type legalization erases
    // them and logical. The nonempty physical global and its location query survive. Replacing
    // every use of logical would instead make those copies mix Empty and DummyCallablePayload.
    // Ray payloads use the same separation. For a direct SPIR-V dispatch, this function replaces
    // its payload operand with a prepared global or fresh outgoing dummy storage. It does not
    // change the containing helper's signature; receiving entry points are adapted separately.
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

    // Visit function bodies to adapt native dispatches and their payload-location queries.
    // The target helpers may create storage and update a native intrinsic's signature, but leave
    // ordinary helper signatures and bodies otherwise intact. Receiving shader interfaces are
    // handled separately by legalizeEntryPoint.
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

    // Give an all-empty receiving interface physical storage while keeping its body logically
    // empty. Adapting dispatches cannot cover a receiver compiled without its callers. Consider:
    //
    //     struct Empty {};
    //     void helper(inout Empty p) { /* ordinary shader work */ }
    //     [shader("miss")] void missMain(inout Empty p) { helper(p); }
    //
    // On D3D, this function changes the IR to the equivalent of:
    //
    //     [shader("miss")] void missMain(inout DummyRayPayload physical)
    //     {
    //         Empty logical;
    //         helper(logical);
    //     }
    //
    // The original Empty type and helper signature remain unchanged. Rewriting just p's type
    // would instead pass DummyRayPayload to helper(inout Empty), so the original body uses are
    // redirected to logical before changing the entry-point parameter and function type.
    //
    // On Khronos targets, the receiving interface is a global. This function leaves p and its
    // uses unchanged and adds the following IR, schematically:
    //
    //     [VulkanRayPayloadIn(0)] physical = globalVar DummyRayPayload;
    //     [DependsOn(physical)] [shader("miss")]
    //     void missMain(inout Empty p) { helper(p); }
    //
    // Normal type legalization can then erase p and helper's empty argument; the DependsOn
    // decoration retains physical as an entry-point interface even without executable uses.
    // Callable receivers use DummyCallablePayload and VulkanCallablePayloadIn instead.
    // For callable data, retaining this unused incoming storage is a policy choice to keep the
    // same physical representation as outgoing dispatches; an unused callable input could be
    // omitted. Vulkan ray-payload declarations, however, must survive even without shader uses.
    // Multiple Khronos out/inout parameters are later consolidated into one incoming object by
    // consolidateRayTracingParameters. If any of them is nonempty, that existing lowering already
    // has data to preserve; do not introduce an extra physical interface beside it.
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

        auto type = getOrCreateDummyPayloadType(isRayPayload);
        IRBuilder builder(module);
        if (isKhronosTarget(targetProgram->getTargetReq()))
        {
            builder.setInsertBefore(func);
            auto physical = builder.createGlobalVar(type);
            builder.addNameHintDecoration(physical, UnownedStringSlice("incomingDummyPayload"));
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
                            call->setArg(
                                paramIndex,
                                createDummyPayloadArgument(call, isRayPayload));
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
