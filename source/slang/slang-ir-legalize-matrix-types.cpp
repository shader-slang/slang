#include "slang-ir-legalize-matrix-types.h"

#include "slang-compiler.h"
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

    MatrixTypeLoweringContext(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink)
        : targetProgram(targetProgram), module(module), sink(sink), workList(module), workListSet(module)
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

    // Check if a type is a lowered matrix (array of vectors with bool/int/uint elements)
    bool isLoweredMatrixType(IRType* type)
    {
        if (auto arrayType = as<IRArrayType>(type))
        {
            if (auto vectorType = as<IRVectorType>(arrayType->getElementType()))
            {
                auto elementType = vectorType->getElementType();
                return as<IRBoolType>(elementType) || as<IRUIntType>(elementType) ||
                       as<IRIntType>(elementType);
            }
        }
        return false;
    }

    // Check if an instruction should be legalized (operates on lowered matrix types)
    bool shouldLegalizeInst(IRInst* inst)
    {
        if (!shouldLowerTarget())
            return false;

        // Helper function to check if a type should be lowered or is already lowered
        auto needsMatrixLegalization = [this](IRType* type) -> bool {
            if (isLoweredMatrixType(type))
                return true;
            if (auto matrixType = as<IRMatrixType>(type))
                return shouldLowerMatrixType(matrixType);
            return false;
        };

        switch (inst->getOp())
        {
        case kIROp_MakeMatrix:
            return needsMatrixLegalization(inst->getDataType());
        
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Less:
        case kIROp_Greater:
        case kIROp_Leq:
        case kIROp_Geq:
        case kIROp_Eql:
        case kIROp_Neq:
            return needsMatrixLegalization(inst->getDataType()) || 
                   needsMatrixLegalization(inst->getOperand(0)->getDataType()) ||
                   (inst->getOperandCount() > 1 && needsMatrixLegalization(inst->getOperand(1)->getDataType()));

        case kIROp_Not:
        case kIROp_BitNot:
        case kIROp_Neg:
            return needsMatrixLegalization(inst->getDataType()) || 
                   needsMatrixLegalization(inst->getOperand(0)->getDataType());

        case kIROp_GetElement:
        case kIROp_GetElementPtr:
            {
                auto operandType = inst->getOperand(0)->getDataType();
                // For GetElementPtr, we need to check the value type of the pointer
                if (auto ptrType = as<IRPtrTypeBase>(operandType))
                    operandType = ptrType->getValueType();
                return needsMatrixLegalization(operandType);
            }

        case kIROp_MatrixReshape:
            return needsMatrixLegalization(inst->getOperand(0)->getDataType()) ||
                   needsMatrixLegalization(inst->getDataType());

        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
            return needsMatrixLegalization(inst->getDataType()) || 
                   needsMatrixLegalization(inst->getOperand(0)->getDataType());

        default:
            return false;
        }
    }

    IRInst* legalizeMatrixConstruction(IRInst* inst)
    {
        auto dataType = inst->getDataType();
        
        // Ensure result type is in lowered form
        if (auto matrixType = as<IRMatrixType>(dataType))
        {
            if (shouldLowerMatrixType(matrixType))
                dataType = as<IRType>(getReplacement(dataType));
        }
        
        auto arrayType = as<IRArrayType>(dataType);
        SLANG_ASSERT(arrayType);
        
        auto vectorType = as<IRVectorType>(arrayType->getElementType());
        SLANG_ASSERT(vectorType);
        
        auto elementType = vectorType->getElementType();
        auto rowCount = getIntVal(arrayType->getElementCount());
        auto colCount = getIntVal(vectorType->getElementCount());

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);

        // If operands are already row vectors, use them directly
        if (inst->getOperandCount() > 0 && as<IRVectorType>(inst->getOperand(0)->getDataType()))
        {
            List<IRInst*> vectors;
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                vectors.add(inst->getOperand(i));
            }
            return builder.emitMakeArray(dataType, vectors.getCount(), vectors.getBuffer());
        }

        // Otherwise, operands are raw elements, construct row vectors first
        List<IRInst*> rowVectors;
        UInt index = 0;
        
        for (IRIntegerValue row = 0; row < rowCount; row++)
        {
            List<IRInst*> colElements;
            for (IRIntegerValue col = 0; col < colCount; col++)
            {
                if (index < inst->getOperandCount())
                {
                    colElements.add(inst->getOperand(index));
                    index++;
                }
                else
                {
                    // Fill missing elements with default values
                    auto defaultVal = builder.emitDefaultConstruct(elementType);
                    colElements.add(defaultVal);
                }
            }
            auto rowVector = builder.emitMakeVector(vectorType, colElements.getCount(), colElements.getBuffer());
            rowVectors.add(rowVector);
        }
        
        return builder.emitMakeArray(dataType, rowVectors.getCount(), rowVectors.getBuffer());
    }

    IRInst* legalizeBinaryOperation(IRInst* inst)
    {
        auto resultType = inst->getDataType();
        
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        auto left = inst->getOperand(0);
        auto right = inst->getOperand(1);
        
        // Convert operands to lowered form if needed
        if (auto matrixType = as<IRMatrixType>(left->getDataType()))
        {
            if (shouldLowerMatrixType(matrixType))
                left = getReplacement(left);
        }
        if (auto matrixType = as<IRMatrixType>(right->getDataType()))
        {
            if (shouldLowerMatrixType(matrixType))
                right = getReplacement(right);
        }
        
        // Ensure result type is also in lowered form
        if (auto matrixType = as<IRMatrixType>(resultType))
        {
            if (shouldLowerMatrixType(matrixType))
                resultType = as<IRType>(getReplacement(resultType));
        }
        
        auto arrayType = as<IRArrayType>(resultType);
        if (!arrayType)
            return inst; // Can't legalize non-array result type
        
        auto vectorType = as<IRVectorType>(arrayType->getElementType());
        if (!vectorType)
            return inst; // Can't legalize non-vector array element type
        
        auto rowCount = getIntVal(arrayType->getElementCount());
        auto colCount = getIntVal(vectorType->getElementCount());

        // Check if operands are arrays (matrices) or scalars
        bool leftIsArray = isLoweredMatrixType(left->getDataType());
        bool rightIsArray = isLoweredMatrixType(right->getDataType());

        List<IRInst*> resultRows;
        
        for (IRIntegerValue i = 0; i < rowCount; i++)
        {
            IRInst* leftOperand;
            IRInst* rightOperand;
            
            if (leftIsArray)
            {
                auto indexVal = builder.getIntValue(builder.getIntType(), i);
                leftOperand = builder.emitElementExtract(left, indexVal);
            }
            else
            {
                // Broadcast scalar to vector
                List<IRInst*> elements;
                for (IRIntegerValue j = 0; j < colCount; j++)
                {
                    elements.add(left);
                }
                leftOperand = builder.emitMakeVector(vectorType, elements.getCount(), elements.getBuffer());
            }
            
            if (rightIsArray)
            {
                auto indexVal = builder.getIntValue(builder.getIntType(), i);
                rightOperand = builder.emitElementExtract(right, indexVal);
            }
            else
            {
                // Broadcast scalar to vector
                List<IRInst*> elements;
                for (IRIntegerValue j = 0; j < colCount; j++)
                {
                    elements.add(right);
                }
                rightOperand = builder.emitMakeVector(vectorType, elements.getCount(), elements.getBuffer());
            }
            
            IRInst* resultRow;
            switch (inst->getOp())
            {
            case kIROp_Add:
                resultRow = builder.emitAdd(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_Sub:
                resultRow = builder.emitSub(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_Mul:
                resultRow = builder.emitMul(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_Lsh:
                resultRow = builder.emitShl(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_Rsh:
                resultRow = builder.emitShr(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_And:
                resultRow = builder.emitAnd(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_Or:
                resultRow = builder.emitOr(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_BitAnd:
                resultRow = builder.emitBitAnd(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_BitOr:
                resultRow = builder.emitBitOr(vectorType, leftOperand, rightOperand);
                break;
            case kIROp_BitXor:
                {
                    // Create BitXor instruction manually since emitBitXor doesn't exist
                    IRInst* operands[2] = { leftOperand, rightOperand };
                    auto bitXorInst = builder.emitIntrinsicInst(vectorType, kIROp_BitXor, 2, operands);
                    resultRow = bitXorInst;
                }
                break;
            case kIROp_Less:
                resultRow = builder.emitLess(leftOperand, rightOperand);
                break;
            case kIROp_Greater:
                {
                    // For vector comparisons, result should match vectorType if it's bool, otherwise bool vector
                    IRType* resultVectorType = vectorType;
                    if (!as<IRBoolType>(vectorType->getElementType()))
                    {
                        resultVectorType = builder.getVectorType(builder.getBoolType(), colCount);
                    }
                    IRInst* operands[2] = { leftOperand, rightOperand };
                    auto greaterInst = builder.emitIntrinsicInst(resultVectorType, kIROp_Greater, 2, operands);
                    resultRow = greaterInst;
                }
                break;
            case kIROp_Leq:
                {
                    // For vector comparisons, result should match vectorType if it's bool, otherwise bool vector
                    IRType* resultVectorType = vectorType;
                    if (!as<IRBoolType>(vectorType->getElementType()))
                    {
                        resultVectorType = builder.getVectorType(builder.getBoolType(), colCount);
                    }
                    IRInst* operands[2] = { leftOperand, rightOperand };
                    auto leqInst = builder.emitIntrinsicInst(resultVectorType, kIROp_Leq, 2, operands);
                    resultRow = leqInst;
                }
                break;
            case kIROp_Geq:
                resultRow = builder.emitGeq(leftOperand, rightOperand);
                break;
            case kIROp_Eql:
                resultRow = builder.emitEql(leftOperand, rightOperand);
                break;
            case kIROp_Neq:
                resultRow = builder.emitNeq(leftOperand, rightOperand);
                break;
            default:
                SLANG_UNEXPECTED("unhandled binary operation");
                return nullptr;
            }
            
            resultRows.add(resultRow);
        }
        
        return builder.emitMakeArray(resultType, resultRows.getCount(), resultRows.getBuffer());
    }

    IRInst* legalizeUnaryOperation(IRInst* inst)
    {
        auto resultType = inst->getDataType();
        auto operand = inst->getOperand(0);
        
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        // Convert operand to lowered form if needed
        if (auto matrixType = as<IRMatrixType>(operand->getDataType()))
        {
            if (shouldLowerMatrixType(matrixType))
                operand = getReplacement(operand);
        }
        
        // Ensure result type is also in lowered form
        if (auto matrixType = as<IRMatrixType>(resultType))
        {
            if (shouldLowerMatrixType(matrixType))
                resultType = as<IRType>(getReplacement(resultType));
        }
        
        auto arrayType = as<IRArrayType>(resultType);
        if (!arrayType)
            return inst; // Can't legalize non-array result type
        
        auto vectorType = as<IRVectorType>(arrayType->getElementType());
        if (!vectorType)
            return inst; // Can't legalize non-vector array element type
        
        auto rowCount = getIntVal(arrayType->getElementCount());

        List<IRInst*> resultRows;
        
        for (IRIntegerValue i = 0; i < rowCount; i++)
        {
            auto indexVal = builder.getIntValue(builder.getIntType(), i);
            auto vectorOperand = builder.emitElementExtract(operand, indexVal);
            
            IRInst* resultRow;
            switch (inst->getOp())
            {
            case kIROp_Not:
                resultRow = builder.emitNot(vectorType, vectorOperand);
                break;
            case kIROp_BitNot:
                resultRow = builder.emitBitNot(vectorType, vectorOperand);
                break;
            default:
                SLANG_UNEXPECTED("unhandled unary operation");
                return nullptr;
            }
            
            resultRows.add(resultRow);
        }
        
        return builder.emitMakeArray(resultType, resultRows.getCount(), resultRows.getBuffer());
    }

    IRInst* legalizeGetElement(IRInst* inst)
    {
        auto getElementInst = as<IRGetElement>(inst);
        SLANG_ASSERT(getElementInst);
        
        auto base = getElementInst->getBase();
        auto index = getElementInst->getIndex();
        
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        // Convert base to lowered form if needed
        if (auto matrixType = as<IRMatrixType>(base->getDataType()))
        {
            if (shouldLowerMatrixType(matrixType))
                base = getReplacement(base);
        }
        
        // The base should now be a lowered matrix (array of vectors)
        // GetElement extracts one row vector
        if (isLoweredMatrixType(base->getDataType()))
        {
            return builder.emitElementExtract(base, index);
        }
        
        // If it's not a lowered matrix type, fall back to original instruction
        return inst;
    }

    IRInst* legalizeGetElementPtr(IRInst* inst)
    {
        auto getElementPtrInst = as<IRGetElementPtr>(inst);
        SLANG_ASSERT(getElementPtrInst);
        
        auto base = getElementPtrInst->getBase();
        auto index = getElementPtrInst->getIndex();
        
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        // For GetElementPtr, check the pointed-to type
        auto ptrType = as<IRPtrTypeBase>(base->getDataType());
        if (ptrType)
        {
            auto valueType = ptrType->getValueType();
            if (auto matrixType = as<IRMatrixType>(valueType))
            {
                if (shouldLowerMatrixType(matrixType))
                {
                    // The base should be a pointer to a lowered matrix (array of vectors)
                    // GetElementPtr gets a pointer to one row vector
                    return builder.emitElementAddress(base, index);
                }
            }
            else if (isLoweredMatrixType(valueType))
            {
                return builder.emitElementAddress(base, index);
            }
        }
        
        // If it's not a matrix pointer type, fall back to original instruction
        return inst;
    }

    IRInst* legalizeMatrixCast(IRInst* inst)
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        auto operand = inst->getOperand(0);
        auto resultType = inst->getDataType();
        
        // Check if we're casting from/to lowered matrix types
        bool operandIsLowered = isLoweredMatrixType(operand->getDataType());
        bool resultIsLowered = isLoweredMatrixType(resultType);
        
        if (operandIsLowered && resultIsLowered)
        {
            // Both are lowered matrices - cast element-wise
            auto operandArrayType = as<IRArrayType>(operand->getDataType());
            auto resultArrayType = as<IRArrayType>(resultType);
            SLANG_ASSERT(operandArrayType && resultArrayType);
            
            auto operandVectorType = as<IRVectorType>(operandArrayType->getElementType());
            auto resultVectorType = as<IRVectorType>(resultArrayType->getElementType());
            SLANG_ASSERT(operandVectorType && resultVectorType);
            
            auto rowCount = getIntVal(operandArrayType->getElementCount());
            
            List<IRInst*> resultRows;
            for (IRIntegerValue i = 0; i < rowCount; i++)
            {
                auto operandRow = builder.emitElementExtract(operand, builder.getIntValue(builder.getIntType(), i));
                IRInst* resultRow = nullptr;
                
                // Apply the same cast operation to the vector
                switch (inst->getOp())
                {
                case kIROp_IntCast:
                    resultRow = builder.emitIntrinsicInst(resultVectorType, kIROp_IntCast, 1, &operandRow);
                    break;
                case kIROp_FloatCast:
                    resultRow = builder.emitIntrinsicInst(resultVectorType, kIROp_FloatCast, 1, &operandRow);
                    break;
                case kIROp_CastIntToFloat:
                    resultRow = builder.emitIntrinsicInst(resultVectorType, kIROp_CastIntToFloat, 1, &operandRow);
                    break;
                case kIROp_CastFloatToInt:
                    resultRow = builder.emitIntrinsicInst(resultVectorType, kIROp_CastFloatToInt, 1, &operandRow);
                    break;
                default:
                    SLANG_UNEXPECTED("unhandled cast operation");
                    return nullptr;
                }
                
                resultRows.add(resultRow);
            }
            
            return builder.emitMakeArray(resultType, resultRows.getCount(), resultRows.getBuffer());
        }
        
        // If only one side is lowered, we shouldn't reach here as the types should match
        SLANG_UNEXPECTED("mismatched matrix types in cast");
    }

    IRInst* legalizeMatrixNegation(IRInst* inst)
    {
        auto operand = inst->getOperand(0);
        auto resultType = inst->getDataType();
        
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        // Convert operand to lowered form if needed
        if (auto matrixType = as<IRMatrixType>(operand->getDataType()))
        {
            if (shouldLowerMatrixType(matrixType))
                operand = getReplacement(operand);
        }
        
        // Ensure result type is also in lowered form
        if (auto matrixType = as<IRMatrixType>(resultType))
        {
            if (shouldLowerMatrixType(matrixType))
                resultType = as<IRType>(getReplacement(resultType));
        }
        
        // Check if operand is now a lowered matrix type
        if (!isLoweredMatrixType(operand->getDataType()))
            return inst; // Not a lowered matrix, can't handle
        
        auto arrayType = as<IRArrayType>(operand->getDataType());
        SLANG_ASSERT(arrayType);
        auto vectorType = as<IRVectorType>(arrayType->getElementType());
        SLANG_ASSERT(vectorType);
        
        auto rowCount = getIntVal(arrayType->getElementCount());
        
        List<IRInst*> resultRows;
        for (IRIntegerValue i = 0; i < rowCount; i++)
        {
            auto operandRow = builder.emitElementExtract(operand, builder.getIntValue(builder.getIntType(), i));
            auto resultRow = builder.emitNeg(vectorType, operandRow);
            resultRows.add(resultRow);
        }
        
        return builder.emitMakeArray(resultType, resultRows.getCount(), resultRows.getBuffer());
    }

    IRInst* legalizeMatrixReshape(IRInst* inst)
    {
        auto operand = inst->getOperand(0);
        auto resultType = inst->getDataType();
        
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        
        // Get source dimensions
        auto srcArrayType = as<IRArrayType>(operand->getDataType());
        SLANG_ASSERT(srcArrayType);
        auto srcVectorType = as<IRVectorType>(srcArrayType->getElementType());
        SLANG_ASSERT(srcVectorType);
        
        auto srcRowCount = getIntVal(srcArrayType->getElementCount());
        auto srcColCount = getIntVal(srcVectorType->getElementCount());
        
        // Get destination dimensions
        auto dstArrayType = as<IRArrayType>(resultType);
        SLANG_ASSERT(dstArrayType);
        auto dstVectorType = as<IRVectorType>(dstArrayType->getElementType());
        SLANG_ASSERT(dstVectorType);
        
        auto dstRowCount = getIntVal(dstArrayType->getElementCount());
        auto dstColCount = getIntVal(dstVectorType->getElementCount());
        
        auto elementType = srcVectorType->getElementType();
        
        // Extract all source elements in row-major order
        List<IRInst*> allElements;
        for (IRIntegerValue r = 0; r < srcRowCount; r++)
        {
            auto srcRow = builder.emitElementExtract(operand, builder.getIntValue(builder.getIntType(), r));
            for (IRIntegerValue c = 0; c < srcColCount; c++)
            {
                auto element = builder.emitElementExtract(srcRow, builder.getIntValue(builder.getIntType(), c));
                allElements.add(element);
            }
        }
        
        // Create zero element for padding if needed
        IRInst* zeroElement = nullptr;
        if (dstRowCount * dstColCount > srcRowCount * srcColCount)
        {
            zeroElement = builder.emitDefaultConstruct(elementType);
        }
        
        // Reorganize into destination shape
        List<IRInst*> dstRows;
        for (IRIntegerValue r = 0; r < dstRowCount; r++)
        {
            List<IRInst*> dstRowElements;
            for (IRIntegerValue c = 0; c < dstColCount; c++)
            {
                auto elementIndex = r * dstColCount + c;
                if (elementIndex < allElements.getCount())
                {
                    dstRowElements.add(allElements[elementIndex]);
                }
                else
                {
                    dstRowElements.add(zeroElement);
                }
            }
            auto dstRow = builder.emitMakeVector(dstVectorType, dstRowElements.getCount(), dstRowElements.getBuffer());
            dstRows.add(dstRow);
        }
        
        return builder.emitMakeArray(resultType, dstRows.getCount(), dstRows.getBuffer());
    }

    IRInst* legalizeInst(IRInst* inst)
    {
        if (!shouldLegalizeInst(inst))
            return inst;

        switch (inst->getOp())
        {
        case kIROp_MakeMatrix:
            return legalizeMatrixConstruction(inst);
            
        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Less:
        case kIROp_Greater:
        case kIROp_Leq:
        case kIROp_Geq:
        case kIROp_Eql:
        case kIROp_Neq:
            return legalizeBinaryOperation(inst);

        case kIROp_Not:
        case kIROp_BitNot:
            return legalizeUnaryOperation(inst);

        case kIROp_GetElement:
            return legalizeGetElement(inst);
        case kIROp_GetElementPtr:
            return legalizeGetElementPtr(inst);
        case kIROp_IntCast:
        case kIROp_FloatCast:
        case kIROp_CastIntToFloat:
        case kIROp_CastFloatToInt:
            return legalizeMatrixCast(inst);
        case kIROp_Neg:
            return legalizeMatrixNegation(inst);
        case kIROp_MatrixReshape:
            return legalizeMatrixReshape(inst);

        default:
            return inst;
        }
    }

    IRInst* getReplacement(IRInst* inst)
    {
        if (auto replacement = replacements.tryGetValue(inst))
            return *replacement;

        IRInst* newInst = inst;

        if (auto matrixType = as<IRMatrixType>(inst))
        {
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

                newInst = arrayType;
            }
        }
        else
        {
            // Try to legalize operations
            newInst = legalizeInst(inst);
        }

        replacements[inst] = newInst;
        return newInst;
    }

    void processModule()
    {
        // First pass: collect all instructions that need legalization
        List<IRInst*> instsToProcess;
        
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            // Check if this instruction needs legalization
            if (shouldLegalizeInst(inst) || (as<IRMatrixType>(inst) && shouldLowerMatrixType(as<IRMatrixType>(inst))))
            {
                instsToProcess.add(inst);
            }

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Second pass: process types first
        for (auto inst : instsToProcess)
        {
            if (as<IRMatrixType>(inst))
            {
                getReplacement(inst);
            }
        }

        // Third pass: process operations
        for (auto inst : instsToProcess)
        {
            if (!as<IRMatrixType>(inst))
            {
                getReplacement(inst);
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
    MatrixTypeLoweringContext context(targetProgram, module, sink);
    context.processModule();
}

} // namespace Slang