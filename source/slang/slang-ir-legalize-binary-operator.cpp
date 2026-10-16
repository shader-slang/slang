#include "slang-ir-legalize-binary-operator.h"

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-matrix-types.h"
#include "slang-ir-util.h"
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

// Return whether `type` is a matrix that the target emits as a matrix. An int/uint/bool matrix is
// still a matrix here but becomes an array of row vectors later, and its operations become
// operations on those rows, which Metal and WGSL support.
static bool isNativeMatrix(TargetProgram* targetProgram, IRType* type)
{
    auto matrixType = as<IRMatrixType>(type);
    return matrixType && !isMatrixTypeLoweredToArray(targetProgram->getTargetReq(), matrixType);
}

static bool isDivisionByMatrix(IRInst* inst, TargetProgram* targetProgram)
{
    return (inst->getOp() == kIROp_Div) &&
           isNativeMatrix(targetProgram, inst->getOperand(1)->getDataType());
}

static bool isMatrixDividedByScalar(IRInst* inst, TargetProgram* targetProgram)
{
    return (inst->getOp() == kIROp_Div) &&
           isNativeMatrix(targetProgram, inst->getOperand(0)->getDataType()) &&
           (as<IRBasicType>(inst->getOperand(1)->getDataType()));
}

// Return a composite type with the shape of `compositeType` and the element type `scalarType`.
// For example, `matrix<int,2,3>` and `uint` give `matrix<uint,2,3>`.
static IRType* getCompositeTypeWithElementType(
    IRBuilder& builder,
    IRType* compositeType,
    IRType* scalarType)
{
    if (auto matrixType = as<IRMatrixType>(compositeType))
    {
        return builder.getMatrixType(
            scalarType,
            matrixType->getRowCount(),
            matrixType->getColumnCount(),
            matrixType->getLayout());
    }
    auto vectorType = as<IRVectorType>(compositeType);
    SLANG_ASSERT(vectorType);
    return builder.getVectorType(scalarType, vectorType->getElementCount());
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
            compositeType =
                getCompositeTypeWithElementType(builder, compositeType, scalarValue->getDataType());
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
            compositeType =
                getCompositeTypeWithElementType(builder, compositeType, scalarValue->getDataType());
        auto newLhs = builder.emitMakeCompositeFromScalar(compositeType, scalarValue);
        builder.replaceOperand(inst->getOperands(), newLhs);
    }
}

// Replaces a division by scalar operation by a multiplication.
// This is done for WGSL where matrix divided by scalar operations are not supported.
static void replaceMatrixDividedByScalarWithMul(IRInst* inst, TargetProgram* targetProgram)
{
    SLANG_ASSERT(isMatrixDividedByScalar(inst, targetProgram));

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

    // Division by matrix is not supported on Metal and WGSL.
    if (isDivisionByMatrix(inst, targetProgram))
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
        if (as<IRVectorType>(shiftAmountType) || as<IRMatrixType>(shiftAmountType))
        {
            IRType* shiftAmountElementType =
                getMatrixElementType(getVectorElementType(shiftAmountType));
            IntInfo opIntInfo =
                getIntTypeInfo(targetProgram->getTargetReq(), shiftAmountElementType);
            if (opIntInfo.isSigned)
            {
                opIntInfo.isSigned = false;
                auto unsignedShiftAmountType = getCompositeTypeWithElementType(
                    builder,
                    shiftAmountType,
                    builder.getType(getIntTypeOpFromInfo(opIntInfo)));
                IRInst* newShiftAmount = builder.emitCast(unsignedShiftAmountType, shiftAmount);
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

    // For matrix divided by scalar operations, do not convert scalar divisor to dividend's matrix
    // type. Division by matrix is not supported on Metal and WGSL.
    if (!isMatrixDividedByScalar(inst, targetProgram))
    {
        legalizeScalarOperandsToMatchComposite(inst);
    }
    else if (isWGPUTarget(targetProgram->getTargetReq()->getTarget()))
    {
        // WGSL does not support matrix division by scalar, convert it to multiplication.
        replaceMatrixDividedByScalarWithMul(inst, targetProgram);
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
                as<IRBoolType>(operandDataType));

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
            as<IRMatrixType>(dataType) || as<IRVectorType>(dataType) || as<IRBoolType>(dataType));
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
