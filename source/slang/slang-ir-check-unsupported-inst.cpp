#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

// Returns true if `type` is itself a leaf opaque "handle" type that SPIR-V
// forbids from being stored to or loaded from
// (VUID-StandaloneSpirv-OpTypeImage-06924): images/textures, samplers, sampled
// images, subpass inputs (including GLSL input attachments), and acceleration
// structures. These map to `OpTypeImage`/`OpTypeSampler`/`OpTypeSampledImage`/
// `OpTypeAccelerationStructureKHR`, none of which may live in a function-local
// variable.
static bool isLeafUnstorableOpaqueHandleType(IRType* type)
{
    return as<IRResourceTypeBase>(type) || as<IRSamplerStateTypeBase>(type) ||
           as<IRSubpassInputType>(type) || type->getOp() == kIROp_GLSLInputAttachmentType ||
           type->getOp() == kIROp_RaytracingAccelerationStructureType;
}

// Find an opaque handle type (see `isLeafUnstorableOpaqueHandleType`) that cannot
// live in a function-local variable, recursing into the element/field types of
// aggregates (arrays, structs, tuples) since storing an aggregate that contains
// such a handle has the same problem. Returns the leaf handle type if found (for
// diagnostics), or null otherwise. `visited` guards against cycles in
// (potentially self-referential) aggregate types, mirroring the peer helper
// `isOpaqueTypeImpl` in slang-legalize-types.cpp.
//
// Note: this is deliberately narrower than `isOpaqueType`. Buffer-backed
// resources (structured / byte-address buffers) and pointers lower to
// pointers and *can* be selected through control flow using SPIR-V variable
// pointers, so they must not be rejected here. `RayQuery`/`HitObject` are also
// excluded as they are legitimately declared as locals.
static IRType* findUnstorableOpaqueHandleType(IRType* type, HashSet<IRType*>& visited)
{
    if (!type)
        return nullptr;

    if (isLeafUnstorableOpaqueHandleType(type))
        return type;

    // Only recurse once per aggregate type to avoid cycling on self-referential
    // types.
    if (!visited.add(type))
        return nullptr;

    if (auto arrayType = as<IRArrayTypeBase>(type))
        return findUnstorableOpaqueHandleType(arrayType->getElementType(), visited);

    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (auto found = findUnstorableOpaqueHandleType(field->getFieldType(), visited))
                return found;
        }
    }

    if (auto tupleType = as<IRTupleTypeBase>(type))
    {
        for (UInt i = 0; i < tupleType->getOperandCount(); i++)
        {
            if (auto elementType = as<IRType>(tupleType->getOperand(i)))
            {
                if (auto found = findUnstorableOpaqueHandleType(elementType, visited))
                    return found;
            }
        }
    }

    return nullptr;
}

static IRType* findUnstorableOpaqueHandleType(IRType* type)
{
    HashSet<IRType*> visited;
    return findUnstorableOpaqueHandleType(type, visited);
}

/// Return the innermost element type stored by a local variable after removing attributed wrappers
/// and homogeneous arrays.
//
// We do not inspect record fields because type legalization has already separated resources from
// programmer-declared aggregates before this check runs. Metal's `introduceExplicitGlobalContext`
// pass also creates a compiler-owned `KernelContext` local whose resource fields are valid in that
// record.
static IRType* getInnermostHomogeneousArrayElementType(IRType* type)
{
    type = as<IRType>(unwrapAttributedType(type));
    SLANG_RELEASE_ASSERT(type);
    while (auto arrayType = as<IRArrayTypeBase>(type))
    {
        type = as<IRType>(unwrapAttributedType(arrayType->getElementType()));
        SLANG_RELEASE_ASSERT(type);
    }
    return type;
}

/// Find a resource type supported by per-invocation replacement that DXBC or DXIL cannot represent
/// in mutable local storage.
//
// We descend through array layers because the downstream D3D compilers reject the local when its
// innermost element has an affected resource type. We then reuse the replacement predicate to
// recognize the resource categories supported by per-invocation replacement.
static IRType* findD3DUnsupportedLocalStorageType(IRType* type)
{
    type = getInnermostHomogeneousArrayElementType(type);
    if (isResourceValueOrArrayTypeSupportedForPerInvocationReplacement(type))
        return type;
    return nullptr;
}

