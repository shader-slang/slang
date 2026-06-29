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
    // Khronos targets (SPIR-V and GLSL) and WGSL cannot place an
    // image/sampler/subpass/acceleration-structure handle in a function-local
    // variable: SPIR-V forbids OpStore/OpLoad (and OpPhi) of such a handle, GLSL
    // likewise forbids opaque-typed locals, and WGSL requires handle-address-space
    // variables (textures/samplers) to be module-scope. A local variable of one
    // of those types reaching here is invalid output we cannot legalize yet
    // (issue #10526, typically from selecting or returning a resource through
    // control flow); reject it with a diagnostic rather than emitting invalid code.
    const bool rejectOpaqueLocals = isKhronosTarget(target) || isWGPUTarget(target);

    // The `String` type has no runtime representation in kernel C++/CUDA output;
    // a use there (e.g. `let s : String = "1"; s.getLength();`) would otherwise
    // emit uncompilable code referencing an undefined `String`/method instead of
    // any diagnostic.
    const bool rejectString = isKernelCPPOrCUDASourceTarget(target);

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            switch (inst->getOp())
            {
            case kIROp_GetArrayLength:
                sink->diagnose(
                    Diagnostics::AttemptToQuerySizeOfUnsizedArray{.location = inst->sourceLoc});
                break;
            case kIROp_Var:
                if (rejectOpaqueLocals)
                {
                    auto valueType = as<IRVar>(inst)->getDataType()->getValueType();
                    if (auto handleType = findUnstorableOpaqueHandleType(valueType))
                    {
                        // The variable is usually synthesized (e.g. by phi
                        // elimination) and has no source location of its own, so
                        // fall back to the location of a use.
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
    for (auto globalInst : module->getGlobalInsts())
    {
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
