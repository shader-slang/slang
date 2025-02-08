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

void legalizeLogicalAndOr(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_And:
    case kIROp_Or:
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
                else if (!as<IRBoolType>(operandDataType))
                {
                    // Cast operand to bool
                    auto s = builder.emitCast(builder.getBoolType(), operand);
                    builder.replaceOperand(inst->getOperands() + i, s);
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
            else if (!as<IRBoolType>(dataType))
            {
                // Return type should be bool
                if (inst->getOp() == kIROp_And)
                {
                    newInst = builder.emitAnd(builder.getBoolType(), lhs, rhs);
                }
                else
                {
                    newInst = builder.emitOr(builder.getBoolType(), lhs, rhs);
                }
                newInst = builder.emitCast(dataType, newInst);
            }

            if (newInst && inst != newInst)
            {
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
            }
        }
        break;
    }

    for (auto child : inst->getModifiableChildren())
    {
        legalizeLogicalAndOr(child);
    }
}

} // namespace Slang
