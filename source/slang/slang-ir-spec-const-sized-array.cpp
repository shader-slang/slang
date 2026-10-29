#include "slang-ir-spec-const-sized-array.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

/// Return true if `type` is an array whose size is a specialization constant, or an expression
/// over one.
static bool isSpecConstSizedArrayType(IRType* type)
{
    auto arrayType = as<IRArrayType>(type);
    return arrayType && isSpecConstRateType(arrayType->getElementCount()->getFullType());
}

/// Return true if `type` or any of its element array types is sized by a specialization constant.
/// This is true for `float[kTile]` and `float[kTile][4]`, and false for `float[4]` and for a
/// struct that has a `float values[kTile]` field.
static bool hasSpecConstSizedArrayLayer(IRType* type)
{
    while (auto arrayType = as<IRArrayTypeBase>(type))
    {
        if (isSpecConstSizedArrayType(arrayType))
            return true;
        type = arrayType->getElementType();
    }
    return false;
}

struct SpecConstSizedArrayCheckContext
{
    CodeGenTarget target;
    DiagnosticSink* sink;

    void checkInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_GlobalVar:
        case kIROp_GlobalParam:
        case kIROp_Var:
            checkDeclaration(inst, inst->getDataType(), inst->sourceLoc);
            break;
        case kIROp_StructField:
            {
                // The lowered struct field carries no location of its own; its key has the
                // location of the field declaration.
                auto field = as<IRStructField>(inst);
                checkDeclaration(inst, field->getFieldType(), field->getKey()->sourceLoc);
            }
            break;
        case kIROp_Func:
            for (auto param : as<IRFunc>(inst)->getParams())
                checkDeclaration(param, param->getDataType(), param->sourceLoc);
            break;
        default:
            break;
        }

        for (auto child : inst->getChildren())
            checkInst(child);
    }

    void checkDeclaration(IRInst* decl, IRType* declType, SourceLoc location)
    {
        // A variable, and an `out` or `inout` parameter, owns the storage its pointer type
        // points to.
        IRType* storageType = declType;
        if (auto ptrType = as<IRPtrTypeBase>(declType))
            storageType = ptrType->getValueType();

        if (!hasSpecConstSizedArrayLayer(storageType))
            return;
        if (isWGPUTarget(target) && as<IRGlobalVar>(decl) &&
            as<IRGroupSharedRate>(decl->getRate()) && isSpecConstSizedArrayType(storageType) &&
            !hasSpecConstSizedArrayLayer(as<IRArrayType>(storageType)->getElementType()))
        {
            // WGSL allows an override-sized array only as the store type of a `var<workgroup>`;
            // its element type needs a size fixed at shader creation:
            // https://www.w3.org/TR/WGSL/#array-types
            return;
        }

        sink->diagnose(Diagnostics::SpecializationConstantSizedArrayNotSupported{
            .target = target,
            .location = location,
        });
    }
};

void checkSpecializationConstantSizedArrays(
    IRModule* module,
    CodeGenTarget target,
    DiagnosticSink* sink)
{
    if (isKhronosTarget(target))
        return;

    SpecConstSizedArrayCheckContext context{target, sink};
    context.checkInst(module->getModuleInst());
}

/// Return the ordinary opcode for the operation that `constexprOp` performs, or `kIROp_Invalid` if
/// `constexprOp` is not one of the `constexpr*` opcodes.
static IROp getOrdinaryOpForConstexprOp(IROp constexprOp)
{
    switch (constexprOp)
    {
    case kIROp_ConstexprAdd:
        return kIROp_Add;
    case kIROp_ConstexprSub:
        return kIROp_Sub;
    case kIROp_ConstexprMul:
        return kIROp_Mul;
    case kIROp_ConstexprNeg:
        return kIROp_Neg;
    case kIROp_ConstexprDiv:
        return kIROp_Div;
    case kIROp_ConstexprIRem:
        return kIROp_IRem;
    case kIROp_ConstexprShl:
        return kIROp_Lsh;
    case kIROp_ConstexprShr:
        return kIROp_Rsh;
    case kIROp_ConstexprBitAnd:
        return kIROp_BitAnd;
    case kIROp_ConstexprBitOr:
        return kIROp_BitOr;
    case kIROp_ConstexprBitXor:
        return kIROp_BitXor;
    case kIROp_ConstexprBitNot:
        return kIROp_BitNot;
    case kIROp_ConstexprNot:
        return kIROp_Not;
    case kIROp_ConstexprEql:
        return kIROp_Eql;
    case kIROp_ConstexprNeq:
        return kIROp_Neq;
    case kIROp_ConstexprGreater:
        return kIROp_Greater;
    case kIROp_ConstexprLess:
        return kIROp_Less;
    case kIROp_ConstexprGeq:
        return kIROp_Geq;
    case kIROp_ConstexprLeq:
        return kIROp_Leq;
    case kIROp_ConstexprAnd:
        return kIROp_And;
    case kIROp_ConstexprOr:
        return kIROp_Or;
    case kIROp_ConstexprSelect:
        return kIROp_Select;
    case kIROp_ConstexprIntCast:
        return kIROp_IntCast;
    case kIROp_ConstexprCastIntToFloat:
        return kIROp_CastIntToFloat;
    case kIROp_ConstexprCastFloatToInt:
        return kIROp_CastFloatToInt;
    case kIROp_ConstexprFloatCast:
        return kIROp_FloatCast;
    // `lowerEnumType` has already replaced each enum type with its tag type, so the enum casts
    // are integer casts.
    case kIROp_ConstexprCastIntToEnum:
    case kIROp_ConstexprCastEnumToInt:
    case kIROp_ConstexprEnumCast:
        return kIROp_IntCast;
    default:
        return kIROp_Invalid;
    }
}

void lowerConstexprInstsToOrdinaryInsts(IRModule* module)
{
    // `constexpr*` instructions are hoistable, so they all live at module scope.
    List<IRInst*> constexprInsts;
    for (auto inst : module->getGlobalInsts())
    {
        if (getOrdinaryOpForConstexprOp(inst->getOp()) != kIROp_Invalid)
            constexprInsts.add(inst);
    }

    IRBuilder builder(module);
    for (auto inst : constexprInsts)
    {
        builder.setInsertBefore(inst);
        List<IRInst*> operands;
        for (UInt i = 0; i < inst->getOperandCount(); i++)
            operands.add(inst->getOperand(i));
        auto ordinaryInst = builder.emitIntrinsicInst(
            inst->getFullType(),
            getOrdinaryOpForConstexprOp(inst->getOp()),
            operands.getCount(),
            operands.getBuffer());
        inst->replaceUsesWith(ordinaryInst);
        inst->removeAndDeallocate();
    }
}

} // namespace Slang