/// Find a resource or sampler type that Metal cannot place in mutable local storage.
//
// Metal rejects a resource or sampler stored directly in `thread` storage. It permits a fixed-size
// array with a resource or sampler as its immediate element. The `transformParamsToConstRef` pass
// uses this allowed form when a caller cannot supply an immutable address for a by-value array
// argument. It copies the argument into a local array and passes that array to the callee. An
// unsized array has no fixed amount of local storage. The current Metal storage contract permits
// exactly one fixed-size array layer and rejects nested arrays. For unsized and nested arrays, we
// inspect the innermost element. A resource or sampler element makes the local storage invalid. We
// remove attributed-type wrappers because they do not change the storage representation.
//
// TODO: Compile generated nested-array locals with metallib, and permit that form if the backend
// accepts its emitted representation.
static IRType* findMetalUnsupportedLocalStorageType(IRType* type)
{
    auto storageType = as<IRType>(unwrapAttributedType(type));
    SLANG_RELEASE_ASSERT(storageType);
    if (auto arrayType = as<IRArrayType>(storageType))
    {
        auto elementType = as<IRType>(unwrapAttributedType(arrayType->getElementType()));
        SLANG_RELEASE_ASSERT(elementType);
        if (as<IRResourceTypeBase>(elementType) || as<IRSamplerStateTypeBase>(elementType))
            return nullptr;
    }

    type = getInnermostHomogeneousArrayElementType(storageType);
    if (as<IRResourceTypeBase>(type) || as<IRSamplerStateTypeBase>(type))
        return type;
    return nullptr;
}

// True if `target` is a C++/CUDA *kernel* output target. The `String` type is
// implemented in terms of the Slang core runtime (`Slang::String`), which is
// available for host C++ output and for the LLVM-backed CPU path, but not in the
// C++/CUDA kernel preludes. Emitting a `String` value for one of these targets
// would reference an undefined `String` type/method (issue #11297), so it must be
// diagnosed instead. Host C++ and the LLVM CPU path are deliberately excluded
// because they do provide a `String` runtime. (CUDA/PTX `String` usage is usually
// also rejected earlier by capability checks, since `String`'s members are
// `[require(cpp)]`; this is the backend-agnostic safety net.)
static bool isKernelCPPOrCUDASourceTarget(TargetRequest* target)
{
    switch (target->getTarget())
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::PyTorchCppBinding:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PTX:
        return true;
    default:
        return false;
    }
}

// False for the targets on which a surviving function-typed value would be emitted as invalid
// output (issue #12367): kernel C++/CUDA, the Metal and WebGPU families, and `ShaderSharedLibrary`/
// `ShaderHostCallable`/`ShaderLLVMIR` (which lower through kernel C++). A `true` result does not
// mean the target can represent a function-typed value. HLSL and GLSL cannot either, but reject one
// loudly during emit (`E99999`), so this check need not cover them. SPIR-V is separately scoped:
// at the default optimization level it aborts in spirv-opt, but at `-O0` it can silently emit
// invalid output; closing that is tracked apart from this change.
static bool doesTargetSupportFuncTypedValue(TargetRequest* target)
{
    switch (target->getTarget())
    {
    // TODO: C++ and CUDA (the CPP*/CUDA*/PTX cases below) can represent a function pointer, so they
    // should eventually support a function-typed value and be removed from this list.
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PTX:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::ShaderHostCallable:
    case CodeGenTarget::ShaderLLVMIR:
        return false;
    default:
        return !isMetalTarget(target) && !isWGPUTarget(target);
    }
}

// True if `funcType` has any parameter or result of type `String`.
static bool funcTypeReferencesStringType(IRFuncType* funcType)
{
    if (as<IRStringType>(funcType->getResultType()))
        return true;
    for (UInt i = 0; i < funcType->getParamCount(); i++)
    {
        if (as<IRStringType>(funcType->getParamType(i)))
            return true;
    }
    return false;
}

// True if `inst` produces or consumes a `String` value that requires the (host-
// only) `String` runtime. This is either an inst whose result type is `String`
// (e.g. `MakeString`, or reading a `String` local), or a call to a function
// whose signature takes/returns `String` (e.g. `String.getLength`).
//
// We must key a call on the *callee's parameter type*, not on its argument
// values: a string literal `"..."` has type `String` even when it is implicitly
// converted to a `NativeString` argument, so `NativeString.getLength("...")`
// (which is supported) would be misflagged if we looked at argument types.
// `NativeString.getLength` takes a `NativeString` parameter, so checking the
// callee's signature correctly distinguishes it from `String.getLength`.
static bool instReferencesStringType(IRInst* inst)
{
    if (as<IRStringType>(inst->getDataType()))
        return true;

    if (auto call = as<IRCall>(inst))
    {
        if (auto callee = call->getCalleeUse()->get())
        {
            if (auto funcType = as<IRFuncType>(callee->getFullType()))
                return funcTypeReferencesStringType(funcType);
        }
    }

    return false;
}

