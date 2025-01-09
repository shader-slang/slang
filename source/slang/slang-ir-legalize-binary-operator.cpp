#include "slang-ir-legalize-binary-operator.h"

#include "slang-ir-insts.h"

namespace Slang
{

void legalizeBinaryOp(IRInst* inst)
{
    // For shifts, ensure that the shift amount is unsigned, as required by
    // https://www.w3.org/TR/WGSL/#bit-expr.
    if (inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh)
    {
        IRInst* shiftAmount = inst->getOperand(1);
        IRType* shiftAmountType = shiftAmount->getDataType();
        if (auto shiftAmountVectorType = as<IRVectorType>(shiftAmountType))
        {
            IRType* shiftAmountElementType = shiftAmountVectorType->getElementType();
            IntInfo opIntInfo = getIntTypeInfo(shiftAmountElementType);
            if (opIntInfo.isSigned)
            {
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
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
            IntInfo opIntInfo = getIntTypeInfo(shiftAmountType);
            if (opIntInfo.isSigned)
            {
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                opIntInfo.isSigned = false;
                shiftAmountType = builder.getType(getIntTypeOpFromInfo(opIntInfo));
                IRInst* newShiftAmount = builder.emitCast(shiftAmountType, shiftAmount);
                builder.replaceOperand(inst->getOperands() + 1, newShiftAmount);
            }
        }
    }

    auto isVectorOrMatrix = [](IRType* type)
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
            compositeType = builder.getVectorType(
                scalarValue->getDataType(),
                vectorType->getElementCount());
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
            compositeType = builder.getVectorType(
                scalarValue->getDataType(),
                vectorType->getElementCount());
        }
        auto newLhs = builder.emitMakeCompositeFromScalar(compositeType, scalarValue);
        builder.replaceOperand(inst->getOperands(), newLhs);
    }
    else if (
        isIntegralType(inst->getOperand(0)->getDataType()) &&
        isIntegralType(inst->getOperand(1)->getDataType()))
    {
        // Unless the operator is a shift, and if the integer operands differ in signedness,
        // then convert the signed one to unsigned.
        // We're assuming that the cases where this is bad have already been caught by
        // common validation checks.
        IntInfo opIntInfo[2] = {
            getIntTypeInfo(inst->getOperand(0)->getDataType()),
            getIntTypeInfo(inst->getOperand(1)->getDataType())};
        bool isShift = inst->getOp() == kIROp_Lsh || inst->getOp() == kIROp_Rsh;
        bool signednessDiffers = opIntInfo[0].isSigned != opIntInfo[1].isSigned;
        if (!isShift && signednessDiffers)
        {
            int signedOpIndex = (int)opIntInfo[1].isSigned;
            opIntInfo[signedOpIndex].isSigned = false;
            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            auto newOp = builder.emitCast(
                builder.getType(getIntTypeOpFromInfo(opIntInfo[signedOpIndex])),
                inst->getOperand(signedOpIndex));
            builder.replaceOperand(inst->getOperands() + signedOpIndex, newOp);
        }
    }
}

} // namespace Slang
