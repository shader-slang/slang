#include "slang-ir-check-unsupported-inst.h"

#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-target.h"

namespace Slang
{

static bool _targetSupportsCoherentMemoryQualifier(TargetRequest* target)
{
    return target && (isD3DTarget(target) || isKhronosTarget(target));
}

static bool _hasCoherentMemoryQualifierDecoration(IRInst* inst)
{
    if (auto decoration = inst->findDecoration<IRMemoryQualifierSetDecoration>())
    {
        auto flags = decoration->getMemoryQualifierBit();
        return (flags & MemoryQualifierSetModifier::Flags::kCoherent) != 0;
    }
    return false;
}

static void _checkCoherentMemoryQualifierDecorationTargetSupport(
    IRInst* inst,
    TargetRequest* target,
    DiagnosticSink* sink)
{
    if (_targetSupportsCoherentMemoryQualifier(target))
        return;
    if (!_hasCoherentMemoryQualifierDecoration(inst))
        return;

    sink->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
        .operation = "coherent memory qualifier",
        .location = findFirstUseLoc(inst)});
}

static void _checkCoherentMemoryQualifierTargetSupport(
    IRType* type,
    TargetRequest* target,
    DiagnosticSink* sink,
    HashSet<IRInst*>& checkedTypes)
{
    if (_targetSupportsCoherentMemoryQualifier(target))
        return;

    while (type)
    {
        if (!checkedTypes.add(type))
            return;

        if ((getMemoryQualifierSetAttrFlags(type) & MemoryQualifierSetModifier::Flags::kCoherent) !=
            0)
        {
            sink->diagnose(Diagnostics::UnsupportedTargetIntrinsic{
                .operation = "coherent memory qualifier",
                .location = findFirstUseLoc(type)});
        }

        auto unwrappedType = unwrapAttributedTypeAndArray(type);
        if (unwrappedType != type)
        {
            type = unwrappedType;
            continue;
        }

        if (auto ptrType = as<IRPtrTypeBase, IRDynamicCastBehavior::NoUnwrap>(type))
        {
            type = ptrType->getValueType();
            continue;
        }
        if (auto rateQualifiedType = as<IRRateQualifiedType, IRDynamicCastBehavior::NoUnwrap>(type))
        {
            type = rateQualifiedType->getValueType();
            continue;
        }
        if (auto descriptorHandleType =
                as<IRDescriptorHandleType, IRDynamicCastBehavior::NoUnwrap>(type))
        {
            type = descriptorHandleType->getResourceType();
            continue;
        }
        return;
    }
}

static void _checkCoherentMemoryQualifierTargetSupport(
    TargetRequest* target,
    IRFunc* func,
    DiagnosticSink* sink,
    HashSet<IRInst*>& checkedTypes)
{
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            _checkCoherentMemoryQualifierDecorationTargetSupport(inst, target, sink);
            _checkCoherentMemoryQualifierTargetSupport(
                inst->getDataType(),
                target,
                sink,
                checkedTypes);
        }
    }
}

void checkUnsupportedCoherentMemoryQualifier(
    IRModule* module,
    TargetRequest* target,
    DiagnosticSink* sink)
{
    // Run before source-target legalization can erase coherent decorations or attributed types.
    HashSet<IRInst*> checkedTypes;

    for (auto globalInst : module->getGlobalInsts())
    {
        _checkCoherentMemoryQualifierDecorationTargetSupport(globalInst, target, sink);

        if (auto type = as<IRType, IRDynamicCastBehavior::NoUnwrap>(globalInst))
        {
            _checkCoherentMemoryQualifierTargetSupport(type, target, sink, checkedTypes);
        }
        else
        {
            _checkCoherentMemoryQualifierTargetSupport(
                globalInst->getDataType(),
                target,
                sink,
                checkedTypes);
        }

        switch (globalInst->getOp())
        {
        case kIROp_Func:
            _checkCoherentMemoryQualifierTargetSupport(
                target,
                as<IRFunc>(globalInst),
                sink,
                checkedTypes);
            break;
        case kIROp_Generic:
            {
                auto generic = as<IRGeneric>(globalInst);
                auto innerFunc = as<IRFunc>(findGenericReturnVal(generic));
                if (innerFunc)
                {
                    _checkCoherentMemoryQualifierTargetSupport(
                        target,
                        innerFunc,
                        sink,
                        checkedTypes);
                }
                break;
            }
        default:
            break;
        }
    }
}

void checkUnsupportedInst(TargetRequest* target, IRFunc* func, DiagnosticSink* sink)
{
    SLANG_UNUSED(target);
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