void checkUnsupportedInst(TargetRequest* target, IRFunc* func, DiagnosticSink* sink)
{
    // We perform four independent checks. We diagnose unsupported function-typed values,
    // `GetArrayLength`, target-specific opaque local storage and default construction, and `String`
    // values on kernel C++ or CUDA targets. We establish the target rules first, then scan the
    // function parameters and instructions once.

    // Resource specialization and SSA simplification run before this check. A resource value may
    // remain as an SSA instruction, which needs no local storage, or phi elimination may place it
    // in an `IRVar`. SPIR-V, GLSL, WGSL, DXBC, DXIL, and Metal cannot represent particular resource
    // types in that local storage. The HLSL emitter can print the local without losing information,
    // so we allow HLSL source emission and let the compiler that receives that source decide
    // whether to accept it.
    const bool shouldUseKhronosOrWGSLOpaqueDiagnostic =
        isKhronosTarget(target) || isWGPUTarget(target);
    const bool shouldUseD3DOpaqueDiagnostic =
        isD3DTarget(target) && target->getTarget() != CodeGenTarget::HLSL;
    const bool shouldRejectOpaqueLocalStorage = shouldUseKhronosOrWGSLOpaqueDiagnostic ||
                                                shouldUseD3DOpaqueDiagnostic ||
                                                isMetalTarget(target);

    // Several unsupported locals can carry the same source location. We report only one
    // D3D or Metal error at each location so that the user does not receive duplicate diagnostics
    // there. The location is only a presentation key; it does not identify the IR instruction.
    HashSet<SourceLoc::RawValue> diagnosedOpaqueLocalStorageLocations;

    // The `String` type has no runtime representation in kernel C++/CUDA output;
    // a use there (e.g. `let s : String = "1"; s.getLength();`) would otherwise
    // emit uncompilable code referencing an undefined `String`/method instead of
    // any diagnostic.
    const bool rejectString = isKernelCPPOrCUDASourceTarget(target);

    const bool supportsFuncTypedValue = doesTargetSupportFuncTypedValue(target);

    auto diagnoseFuncTypedValue = [&](IRInst* inst)
    {
        if (supportsFuncTypedValue || !as<IRFuncType>(unwrapArrayAndPointers(inst->getDataType())))
            return;
        // One mistake is reachable as several insts (the local, a parameter it flows to,
        // synthesized temporaries), some without a location. Reporting only those that can name a
        // position keeps it to one error per mistake instead of several pointing nowhere.
        auto loc = inst->sourceLoc.isValid() ? inst->sourceLoc : findFirstUseLoc(inst);
        if (loc.isValid())
            sink->diagnose(Diagnostics::FuncTypeNotSupportedOnTarget{.location = loc});
    };

    if (!supportsFuncTypedValue)
    {
        if (auto firstBlock = func->getFirstBlock(); firstBlock)
        {
            for (auto param : firstBlock->getParams())
                diagnoseFuncTypedValue(param);
        }
    }

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->getOp() == kIROp_Var)
                diagnoseFuncTypedValue(inst);

            switch (inst->getOp())
            {
            case kIROp_GetArrayLength:
                sink->diagnose(
                    Diagnostics::AttemptToQuerySizeOfUnsizedArray{.location = inst->sourceLoc});
                break;
            case kIROp_Var:
                if (shouldRejectOpaqueLocalStorage)
                {
                    auto pointerType = as<IRPtrTypeBase>(unwrapAttributedType(inst->getDataType()));
                    SLANG_RELEASE_ASSERT(pointerType);
                    auto valueType = pointerType->getValueType();
                    SLANG_RELEASE_ASSERT(valueType);
                    IRType* opaqueType = nullptr;
                    if (shouldUseKhronosOrWGSLOpaqueDiagnostic)
                        opaqueType = findUnstorableOpaqueHandleType(valueType);
                    else if (shouldUseD3DOpaqueDiagnostic)
                        opaqueType = findD3DUnsupportedLocalStorageType(valueType);
                    else
                        opaqueType = findMetalUnsupportedLocalStorageType(valueType);

                    if (opaqueType)
                    {
                        // A synthesized variable, such as one created by phi elimination, may have
                        // no source location. In that case we report the first operation that uses
                        // the storage so that the diagnostic still identifies relevant user code.
                        auto loc =
                            inst->sourceLoc.isValid() ? inst->sourceLoc : findFirstUseLoc(inst);
                        if (shouldUseKhronosOrWGSLOpaqueDiagnostic)
                        {
                            sink->diagnose(
                                Diagnostics::OpaqueTypeInLocalVariableNotAllowedOnKhronos{
                                    .type = opaqueType,
                                    .location = loc});
                        }
                        else
                        {
                            bool shouldDiagnose = !loc.isValid();
                            if (loc.isValid())
                            {
                                shouldDiagnose =
                                    diagnosedOpaqueLocalStorageLocations.add(loc.getRaw());
                            }
                            if (shouldDiagnose)
                            {
                                sink->diagnose(Diagnostics::OpaqueLocalStorageNotSupportedForTarget{
                                    .type = opaqueType,
                                    .target = target->getTarget(),
                                    .location = loc});
                            }
                        }
                    }
                }
                break;
            case kIROp_DefaultConstruct:
                if (shouldUseKhronosOrWGSLOpaqueDiagnostic)
                {
                    // There is no default/zero value for an opaque handle (an
                    // image/sampler/subpass/acceleration-structure has no bit
                    // pattern we can materialize), so a `defaultConstruct` of such
                    // a type is invalid output for Khronos/WGSL. This typically
                    // arises from `Optional<Texture2D>` being lowered when a
                    // generic wrapper is instantiated with a resource type and a
                    // `none` payload is default-constructed (issue #7878); the
                    // front-end `Optional<T>` check does not fire because `T` is
                    // only known after specialization. Diagnose instead of letting
                    // the unhandled inst reach spirv-emit and abort.
                    if (auto handleType = findUnstorableOpaqueHandleType(inst->getDataType()))
                    {
                        auto loc =
                            inst->sourceLoc.isValid() ? inst->sourceLoc : findFirstUseLoc(inst);
                        sink->diagnose(Diagnostics::OpaqueTypeInLocalVariableNotAllowedOnKhronos{
                            .type = handleType,
                            .location = loc});
                    }
                }
                break;
            }

            // A `String` value has no valid lowering for a kernel C++/CUDA
            // target. Diagnose a `String`-typed result or a call into a
            // `String`-signature function (e.g. `String.getLength`) rather than
            // emitting uncompilable code referencing an undefined `String`.
            if (rejectString && instReferencesStringType(inst))
            {
                auto loc = inst->sourceLoc.isValid() ? inst->sourceLoc : findFirstUseLoc(inst);
                sink->diagnose(Diagnostics::StringTypeNotSupportedOnKernelTarget{.location = loc});
            }
        }
    }
}

