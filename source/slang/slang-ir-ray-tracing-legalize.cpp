// slang-ir-ray-tracing-legalize.cpp
//
// Empty logical payloads may disappear from ordinary code, but native ray-tracing interfaces
// can still require storage. After specialization, replace only these interface uses with a
// one-field struct. Keep the original empty values and copies intact for type legalization to
// erase normally; neither source types nor ordinary helper signatures acquire padding.
//
// D3D uses local arguments and entry-point parameters. D3D additionally requires every nonempty
// payload to be a struct, so a non-struct one gets a one-field wrapper struct: a receiving
// shader's parameter is retyped in place, while each dispatch and each ordinary call to that
// shader passes a copy-in/copy-out wrapper temporary. GLSL and SPIR-V use payload globals:
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

    // Create uninitialized local storage for a D3D call's empty payload argument.
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
    //
    // It returns dummy to the caller, which uses replaceNativeCallArgumentAndUpdateSignature to
    // produce CallShader(0, dummy). TraceRay uses the same sequence with DummyRayPayload. No source
    // data needs copying from p or back to p: in this example only the native call uses dummy,
    // and helper keeps its Empty parameter until normal type legalization erases it. The artificial
    // field has no logical value to preserve or observe, so it needs storage but no initialization
    // store. legalizeEntryPoint also uses this allocation for ordinary calls to a shader entry
    // point whose physical payload parameter has been adapted.
    IRInst* createDummyPayloadArgument(IRCall* call, bool isRayPayload)
    {
        auto type = getOrCreateDummyPayloadType(isRayPayload);
        IRBuilder builder(call);
        builder.setInsertBefore(call);
        return builder.emitVar(type);
    }

    // Replace one native-call argument and update the corresponding parameter and function
    // type. This changes only the specialized intrinsic declaration, never a user helper.
    //
    // Consider the specialized declaration and call site for CallShader with an Empty payload.
    // In this schematic IR, source-like declaration syntax describes Slang's IRFunc for the
    // intrinsic, not a receiving shader entry point or a declaration emitted into HLSL:
    //
    //     // Specialized declaration:
    //     void CallShader(uint shaderIndex, inout Empty payload);
    //
    //     // Call site:
    //     logical = var Empty;
    //     call CallShader(0, logical);
    //
    // Supplying the variable from createDummyPayloadArgument changes both to:
    //
    //     // Specialized declaration:
    //     void CallShader(uint shaderIndex, inout DummyCallablePayload payload);
    //
    //     // Call site:
    //     logical = var Empty;
    //     dummy = var DummyCallablePayload;
    //     call CallShader(0, dummy);
    //
    // Here logical is addressable storage for an Empty value: the IR var produces the pointer
    // passed to the inout parameter. This helper leaves it alone because other ordinary uses
    // may still need the original type. In this isolated example it is now unused; normal DCE
    // and type legalization remove it later. dummy is separate storage created by the caller.
    //
    // Replacing only the argument would leave an Empty parameter that type legalization can
    // erase, and the call would disagree with its declaration. Preserve the parameter's pointer
    // kind/address space, change its value type, then rebuild the IRFunc's type. These IRParams
    // describe a target intrinsic's signature; they are not an ordinary helper body whose typed
    // uses would also need rewriting. The callers adapt every call to that specialization using
    // shared physical types, so repeated calls finish with the same declaration and argument type.
    // fixUpFuncType rebuilds only Slang's IR function type from those parameters. The intrinsic
    // mapping stays unchanged: the HLSL emitter omits the intrinsic declaration and emits
    // CallShader(0U, dummy), using the struct-typed payload accepted by the native operation.
    // A receiving shader instead has a signature such as
    // [shader("callable")] void callableMain(inout Empty payload), with no shader-index parameter;
    // legalizeEntryPoint handles that separate interface.
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

    // Return the cached one-field wrapper struct for a non-struct D3D payload, creating it on
    // first use. Dispatch arguments and receiving shader parameters both use it.
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
    // must use the same struct type, not a fresh nominally distinct struct for each call. A
    // receiver in the same module shares that wrapper as well; a separately compiled one relies
    // only on the wrapper having its value type's layout.
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
    // For just the CallShader part of the example, the intermediate IR is schematically:
    //
    //     void helper(inout Empty p)
    //     {
    //         before();
    //         dummy = var DummyCallablePayload;
    //         call CallShader(0, dummy);
    //         after();
    //     }
    //
    // Later legalization removes helper's empty parameter, and HLSL emission writes
    // CallShader(0U, dummy) inside helper. TraceRay gets its own DummyRayPayload local at its
    // native call in the same way.
    //
    // CallShader's HLSL arm is a native intrinsic identified by KnownBuiltinDeclName::CallShader;
    // it has no struct-only marker. Its empty second argument gets a DummyCallablePayload local,
    // and a nonempty non-struct one gets the ForceVarIntoStructTemporarily_t wrapper with
    // copy-in/copy-out, the type wrapNonStructEntryPointPayload also gives a callable shader's
    // parameter. Only a ray-payload marker selects dummy storage for an empty value: no source
    // uses the non-ray ForceVarIntoStructTemporarily marker, so emitForcedStructTemporary
    // asserts rather than wraps an empty value reaching it.
    // TraceRay's HLSL arm instead passes p through ForceVarIntoRayPayloadStructTemporarily, which
    // identifies the argument needing a DummyRayPayload local. Both use
    // replaceNativeCallArgumentAndUpdateSignature to keep the native call and signature consistent.
    // Nonempty structs pass through unchanged; other nonempty values behind a struct-only marker
    // keep their real values through the wrapper and copy-in/copy-out sequence shown in
    // getForcedStructType's comment.
    void legalizeD3DCall(IRCall* call)
    {
        if (getBuiltinFuncEnum(call->getCallee()) == KnownBuiltinDeclName::CallShader)
        {
            SLANG_RELEASE_ASSERT(call->getArgCount() == 2);
            auto payload = call->getArg(1);
            auto valueType = cast<IRPtrTypeBase>(payload->getDataType())->getValueType();
            if (isEmptyType(valueType))
                replaceNativeCallArgumentAndUpdateSignature(
                    call,
                    1,
                    createDummyPayloadArgument(call, false));
            else if (!as<IRStructType>(valueType))
                replaceNativeCallArgumentAndUpdateSignature(
                    call,
                    1,
                    emitForcedStructTemporary(call, payload, false));
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
                {
                    IRBuilder builder(module);
                    addRayPayloadDecorationIfNeeded(builder, structType);
                }
            }
            else
            {
                replaceNativeCallArgumentAndUpdateSignature(
                    call,
                    i,
                    emitForcedStructTemporary(call, logical, isRayPayload));
            }
        }
    }

    // Return a wrapper temporary for call to use in place of logical, a pointer to nonempty
    // non-struct storage. Before call we copy logical's value into the wrapper's data field, and
    // after call we copy it back; the caller installs the returned temporary as the argument.
    IRInst* emitForcedStructTemporary(IRCall* call, IRInst* logical, bool isRayPayload)
    {
        auto valueType = cast<IRPtrTypeBase>(logical->getDataType())->getValueType();
        SLANG_RELEASE_ASSERT(!as<IRStructType>(valueType) && !isEmptyType(valueType));
        auto type = getForcedStructType(valueType, isRayPayload);
        auto field = *type->getFields().begin();
        IRBuilder builder(call);
        builder.setInsertBefore(call);
        auto var = builder.emitVar(type);
        builder.addNameHintDecoration(
            var,
            UnownedStringSlice(isRayPayload ? "rayPayload" : "forceVarIntoStructTemporarily"));
        auto data = builder.emitFieldAddress(builder.getPtrType(valueType), var, field->getKey());
        builder.emitStore(data, builder.emitLoad(logical));
        builder.setInsertAfter(call);
        builder.emitStore(logical, builder.emitLoad(data));
        return var;
    }

    // Prepare a decorated empty global's physical interface before instruction rewriting.
    // Transfer its decorations and DependsOn references to dummy storage and record the
    // logical-to-physical mapping. Leave ordinary loads/stores using the logical global so
    // their types stay consistent. The logical global is not replaced or retyped here; only
    // interface uses are redirected to the physical global. Ordinary type legalization later
    // erases the empty logical storage and copies. See legalizeKhronosInstruction for an example.
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

    // Give a D3D receiving shader's nonempty non-struct payload parameter the wrapper struct that
    // getForcedStructType supplies to dispatches of the same value type, because D3D accepts
    // only a struct payload. Consider this example:
    //
    //     [shader("miss")] void missMain(inout float4 p) { p.x = 1; helper(p); }
    //
    // This function changes the IR to the equivalent of:
    //
    //     [shader("miss")] void missMain(inout RayPayload_t p) { p.data.x = 1; helper(p.data); }
    //
    // The body accesses the payload in place through the data field, like a source struct
    // payload's field, so a write stays in the payload even when a later AcceptHitAndEndSearch
    // ends the shader. An ordinary call to the entry point passes a wrapper temporary instead,
    // the same copy-in/copy-out convention HLSL applies to any inout argument.
    void wrapNonStructEntryPointPayload(IRFunc* func, IRParam* param, bool isRayPayload)
    {
        auto ptrType = cast<IRPtrTypeBase>(param->getDataType());
        auto valueType = ptrType->getValueType();
        SLANG_RELEASE_ASSERT(isD3DTarget(targetProgram->getTargetReq()));
        SLANG_RELEASE_ASSERT(!as<IRStructType>(valueType) && !isEmptyType(valueType));
        auto type = getForcedStructType(valueType, isRayPayload);
        IRBuilder builder(module);
        param->setFullType(builder.getPtrTypeWithAddressSpace(type, ptrType));
        fixUpFuncType(func);

        builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
        auto data = builder.emitFieldAddress(
            builder.getPtrType(valueType),
            param,
            (*type->getFields().begin())->getKey());
        traverseUses(
            param,
            [&](IRUse* use)
            {
                if (use->getUser() != data)
                    use->set(data);
            });

        auto paramIndex = func->getFirstBlock()->getParamIndex(param);
        replaceOrdinaryCallArguments(
            func,
            paramIndex,
            [&](IRCall* call)
            { return emitForcedStructTemporary(call, call->getArg(paramIndex), isRayPayload); });
    }

    // Replace argument paramIndex of every ordinary call to the entry point func with
    // makeArgument(call). An entry point can have ordinary call sites on D3D, and
    // fixEntryPointCallsites separates their callable bodies only later in the pipeline, so those
    // calls must already match func's adapted physical signature.
    template<typename F>
    void replaceOrdinaryCallArguments(IRFunc* func, Index paramIndex, const F& makeArgument)
    {
        traverseUses(
            func,
            [&](IRUse* use)
            {
                auto call = as<IRCall>(use->getUser());
                if (!call || call->getCallee() != func)
                    return;
                call->setArg(paramIndex, makeArgument(call));
            });
    }

    // Give a receiving shader interface the physical payload storage its target requires: an
    // all-empty interface gets dummy storage while its body stays logically empty, and on D3D a
    // nonempty non-struct payload parameter gets a wrapper struct. Adapting dispatches cannot
    // cover a receiver compiled without its callers. Consider this all-empty example:
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
    // wrapNonStructEntryPointPayload shows the corresponding D3D change for inout float4.
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
            auto valueType = ptrType->getValueType();
            allEmpty &= isEmptyType(valueType);
            if (!isD3DTarget(targetProgram->getTargetReq()))
                continue;
            if (auto type = as<IRStructType>(valueType))
            {
                if (isRayPayload)
                {
                    IRBuilder builder(module);
                    addRayPayloadDecorationIfNeeded(builder, type);
                }
            }
            else if (!isEmptyType(valueType))
                wrapNonStructEntryPointPayload(func, param, isRayPayload);
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

            // Ordinary calls get fresh dummy storage; the body still uses its logical local.
            replaceOrdinaryCallArguments(
                func,
                func->getFirstBlock()->getParamIndex(param),
                [&](IRCall* call) { return createDummyPayloadArgument(call, isRayPayload); });
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

    // Prepare Khronos bindings before rewriting location queries. D3D does not use payload
    // globals. Walk directly instead of collecting every global/function into temporary lists:
    // this pass creates types/globals but never removes or reorders existing globals/functions.
    // Advance before transforming; physical globals inserted before the current logical global
    // do not need another visit, and appended types/keys are ignored by the global-variable test.
    if (isKhronosTarget(target))
    {
        for (auto inst = module->getModuleInst()->getFirstChild(); inst;)
        {
            auto current = inst;
            inst = inst->getNextInst();
            if (auto global = as<IRGlobalVar>(current))
                context.separateEmptyGlobal(global);
        }
    }

    // Every defined function can contain a native dispatch, including ordinary nested helpers
    // with no payload-related decoration or parameter. Skip declarations without blocks, but
    // do not filter to entry points or builtins. No transformation here creates new functions.
    for (auto inst = module->getModuleInst()->getFirstChild(); inst;)
    {
        auto current = inst;
        inst = inst->getNextInst();
        if (auto func = as<IRFunc>(current))
        {
            if (!func->isDefinition())
                continue;
            context.legalizeInstructions(func);
            context.legalizeEntryPoint(func);
        }
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
