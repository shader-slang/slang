#include "slang-ir-legalize-binary-operator.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang-ir-insts.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static bool isVectorOrMatrix(IRType* type)
{
    switch (type->getOp())
    {
    case kIROp_VectorType:
    case kIROp_MatrixType:
        return true;
    default:
        return false;
    }
};

static bool isDivisionByMatrix(IRInst* inst)
{
    return (inst->getOp() == kIROp_Div) && (as<IRMatrixType>(inst->getOperand(1)->getDataType()));
}

static bool isMatrixDividedByScalar(IRInst* inst)
{
    return (inst->getOp() == kIROp_Div) && (as<IRMatrixType>(inst->getOperand(0)->getDataType())) &&
           (as<IRBasicType>(inst->getOperand(1)->getDataType()));
}

// If one operand is a composite type (vector or matrix), and the other one is a scalar
// type, then the scalar is converted to a composite type.
static void legalizeScalarOperandsToMatchComposite(IRInst* inst)
{
    if (isVectorOrMatrix(inst->getOperand(0)->getDataType()) &&
        as<IRBasicType>(inst->getOperand(1)->getDataType()))
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        IRType* compositeType = inst->getOperand(0)->getDataType();
        IRInst* scalarValue = inst->getOperand(1);
        // Retain the scalar type for shifts
        if (inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh)
        {
            auto vectorType = as<IRVectorType>(compositeType);
            compositeType =
                builder.getVectorType(scalarValue->getDataType(), vectorType->getElementCount());
        }
        auto newRhs = builder.emitMakeCompositeFromScalar(compositeType, scalarValue);
        builder.replaceOperand(inst->getOperands() + 1, newRhs);
    }
    else if (
        as<IRBasicType>(inst->getOperand(0)->getDataType()) &&
        isVectorOrMatrix(inst->getOperand(1)->getDataType()))
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        IRType* compositeType = inst->getOperand(1)->getDataType();
        IRInst* scalarValue = inst->getOperand(0);
        // Retain the scalar type for shifts
        if (inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh)
        {
            auto vectorType = as<IRVectorType>(compositeType);
            compositeType =
                builder.getVectorType(scalarValue->getDataType(), vectorType->getElementCount());
        }
        auto newLhs = builder.emitMakeCompositeFromScalar(compositeType, scalarValue);
        builder.replaceOperand(inst->getOperands(), newLhs);
    }
}

// Replaces a division by scalar operation by a multiplication.
// This is done for WGSL where matrix divided by scalar operations are not supported.
static void replaceMatrixDividedByScalarWithMul(IRInst* inst)
{
    SLANG_ASSERT(isMatrixDividedByScalar(inst));

    IRBuilder builder(inst);
    builder.setInsertBefore(inst);

    auto scalarType = inst->getOperand(1)->getDataType();
    auto newRhs =
        builder.emitDiv(scalarType, builder.getFloatValue(scalarType, 1.0), inst->getOperand(1));
    auto newOp = builder.emitMul(inst->getDataType(), inst->getOperand(0), newRhs);

    inst->replaceUsesWith(newOp);
    inst->transferDecorationsTo(newOp);
}

