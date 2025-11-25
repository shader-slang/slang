#include "slang-ir-legalize-matrix-types.h"

#include "slang-compiler.h"
#include "slang-ir-insts-enum.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct MatrixTypeLoweringContext
{
    TargetProgram* targetProgram;
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    Dictionary<IRInst*, IRInst*> replacements;

    MatrixTypeLoweringContext(TargetProgram* targetProgram, IRModule* module)
        : targetProgram(targetProgram), module(module), workList(module), workListSet(module)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    bool shouldLowerTarget()
    {
        auto target = targetProgram->getTargetReq()->getTarget();
        switch (target)
        {
        case CodeGenTarget::SPIRV:
        case CodeGenTarget::SPIRVAssembly:
        case CodeGenTarget::GLSL:
        case CodeGenTarget::WGSL:
        case CodeGenTarget::WGSLSPIRV:
        case CodeGenTarget::WGSLSPIRVAssembly:
        case CodeGenTarget::Metal:
        case CodeGenTarget::MetalLib:
        case CodeGenTarget::MetalLibAssembly:
            return true;
        default:
            return false;
        }
    }

    bool shouldLowerMatrixType(IRMatrixType* matrixType)
    {
        if (!shouldLowerTarget())
            return false;

        auto elementType = matrixType->getElementType();
        return as<IRBoolType>(elementType) || as<IRUIntType>(elementType) ||
               as<IRIntType>(elementType);
    }

    IRInst* legalizeMatrixTypeDeclaration(IRInst* inst)
    {
        auto matrixType = as<IRMatrixType>(inst);
        if (shouldLowerMatrixType(matrixType))
        {
            // Lower matrix<T, R, C> to T[R][C] (array of R vectors of length C)
            auto elementType = matrixType->getElementType();
            auto rowCount = matrixType->getRowCount();
            auto columnCount = matrixType->getColumnCount();

            IRBuilder builder(matrixType);
            builder.setInsertBefore(matrixType);

            // Create vector type for columns: vector<T, C>
            auto vectorType = builder.getVectorType(elementType, columnCount);

            // Create array type for rows: vector<T, C>[R]
            auto arrayType = builder.getArrayType(vectorType, rowCount);

            return arrayType;
        }
        return inst;
    }

    IRInst* legalizeMakeMatrix(IRInst* inst)
    {
        auto makeMatrix = as<IRMakeMatrix>(inst);
        auto matrixType = as<IRMatrixType>(makeMatrix->getDataType());

        SLANG_ASSERT(matrixType && "Matrix type is expected");
        SLANG_ASSERT(
            shouldLowerMatrixType(matrixType) && "Matrix type is expected to need legalization");

        // Lower makeMatrix to makeArray of makeVectors
        auto elementType = matrixType->getElementType();
        auto rowCount = as<IRIntLit>(matrixType->getRowCount());
        auto columnCount = as<IRIntLit>(matrixType->getColumnCount());

        SLANG_ASSERT(
            rowCount && columnCount &&
            "Matrix dimensions must be compile-time constants for lowering");

        IRBuilder builder(makeMatrix);
        builder.setInsertBefore(makeMatrix);

        // Create vector type for rows: vector<T, C>
        auto vectorType = builder.getVectorType(elementType, columnCount);

        // Create array type: vector<T, C>[R]
        auto arrayType = builder.getArrayType(vectorType, rowCount);

        // Group operands into rows and create vectors
        List<IRInst*> rowVectors;
        UInt operandIndex = 0;

        // Assert that we have the expected number of operands
        if (makeMatrix->getOperandCount() == UInt(rowCount->getValue() * columnCount->getValue()))
        {
            // Each operand is a matrix element
            for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
            {
                List<IRInst*> rowElements;
                for (IRIntegerValue col = 0; col < columnCount->getValue(); col++)
                {
                    SLANG_ASSERT(
                        operandIndex < makeMatrix->getOperandCount() &&
                        "Operand index out of bounds");
                    rowElements.add(getReplacement(makeMatrix->getOperand(operandIndex)));
                    operandIndex++;
                }

                SLANG_ASSERT(
                    rowElements.getCount() == columnCount->getValue() &&
                    "Row elements count must match column count");
                auto rowVector = builder.emitMakeVector(vectorType, rowElements);
                rowVectors.add(rowVector);
            }
        }
        else if (makeMatrix->getOperandCount() == UInt(rowCount->getValue()))
        {
            // Each operand is a vector with width columnCount->getValue().
            for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
            {
                auto rowVector = getReplacement(makeMatrix->getOperand(row));
                auto vecType = as<IRVectorType>(rowVector->getDataType());
                SLANG_ASSERT(
                    getIntVal(vecType->getElementCount()) == columnCount->getValue() &&
                    "Row elements count must match column count");
                rowVectors.add(rowVector);
            }
        }
        else
            SLANG_ASSERT_FAILURE("makeMatrix operand count must match matrix dimensions");

        SLANG_ASSERT(
            rowVectors.getCount() == rowCount->getValue() &&
            "Row vectors count must match matrix row count");
        return builder.emitMakeArray(arrayType, rowVectors.getCount(), rowVectors.getBuffer());
    }

    IRInst* legalizeMakeMatrixFromScalar(IRInst* inst)
    {
        auto matrixType = as<IRMatrixType>(inst->getDataType());

        SLANG_ASSERT(matrixType && "Matrix type is expected");
        SLANG_ASSERT(
            shouldLowerMatrixType(matrixType) && "Matrix type is expected to need legalization");

        // Lower makeMatrixFromScalar to makeArray of makeVectors from scalar
        auto elementType = matrixType->getElementType();
        auto rowCount = as<IRIntLit>(matrixType->getRowCount());
        auto columnCount = as<IRIntLit>(matrixType->getColumnCount());

        SLANG_ASSERT(
            rowCount && columnCount &&
            "Matrix dimensions must be compile-time constants for lowering");

        SLANG_ASSERT(
            inst->getOperandCount() == 1 && "makeMatrixFromScalar should have exactly one operand");

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Get the scalar operand
        auto scalarOperand = getReplacement(inst->getOperand(0));

        // Create vector type for rows: vector<T, C>
        auto vectorType = builder.getVectorType(elementType, columnCount);

        // Create array type: vector<T, C>[R]
        auto arrayType = builder.getArrayType(vectorType, rowCount);

        // Create a vector from the scalar (replicated C times)
        List<IRInst*> vectorElements;
        for (IRIntegerValue col = 0; col < columnCount->getValue(); col++)
        {
            vectorElements.add(scalarOperand);
        }
        auto rowVector = builder.emitMakeVector(vectorType, vectorElements);

        // Create array with R copies of the same vector
        List<IRInst*> rowVectors;
        for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
        {
            rowVectors.add(rowVector);
        }

        SLANG_ASSERT(
            rowVectors.getCount() == rowCount->getValue() &&
            "Row vectors count must match matrix row count");
        return builder.emitMakeArray(arrayType, rowVectors.getCount(), rowVectors.getBuffer());
    }

    IRInst* legalizeMatrixMatrixBinaryOperation(
        IRBuilder& builder,
        IRInst* legalizedA,
        IRInst* legalizedB,
        IRMatrixType* resultMatrixType,
        IROp binaryOp)
    {
        auto elementType = resultMatrixType->getElementType();
        auto rowCount = as<IRIntLit>(resultMatrixType->getRowCount());
        auto columnCount = as<IRIntLit>(resultMatrixType->getColumnCount());

        SLANG_ASSERT(
            rowCount && columnCount &&
            "Matrix dimensions must be compile-time constants for lowering");

        // Create vector type for rows: vector<T, C>
        auto vectorType = builder.getVectorType(elementType, columnCount);

        // Create array type: vector<T, C>[R]
        auto arrayType = builder.getArrayType(vectorType, rowCount);

        // Extract vectors from both arrays and apply binary operation
        List<IRInst*> resultVectors;

        for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
        {
            // Extract the row vector from each operand array
            auto rowIndexInst = builder.getIntValue(builder.getIntType(), row);
            auto vectorA = builder.emitElementExtract(legalizedA, rowIndexInst);
            auto vectorB = builder.emitElementExtract(legalizedB, rowIndexInst);

            // Apply the binary operation to the vectors
            IRInst* args[] = {vectorA, vectorB};
            auto resultVector = builder.emitIntrinsicInst(vectorType, binaryOp, 2, args);

            resultVectors.add(resultVector);
        }

        // Create the result array from the vectors
        return builder.emitMakeArray(
            arrayType,
            resultVectors.getCount(),
            resultVectors.getBuffer());
    }


    template<bool matrixIsFirst>
    IRInst* legalizeMatrixMixedBinaryOperation(
        IRBuilder& builder,
        IRInst* legalizedMatrix,
        IRInst* legalizedOther,
        IRMatrixType* resultMatrixType,
        IROp binaryOp)
    {
        // Verify that the other operand is either a vector or scalar type
        auto otherType = legalizedOther->getDataType();
        auto otherVectorType = as<IRVectorType>(otherType);
        auto otherBasicType = as<IRBasicType>(otherType);
        SLANG_ASSERT(
            (otherVectorType || otherBasicType) && "Other operand must be vector or scalar type");

        auto elementType = resultMatrixType->getElementType();
        auto rowCount = as<IRIntLit>(resultMatrixType->getRowCount());
        auto columnCount = as<IRIntLit>(resultMatrixType->getColumnCount());

        SLANG_ASSERT(
            rowCount && columnCount &&
            "Matrix dimensions must be compile-time constants for lowering");

        // Create vector type for rows: vector<T, C>
        auto vectorType = builder.getVectorType(elementType, columnCount);

        // Create array type: vector<T, C>[R]
        auto arrayType = builder.getArrayType(vectorType, rowCount);

        // Extract vectors from matrix array and apply binary operation with other operand
        List<IRInst*> resultVectors;

        for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
        {
            // Extract the row vector from matrix array
            auto rowIndexInst = builder.getIntValue(builder.getIntType(), row);
            auto matrixRowVector = builder.emitElementExtract(legalizedMatrix, rowIndexInst);

            // Apply the binary operation between matrix row vector and other operand
            IRInst* args[2];
            if constexpr (matrixIsFirst)
            {
                args[0] = matrixRowVector;
                args[1] = legalizedOther;
            }
            else
            {
                args[0] = legalizedOther;
                args[1] = matrixRowVector;
            }
            auto resultVector = builder.emitIntrinsicInst(vectorType, binaryOp, 2, args);

            resultVectors.add(resultVector);
        }

        // Create the result array from the vectors
        return builder.emitMakeArray(
            arrayType,
            resultVectors.getCount(),
            resultVectors.getBuffer());
    }

    IRInst* legalizeBinaryOperation(IRInst* inst, IROp binaryOp)
    {
        IRInst* opdA = inst->getOperand(0);
        IRInst* opdB = inst->getOperand(1);

        // Check what types we're dealing with
        auto typeA = opdA->getDataType();
        auto typeB = opdB->getDataType();

        auto matrixTypeA = as<IRMatrixType>(typeA);
        auto matrixTypeB = as<IRMatrixType>(typeB);

        bool shouldLowerA = matrixTypeA && shouldLowerMatrixType(matrixTypeA);
        bool shouldLowerB = matrixTypeB && shouldLowerMatrixType(matrixTypeB);

        // Get the result matrix type to determine dimensions
        auto resultMatrixType = as<IRMatrixType>(inst->getDataType());
        SLANG_ASSERT(resultMatrixType && "Binary operation should have matrix result type");
        SLANG_ASSERT(
            shouldLowerMatrixType(resultMatrixType) &&
            "Result matrix type should need legalization");

        // Create IRBuilder at the top level
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Get legalized operands once
        IRInst* legalizedA = getReplacement(opdA);
        IRInst* legalizedB = getReplacement(opdB);

        if (shouldLowerA && shouldLowerB)
        {
            return legalizeMatrixMatrixBinaryOperation(
                builder,
                legalizedA,
                legalizedB,
                resultMatrixType,
                binaryOp);
        }
        else if (shouldLowerA && !shouldLowerB)
        {
            return legalizeMatrixMixedBinaryOperation<true>(
                builder,
                legalizedA,
                legalizedB,
                resultMatrixType,
                binaryOp);
        }
        else if (!shouldLowerA && shouldLowerB)
        {
            return legalizeMatrixMixedBinaryOperation<false>(
                builder,
                legalizedB,
                legalizedA,
                resultMatrixType,
                binaryOp);
        }

        // Neither operand is a matrix that needs lowering, shouldn't reach here
        SLANG_UNREACHABLE("legalizeBinaryOperation called but no matrix operand needs lowering");
    }

    IRInst* legalizeComparisonOperation(IRInst* inst, IROp comparisonOp)
    {
        IRInst* opdA = inst->getOperand(0);
        IRInst* opdB = inst->getOperand(1);

        // Check what types we're dealing with
        auto typeA = opdA->getDataType();
        auto typeB = opdB->getDataType();

        auto matrixTypeA = as<IRMatrixType>(typeA);
        auto matrixTypeB = as<IRMatrixType>(typeB);

        bool shouldLowerA = matrixTypeA && shouldLowerMatrixType(matrixTypeA);
        bool shouldLowerB = matrixTypeB && shouldLowerMatrixType(matrixTypeB);

        // Only matrix-matrix comparisons are supported
        SLANG_ASSERT(
            shouldLowerA && shouldLowerB &&
            "Comparison operations only supported between matrices that need lowering");

        // Create IRBuilder at the top level
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Get legalized operands
        IRInst* legalizedA = getReplacement(opdA);
        IRInst* legalizedB = getReplacement(opdB);

        auto rowCount = as<IRIntLit>(matrixTypeA->getRowCount());
        auto columnCount = as<IRIntLit>(matrixTypeA->getColumnCount());

        SLANG_ASSERT(
            rowCount && columnCount &&
            "Matrix dimensions must be compile-time constants for lowering");

        // Create boolean vector type for rows: vector<bool, C>
        auto boolType = builder.getBoolType();
        auto boolVectorType = builder.getVectorType(boolType, columnCount);

        // Create array type: vector<bool, C>[R]
        auto boolArrayType = builder.getArrayType(boolVectorType, rowCount);

        // Extract vectors from both arrays and apply comparison operation
        List<IRInst*> resultVectors;

        for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
        {
            // Extract the row vector from each operand array
            auto rowIndexInst = builder.getIntValue(builder.getIntType(), row);
            auto vectorA = builder.emitElementExtract(legalizedA, rowIndexInst);
            auto vectorB = builder.emitElementExtract(legalizedB, rowIndexInst);

            // Apply the comparison operation to the vectors
            IRInst* args[] = {vectorA, vectorB};
            auto resultVector = builder.emitIntrinsicInst(boolVectorType, comparisonOp, 2, args);

            resultVectors.add(resultVector);
        }

        // Create the result array from the vectors
        return builder.emitMakeArray(
            boolArrayType,
            resultVectors.getCount(),
            resultVectors.getBuffer());
    }

    IRInst* legalizeUnaryOperation(IRInst* inst, IROp unaryOp, IRMatrixType* resultMatrixType)
    {
        IRInst* operand = inst->getOperand(0);

        // Get the legalized operand (should be an array of vectors)
        IRInst* legalizedOperand = getReplacement(operand);

        auto elementType = resultMatrixType->getElementType();
        auto rowCount = as<IRIntLit>(resultMatrixType->getRowCount());
        auto columnCount = as<IRIntLit>(resultMatrixType->getColumnCount());

        SLANG_ASSERT(
            rowCount && columnCount &&
            "Matrix dimensions must be compile-time constants for lowering");

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // Create vector type for rows: vector<T, C>
        auto vectorType = builder.getVectorType(elementType, columnCount);

        // Create array type: vector<T, C>[R]
        auto arrayType = builder.getArrayType(vectorType, rowCount);

        // Extract vectors from array and apply unary operation
        List<IRInst*> resultVectors;

        for (IRIntegerValue row = 0; row < rowCount->getValue(); row++)
        {
            // Extract the row vector from operand array
            auto rowIndexInst = builder.getIntValue(builder.getIntType(), row);
            auto vector = builder.emitElementExtract(legalizedOperand, rowIndexInst);

            // Apply the unary operation to the vector
            IRInst* args[] = {vector};
            auto resultVector = builder.emitIntrinsicInst(vectorType, unaryOp, 1, args);

            resultVectors.add(resultVector);
        }

        // Create the result array from the vectors if the result matrix type needs lowering,
        // otherwise create the result matrix from the vectors
        if (shouldLowerMatrixType(resultMatrixType))
        {
            return builder.emitMakeArray(
                arrayType,
                resultVectors.getCount(),
                resultVectors.getBuffer());
        }
        else
        {
            return builder.emitMakeMatrix(
                resultMatrixType,
                resultVectors.getCount(),
                resultVectors.getBuffer());
        }
    }

    IRInst* legalizeUnaryCastOperation(IRInst* inst, IROp unaryOp)
    {
        // Get the result matrix type to determine dimensions
        auto resultMatrixType = as<IRMatrixType>(inst->getDataType());
        SLANG_ASSERT(resultMatrixType && "Unary operation should have matrix result type");

        return legalizeUnaryOperation(inst, unaryOp, resultMatrixType);
    }

    IRInst* legalizeUnaryLogicalOperation(IRInst* inst, IROp unaryOp)
    {
        // Get the result matrix type to determine dimensions
        auto resultMatrixType = as<IRMatrixType>(inst->getDataType());
        SLANG_ASSERT(resultMatrixType && "Unary operation should have matrix result type");
        SLANG_ASSERT(
            shouldLowerMatrixType(resultMatrixType) &&
            "Result matrix type should need legalization");

        return legalizeUnaryOperation(inst, unaryOp, resultMatrixType);
    }

    IRInst* legalizeMatrixProducingInstruction(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_MakeMatrix:
            return legalizeMakeMatrix(inst);
        case kIROp_MakeMatrixFromScalar:
            return legalizeMakeMatrixFromScalar(inst);
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_IRem:
        case kIROp_FRem:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
            return legalizeBinaryOperation(inst, inst->getOp());
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Less:
        case kIROp_Geq:
        case kIROp_Leq:
            return legalizeComparisonOperation(inst, inst->getOp());
        case kIROp_Not:
        case kIROp_BitNot:
        case kIROp_Neg:
            return legalizeUnaryLogicalOperation(inst, inst->getOp());
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
            return legalizeUnaryCastOperation(inst, inst->getOp());
        default:
            break;
        }

        return inst;
    }

    IRInst* getReplacement(IRInst* inst)
    {
        if (auto replacement = replacements.tryGetValue(inst))
            return *replacement;

        IRInst* newInst = inst;
        if (as<IRMatrixType>(inst))
            newInst = legalizeMatrixTypeDeclaration(inst);

        IRType* resultType = inst->getDataType();
        if (auto matrixType = as<IRMatrixType>(resultType))
        {
            // On targets that require matrix lowering, some matrix result types (e.g. float4x4) do
            // not need to be lowered, but can be cast from a matrix type that does need to be
            // lowered (e.g. uint4x4), so we need to check if we should lower the operand even if we
            // should not lower the result
            bool shouldLowerOperandMatrixType = false;
            if (inst->getOperandCount() > 0)
            {
                auto operand = inst->getOperand(0);
                IRType* operandType = nullptr;
                if (operand)
                {
                    operandType = operand->getDataType();
                }
                if (auto operandMatrixType = as<IRMatrixType>(operandType))
                {
                    shouldLowerOperandMatrixType = shouldLowerMatrixType(operandMatrixType);
                }
            }

            if (shouldLowerMatrixType(matrixType) || shouldLowerOperandMatrixType)
                newInst = legalizeMatrixProducingInstruction(inst);
        }

        replacements[inst] = newInst;
        return newInst;
    }

    void processModule()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            // Run this inst through the replacer
            getReplacement(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Apply all replacements
        for (const auto& [old, replacement] : replacements)
        {
            if (old != replacement)
            {
                old->replaceUsesWith(replacement);
                old->removeAndDeallocate();
            }
        }
    }
};

void legalizeMatrixTypes(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink)
{
    MatrixTypeLoweringContext context(targetProgram, module);
    context.sink = sink;
    context.processModule();
}

} // namespace Slang
