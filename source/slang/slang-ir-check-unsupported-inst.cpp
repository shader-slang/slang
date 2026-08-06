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

// True if `target` generates output that has no representation for a function-typed
// value, i.e. a value that holds a function and is stored, loaded, or selected at
// runtime rather than named directly at a call site.
//
// Consider this shader:
//
//      int addOne(int x) { return x + 1; }
//      static functype(int) -> int gFn = addOne;
//      void computeMain() { outBuf[0] = applyIt(gFn, 41); }
//
// `specializeHigherOrderParameters` normally replaces a function-typed argument with a direct
// call, but it is best-effort: `isParamSuitableForSpecialization` only accepts an argument it
// can resolve statically, and a call it cannot specialize is silently skipped. Reading `gFn`
// out of a variable is not one of those cases, so the function type survives to emit, where
// each of these targets writes something no prelude defines: `Slang_FuncType<...>` for
// C++/CUDA kernels (defined only in `slang-cpp-host-prelude.h`), `Func<...>` from Metal's
// op-name fallback, or an empty type annotation for WGSL (issue #12367).
//
// Host C++ is excluded because its prelude does define `Slang_FuncType`, for `[DllImport]`.
// The set is otherwise limited to targets where a function-typed value has been observed
// reaching emission and producing invalid output, which is why the PyTorch binding target is
// absent even though its prelude does not define the name either.
//
// `ShaderSharedLibrary` and `ShaderHostCallable` compile through kernel C++
// (`_getDefaultSourceForTarget`), so without them the undefined name is reported by the
// downstream compiler -- `'Slang_FuncType' does not name a type` from gcc -- rather than here.
static bool isTargetWithoutFuncTypeSupport(TargetRequest* target)
{
    switch (target->getTarget())
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PTX:
    case CodeGenTarget::ShaderSharedLibrary:
    case CodeGenTarget::ShaderHostCallable:
        return true;
    default:
        return isMetalTarget(target) || isWGPUTarget(target);
    }
}

// True if `type` is a function type, or an array of one, looking through the pointer that a
// variable or struct field is declared with. An array is included because it is emitted
// element-type-first and so produces the same undefined spelling.
//
// Struct types are deliberately not recursed into: every struct's fields are checked directly
// by the module-level walk, and a pointer to the synthesized `KernelContext` is passed around
// as a parameter and a local, so looking inside it here would report the same field once per
// value that happens to point at the struct.
static bool holdsFuncType(IRType* type)
{
    if (!type)
        return false;

    if (as<IRFuncType>(type))
        return true;

    if (auto ptrType = as<IRPtrTypeBase>(type))
        return holdsFuncType(ptrType->getValueType());

    if (auto arrayType = as<IRArrayTypeBase>(type))
        return holdsFuncType(arrayType->getElementType());

    return false;
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

    // See `isTargetWithoutFuncTypeSupport`. A function-typed value also reaches emission as a
    // local variable or a parameter that specialization could not resolve, which the
    // module-level checks for globals and `KernelContext` fields do not see:
    //
    //      functype(int) -> int local = (tid.x > 0) ? addOne : addTwo;
    //
    // A `select` between two functions is not a shape `isParamSuitableForSpecialization`
    // accepts, so both the local and the parameter it is passed to keep their function type.
    const bool rejectFuncTypedValue = isTargetWithoutFuncTypeSupport(target);

    // A single unsupported value is reachable as several insts -- the local it is stored in, a
    // parameter it is passed through, and a temporary introduced for an aggregate -- and some
    // of those are synthesized with no location. Report only the ones that can name a source
    // position, so one mistake produces one actionable error rather than several, most of which
    // point nowhere.
    auto diagnoseFuncTypedValue = [&](IRInst* inst)
    {
        if (!rejectFuncTypedValue || !holdsFuncType(inst->getFullType()))
            return;
        auto loc = inst->sourceLoc.isValid() ? inst->sourceLoc : findFirstUseLoc(inst);
        if (loc.isValid())
            sink->diagnose(Diagnostics::FuncTypeNotSupportedOnTarget{.location = loc});
    };

    if (auto firstBlock = func->getFirstBlock(); firstBlock)
    {
        for (auto param : firstBlock->getParams())
            diagnoseFuncTypedValue(param);
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
            case kIROp_DefaultConstruct:
                if (rejectOpaqueLocals)
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
    // See `isTargetWithoutFuncTypeSupport`. A function-typed global reaches here as one of two
    // shapes: `introduceExplicitGlobalContext` (C++/CUDA/Metal) moves it into a `KernelContext`
    // struct field, while WGSL keeps it a global variable. Both are written out by the ordinary
    // type-emission path, so a global that is only ever written still emits its declaration.
    const bool rejectFuncTypedValue = isTargetWithoutFuncTypeSupport(target);

    for (auto globalInst : module->getGlobalInsts())
    {
        if (rejectFuncTypedValue)
        {
            if (auto structType = as<IRStructType>(globalInst))
            {
                for (auto field : structType->getFields())
                {
                    if (holdsFuncType(field->getFieldType()))
                    {
                        // The key carries the location of the global this field replaced (see
                        // `introduceExplicitGlobalContext`). Without a location there is no
                        // source position to report and nothing the reader could act on, so the
                        // value is passed over -- which is also what keeps the function pointer
                        // `generateDllImportFuncs` synthesizes for `[DllImport]` from being
                        // rejected on the C++ targets that support it.
                        auto key = field->getKey();
                        if (key->sourceLoc.isValid())
                        {
                            sink->diagnose(Diagnostics::FuncTypeNotSupportedOnTarget{
                                .location = key->sourceLoc});
                        }
                    }
                }
            }
            else if (globalInst->getOp() == kIROp_GlobalVar)
            {
                // WGSL keeps the global rather than moving it into a context struct. Same
                // location requirement as the struct field above.
                if (holdsFuncType(globalInst->getFullType()) && globalInst->sourceLoc.isValid())
                {
                    sink->diagnose(Diagnostics::FuncTypeNotSupportedOnTarget{
                        .location = globalInst->sourceLoc});
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