void legalizeBinaryOp(IRInst* inst, DiagnosticSink* sink, TargetProgram* targetProgram)
{
    IRBuilder builder(inst);
    builder.setInsertBefore(inst);
    CodeGenTarget target = targetProgram->getTargetReq()->getTarget();

    // Division by matrix is not supported on Metal and WGSL.
    if (isDivisionByMatrix(inst) && (isMetalTarget(target) || isWGPUTarget(target)))
    {
        sink->diagnose(Diagnostics::DivisionByMatrixNotSupported{.location = inst->sourceLoc});
        return;
    }

    // For shifts, ensure that the shift amount is unsigned, as required by
    // https://www.w3.org/TR/WGSL/#bit-expr.
    if (inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh)
    {
        IRInst* shiftAmount = inst->getOperand(1);
        IRType* shiftAmountType = shiftAmount->getDataType();
        if (auto shiftAmountVectorType = as<IRVectorType>(shiftAmountType))
        {
            IRType* shiftAmountElementType = shiftAmountVectorType->getElementType();
            IntInfo opIntInfo =
                getIntTypeInfo(targetProgram->getTargetReq(), shiftAmountElementType);
            if (opIntInfo.isSigned)
            {
                opIntInfo.isSigned = false;
                shiftAmountElementType = builder.getType(getIntTypeOpFromInfo(opIntInfo));
                shiftAmountVectorType = builder.getVectorType(
                    shiftAmountElementType,
                    shiftAmountVectorType->getElementCount());
                IRInst* newShiftAmount = builder.emitCast(shiftAmountVectorType, shiftAmount);
                builder.replaceOperand(inst->getOperands() + 1, newShiftAmount);
            }
        }
        else if (isIntegralType(shiftAmountType))
        {
            IntInfo opIntInfo = getIntTypeInfo(targetProgram->getTargetReq(), shiftAmountType);
            if (opIntInfo.isSigned)
            {
                opIntInfo.isSigned = false;
                shiftAmountType = builder.getType(getIntTypeOpFromInfo(opIntInfo));
                IRInst* newShiftAmount = builder.emitCast(shiftAmountType, shiftAmount);
                builder.replaceOperand(inst->getOperands() + 1, newShiftAmount);
            }
        }
    }

    // HGBC has one declared operand type per arithmetic record, so it needs the ordinary scalar
    // expansion even for matrix division. Metal preserves its native matrix/scalar division, while
    // WGSL rewrites that operation below.
    if (!isMatrixDividedByScalar(inst) || target == CodeGenTarget::HGBC)
    {
        legalizeScalarOperandsToMatchComposite(inst);
    }
    else if (isWGPUTarget(target))
    {
        // WGSL does not support matrix division by scalar, convert it to multiplication.
        replaceMatrixDividedByScalarWithMul(inst);
    }
    else
    {
        // Matrix divided by scalar is natively supported on Metal - leave it as is.
    }

    if (isIntegralType(inst->getOperand(0)->getDataType()) &&
        isIntegralType(inst->getOperand(1)->getDataType()))
    {
        // Unless the operator is a shift, and if the integer operands differ in signedness,
        // then convert the signed one to unsigned.
        // We're assuming that the cases where this is bad have already been caught by
        // common validation checks.
        IntInfo opIntInfo[2] = {
            getIntTypeInfo(targetProgram->getTargetReq(), inst->getOperand(0)->getDataType()),
            getIntTypeInfo(targetProgram->getTargetReq(), inst->getOperand(1)->getDataType())};
        bool isShift = inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh;
        bool signednessDiffers = opIntInfo[0].isSigned != opIntInfo[1].isSigned;
        if (!isShift && signednessDiffers)
        {
            int signedOpIndex = (int)opIntInfo[1].isSigned;
            opIntInfo[signedOpIndex].isSigned = false;
            auto newOp = builder.emitCast(
                builder.getType(getIntTypeOpFromInfo(opIntInfo[signedOpIndex])),
                inst->getOperand(signedOpIndex));
            builder.replaceOperand(inst->getOperands() + signedOpIndex, newOp);
        }
    }
}

/// Returns whether HGBC emits an instruction with one declared type shared by both operands.
static bool isHGBCTypedBinaryOp(IROp op)
{
    switch (op)
    {
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_IRem:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_Geq:
    case kIROp_Leq:
        return true;
    default:
        return false;
    }
}

