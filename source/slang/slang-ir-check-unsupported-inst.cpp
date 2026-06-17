#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

// Find an opaque "handle" type that SPIR-V forbids from being stored to or
// loaded from (VUID-StandaloneSpirv-OpTypeImage-06924): images/textures,
// samplers, sampled images, subpass inputs, and acceleration structures. These
// map to `OpTypeImage`/`OpTypeSampler`/`OpTypeSampledImage`/
// `OpTypeAccelerationStructureKHR`, none of which may live in a function-local
// variable. Recurses into struct fields and array elements since storing an
// aggregate that contains such a handle has the same problem. Returns the leaf
// handle type if found (for diagnostics), or null otherwise.
//
// Note: this is deliberately narrower than `isOpaqueType`. Buffer-backed
// resources (structured / byte-address buffers) and pointers lower to
// pointers and *can* be selected through control flow using SPIR-V variable
// pointers, so they must not be rejected here. `RayQuery`/`HitObject` are also
// excluded as they are legitimately declared as locals.
static IRType* findUnstorableOpaqueHandleType(IRType* type)
{
    if (!type)
        return nullptr;

    if (as<IRResourceTypeBase>(type) || as<IRSamplerStateTypeBase>(type) ||
        as<IRSubpassInputType>(type) || type->getOp() == kIROp_RaytracingAccelerationStructureType)
    {
        return type;
    }

    if (auto arrayType = as<IRArrayTypeBase>(type))
        return findUnstorableOpaqueHandleType(arrayType->getElementType());

    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (auto found = findUnstorableOpaqueHandleType(field->getFieldType()))
                return found;
        }
    }

    return nullptr;
}

void checkUnsupportedInst(TargetRequest* target, IRFunc* func, DiagnosticSink* sink)
{
    // SPIR-V cannot place an image/sampler/subpass/acceleration-structure handle
    // in a function-local variable: it forbids OpStore/OpLoad (and OpPhi) of such
    // a handle. A local variable of one of those types reaching here is invalid
    // output we cannot legalize yet (issue #10526, typically from selecting or
    // returning a resource through control flow); reject it with a diagnostic
    // rather than emitting invalid SPIR-V.
    const bool rejectOpaqueLocals = isKhronosTarget(target);

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
                        sink->diagnose(Diagnostics::OpaqueTypeInLocalVariableNotAllowedOnSpirv{
                            .type = handleType,
                            .location = loc});
                    }
                }
                break;
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