void checkUnsupportedInst(IRModule* module, TargetRequest* target, DiagnosticSink* sink)
{
    // The CUDA/PTX emitter has no type name for a multisampled texture, whereas
    // the host C++ emitter does.
    const bool supportsMultisampledTexture = !isCUDATarget(target);
    const bool supportsFuncTypedValue = doesTargetSupportFuncTypedValue(target);

    for (auto globalInst : module->getGlobalInsts())
    {
        if (!supportsFuncTypedValue)
        {
            // C++/CUDA/Metal move a global into a `KernelContext` struct field
            // (`introduceExplicitGlobalContext`), so a function-typed global appears as a field.
            if (auto structType = as<IRStructType>(globalInst))
            {
                for (auto field : structType->getFields())
                {
                    if (as<IRFuncType>(unwrapArrayAndPointers(field->getFieldType())))
                    {
                        auto key = field->getKey();
                        auto loc = key->sourceLoc.isValid() ? key->sourceLoc : findFirstUseLoc(key);
                        sink->diagnose(Diagnostics::FuncTypeNotSupportedOnTarget{.location = loc});
                    }
                }
            }
            // WGSL does not run that pass, so its globals stay module-scope variables.
            else if (globalInst->getOp() == kIROp_GlobalVar)
            {
                if (as<IRFuncType>(unwrapArrayAndPointers(globalInst->getDataType())))
                {
                    auto loc = globalInst->sourceLoc.isValid() ? globalInst->sourceLoc
                                                               : findFirstUseLoc(globalInst);
                    sink->diagnose(Diagnostics::FuncTypeNotSupportedOnTarget{.location = loc});
                }
            }
        }

        switch (globalInst->getOp())
        {
        case kIROp_VectorType:
        case kIROp_MatrixType:
            {
                if (!as<IRBasicType>(globalInst->getOperand(0)) &&
                    !as<IRPackedFloatType>(globalInst->getOperand(0)))
                {
                    sink->diagnose(Diagnostics::UnsupportedBuiltinType{
                        .type = globalInst,
                        .location = findFirstUseLoc(globalInst)});
                }
                break;
            }
        case kIROp_TextureType:
            {
                // Texture types are hoisted, deduplicated global insts, so checking
                // the type here catches every use site (global param, struct field,
                // local, parameter) with a single diagnostic.
                if (!supportsMultisampledTexture &&
                    as<IRTextureTypeBase>(globalInst)->isMultisample())
                {
                    sink->diagnose(Diagnostics::MultisampledTextureNotSupportedOnTarget{
                        .type = globalInst,
                        .location = findFirstUseLoc(globalInst)});
                }
                break;
            }
        case kIROp_Func:
            checkUnsupportedInst(target, as<IRFunc>(globalInst), sink);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                    checkUnsupportedInst(target, innerFunc, sink);
                break;
            }
        default:
            break;
        }
    }
}

} // namespace Slang