/// Gives an HGBC shift amount exactly the type of the value being shifted.
///
/// HGBC shift records declare one operand type, so `uint3Value << 1u` becomes
/// `uint3Value << uint3(1u)` and `intValue >> uintAmount` becomes `intValue >> int(uintAmount)`.
/// Converting the amount between signed and unsigned keeps its low bits, which are the only bits
/// a masked shift uses.
static void legalizeShiftForHGBC(IRInst* inst)
{
    IRInst* value = inst->getOperand(0);
    IRInst* amount = inst->getOperand(1);
    IRType* valueType = value->getDataType();
    if (amount->getDataType() == valueType)
        return;

    IRBuilder builder(inst);
    builder.setInsertBefore(inst);
    IRType* valueScalarType = valueType;
    if (auto vectorType = as<IRVectorType>(valueType))
        valueScalarType = vectorType->getElementType();

    IRInst* newAmount = amount;
    if (auto amountVectorType = as<IRVectorType>(amount->getDataType()))
    {
        if (amountVectorType->getElementType() != valueScalarType)
        {
            newAmount = builder.emitCast(
                builder.getVectorType(valueScalarType, amountVectorType->getElementCount()),
                newAmount);
        }
    }
    else
    {
        if (amount->getDataType() != valueScalarType)
            newAmount = builder.emitCast(valueScalarType, newAmount);
        if (as<IRVectorType>(valueType))
            newAmount = builder.emitMakeCompositeFromScalar(valueType, newAmount);
    }
    builder.replaceOperand(inst->getOperands() + 1, newAmount);
}

/// Rewrites unsigned negation as subtraction from zero, which HGBC already represents.
///
/// For example, `-uintValue` becomes `0u - uintValue` and `-uint2Value` becomes
/// `uint2(0u) - uint2Value`. Both wrap modulo 2^32 exactly as GPU integer negation does, so HGBC
/// needs no unsigned variant of `OpNeg`.
static void legalizeUnsignedNegationForHGBC(IRInst* inst)
{
    IRType* type = inst->getDataType();
    IRType* scalarType = type;
    if (auto vectorType = as<IRVectorType>(type))
        scalarType = vectorType->getElementType();
    if (scalarType->getOp() != kIROp_UIntType)
        return;

    IRBuilder builder(inst);
    builder.setInsertBefore(inst);
    IRInst* zero = builder.getIntValue(scalarType, 0);
    if (as<IRVectorType>(type))
        zero = builder.emitMakeCompositeFromScalar(type, zero);
    IRInst* difference = builder.emitSub(type, zero, inst->getOperand(0));
    inst->replaceUsesWith(difference);
    inst->transferDecorationsTo(difference);
    inst->removeAndDeallocate();
}

/// Walks the linked HGBC program and normalizes each typed binary operation exactly once.
static void legalizeBinaryOpsForHGBCRec(
    IRInst* inst,
    DiagnosticSink* sink,
    TargetProgram* targetProgram)
{
    if (isHGBCTypedBinaryOp(inst->getOp()))
        legalizeBinaryOp(inst, sink, targetProgram);
    else if (inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh)
        legalizeShiftForHGBC(inst);
    else if (inst->getOp() == kIROp_Neg)
    {
        legalizeUnsignedNegationForHGBC(inst);
        return;
    }

    for (auto child : inst->getModifiableChildren())
        legalizeBinaryOpsForHGBCRec(child, sink, targetProgram);
}

void legalizeBinaryOpsForHGBC(IRModule* module, DiagnosticSink* sink, TargetProgram* targetProgram)
{
    SLANG_ASSERT(targetProgram->getTargetReq()->getTarget() == CodeGenTarget::HGBC);
    legalizeBinaryOpsForHGBCRec(module->getModuleInst(), sink, targetProgram);
}

void legalizeLogicalAndOr(IRInst* inst, TargetProgram* targetProgram)
{
    auto op = inst->getOp();
    if (op == kIROp_And || op == kIROp_Or)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Logical-AND and logical-OR takes boolean types as its operands.
        // If they are not, legalize them by casting to boolean type.
        //
        SLANG_ASSERT(inst->getOperandCount() == 2);
        for (UInt i = 0; i < 2; i++)
        {
            auto operand = inst->getOperand(i);
            auto operandDataType = operand->getDataType();

            SLANG_ASSERT(
                as<IRMatrixType>(operandDataType) || as<IRVectorType>(operandDataType) ||
                as<IRArrayType>(operandDataType) || as<IRBoolType>(operandDataType));

            if (auto vecType = as<IRVectorType>(operandDataType))
            {
                if (!as<IRBoolType>(vecType->getElementType()))
                {
                    // Cast operand to vector<bool,N>
                    auto elemCount = vecType->getElementCount();
                    auto vb = builder.getVectorType(builder.getBoolType(), elemCount);
                    auto v = builder.emitCast(vb, operand);
                    builder.replaceOperand(inst->getOperands() + i, v);
                }
            }
        }

        // Legalize the return type; mostly for SPIRV.
        // The return type of OpLogicalOr must be boolean type.
        // If not, we need to recreate the instruction with boolean return type.
        // Then, we have to cast it back to the original type so that other instrucitons that
        // use have the matching types.
        //
        auto dataType = inst->getDataType();
        auto lhs = inst->getOperand(0);
        auto rhs = inst->getOperand(1);
        IRInst* newInst = nullptr;

        SLANG_ASSERT(
            as<IRMatrixType>(dataType) || as<IRVectorType>(dataType) || as<IRBoolType>(dataType) ||
            as<IRArrayType>(dataType));
        if (auto vecType = as<IRVectorType>(dataType))
        {
            if (!as<IRBoolType>(vecType->getElementType()))
            {
                // Return type should be vector<bool,N>
                auto elemCount = vecType->getElementCount();
                auto vb = builder.getVectorType(builder.getBoolType(), elemCount);

                if (inst->getOp() == kIROp_And)
                {
                    newInst = builder.emitAnd(vb, lhs, rhs);
                }
                else
                {
                    newInst = builder.emitOr(vb, lhs, rhs);
                }
                newInst = builder.emitCast(dataType, newInst);
            }
        }
        else if (auto arrayType = as<IRArrayType>(dataType))
        {
            // Handle lowered matrices (arrays of vectors)
            auto arrayVecType = as<IRVectorType>(arrayType->getElementType());
            SLANG_ASSERT(arrayVecType);

            // At this point, lhs and rhs should already be converted to bool arrays
            auto lhsArrayType = as<IRArrayType>(lhs->getDataType());
            auto rhsArrayType = as<IRArrayType>(rhs->getDataType());
            SLANG_ASSERT(lhsArrayType && rhsArrayType);

            auto lhsVecType = as<IRVectorType>(lhsArrayType->getElementType());
            auto rhsVecType = as<IRVectorType>(rhsArrayType->getElementType());
            SLANG_ASSERT(lhsVecType && rhsVecType);

            SLANG_ASSERT(
                as<IRBoolType>(lhsVecType->getElementType()) &&
                as<IRBoolType>(rhsVecType->getElementType()));

            auto arraySize = arrayType->getElementCount();
            List<IRInst*> resultElements;

            // Extract each vector from both arrays, perform AND/OR, collect results
            for (IRIntegerValue i = 0; i < getIntVal(arraySize); i++)
            {
                auto indexVal = builder.getIntValue(builder.getIntType(), i);
                auto lhsElement = builder.emitElementExtract(lhs, indexVal);
                auto rhsElement = builder.emitElementExtract(rhs, indexVal);

                IRInst* resultElement;
                if (inst->getOp() == kIROp_And)
                {
                    resultElement =
                        builder.emitAnd(lhsElement->getDataType(), lhsElement, rhsElement);
                }
                else
                {
                    resultElement =
                        builder.emitOr(lhsElement->getDataType(), lhsElement, rhsElement);
                }
                resultElements.add(resultElement);
            }

            // Construct the result array from the individual vector results
            newInst =
                builder.emitMakeArray(dataType, getIntVal(arraySize), resultElements.getBuffer());
        }

        if (newInst && inst != newInst)
        {
            inst->replaceUsesWith(newInst);
            inst->removeAndDeallocate();
        }
    }

    for (auto child : inst->getModifiableChildren())
    {
        legalizeLogicalAndOr(child, targetProgram);
    }
}

void legalizeLogicalAndOr(IRModule* module, TargetProgram* targetProgram)
{
    legalizeLogicalAndOr(module->getModuleInst(), targetProgram);
}

} // namespace Slang
