// slang-emit-cpp.cpp
#include "slang-emit-cpp.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include "slang-ir-clone.h"

#include <assert.h>

namespace Slang {

static const char s_elemNames[] = "xyzw";

static UnownedStringSlice _getTypePrefix(IROp op)
{
    switch (op)
    {
        case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("Bool");
        case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I32");
        case kIROp_UIntType:        return UnownedStringSlice::fromLiteral("U32");
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F32");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
        default:                    return UnownedStringSlice::fromLiteral("?");
    }
}

static IROp _getTypeStyle(IROp op)
{
    switch (op)
    {
        case kIROp_VoidType:    
        case kIROp_BoolType:
        {
            return op;
        }
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
        {
            // All int like 
            return kIROp_IntType;
        }
        case kIROp_HalfType:
        case kIROp_FloatType:
        case kIROp_DoubleType:
        {
            // All float like
            return kIROp_FloatType;
        }
        default: return kIROp_Invalid;
    }
}

static IROp _getCType(IROp op)
{
    switch (op)
    {
        case kIROp_VoidType:
        case kIROp_BoolType:
        {
            return op;
        }
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        {
            // Promote all these to Int
            return kIROp_IntType;
        }
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
        {
            // Promote all these to Int64, we can just vary the call to make these work
            return kIROp_Int64Type;
        }
        case kIROp_DoubleType:
        {
            return kIROp_DoubleType;
        }
        case kIROp_HalfType:
        case kIROp_FloatType:
        {
            // Promote both to float
            return kIROp_FloatType;
        }
        default:
        {
            SLANG_ASSERT(!"Unhandled type");
            return kIROp_undefined;
        }
    }
}

static UnownedStringSlice _getCTypeVecPostFix(IROp op)
{
    switch (op)
    {
        case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("B");
        case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I");
        case kIROp_UIntType:        return UnownedStringSlice::fromLiteral("U");
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
        default:                    return UnownedStringSlice::fromLiteral("?");
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!! CPPEmitHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

static const CPPSourceEmitter::OperationInfo s_operationInfos[] =
{
#define SLANG_CPP_INTRINSIC_OP_INFO(x, funcName, numOperands) { UnownedStringSlice::fromLiteral(#x), UnownedStringSlice::fromLiteral(funcName), int8_t(numOperands)  },
    SLANG_CPP_INTRINSIC_OP(SLANG_CPP_INTRINSIC_OP_INFO)
};

/* static */ UnownedStringSlice CPPSourceEmitter::getBuiltinTypeName(IROp op)
{
    switch (op)
    {
        case kIROp_VoidType:    return UnownedStringSlice("void");
        case kIROp_BoolType:    return UnownedStringSlice("bool");

        case kIROp_Int8Type:    return UnownedStringSlice("int8_t");
        case kIROp_Int16Type:   return UnownedStringSlice("int16_t");
        case kIROp_IntType:     return UnownedStringSlice("int32_t");
        case kIROp_Int64Type:   return UnownedStringSlice("int64_t");

        case kIROp_UInt8Type:   return UnownedStringSlice("uint8_t");
        case kIROp_UInt16Type:  return UnownedStringSlice("uint16_t");
        case kIROp_UIntType:    return UnownedStringSlice("uint32_t");
        case kIROp_UInt64Type:  return UnownedStringSlice("uint64_t");

        // Not clear just yet how we should handle half... we want all processing as float probly, but when reading/writing to memory converting

        case kIROp_HalfType:    return UnownedStringSlice("half");

        case kIROp_FloatType:   return UnownedStringSlice("float");
        case kIROp_DoubleType:  return UnownedStringSlice("double");
        default:                return UnownedStringSlice();
    }
}


/* static */const CPPSourceEmitter::OperationInfo& CPPSourceEmitter::getOperationInfo(IntrinsicOp op)
{
    return s_operationInfos[int(op)];
}

/* static */CPPSourceEmitter::IntrinsicOp CPPSourceEmitter::getOperation(IROp op)
{
    switch (op)
    {
        case kIROp_Add:     return IntrinsicOp::Add;
        case kIROp_Mul:     return IntrinsicOp::Mul;
        case kIROp_Sub:     return IntrinsicOp::Sub;
        case kIROp_Div:     return IntrinsicOp::Div;
        case kIROp_Lsh:     return IntrinsicOp::Lsh;
        case kIROp_Rsh:     return IntrinsicOp::Rsh;
        case kIROp_Mod:     return IntrinsicOp::Mod;

        case kIROp_Eql:     return IntrinsicOp::Eql;
        case kIROp_Neq:     return IntrinsicOp::Neq;
        case kIROp_Greater: return IntrinsicOp::Greater;
        case kIROp_Less:    return IntrinsicOp::Less;
        case kIROp_Geq:     return IntrinsicOp::Geq;
        case kIROp_Leq:     return IntrinsicOp::Leq;

        case kIROp_BitAnd:  return IntrinsicOp::BitAnd;
        case kIROp_BitXor:  return IntrinsicOp::BitXor;
        case kIROp_BitOr:   return IntrinsicOp::BitOr;
                
        case kIROp_And:     return IntrinsicOp::And;
        case kIROp_Or:      return IntrinsicOp::Or;

        case kIROp_Neg:     return IntrinsicOp::Neg;
        case kIROp_Not:     return IntrinsicOp::Not;
        case kIROp_BitNot:  return IntrinsicOp::BitNot;

        default:            return IntrinsicOp::Invalid;
    }
}

CPPSourceEmitter::IntrinsicOp CPPSourceEmitter::getOperationByName(const UnownedStringSlice& slice)
{
    Index index = m_slicePool.findIndex(slice);
    if (index >= 0 && index < m_intrinsicOpMap.getCount())
    {
        IntrinsicOp op = m_intrinsicOpMap[index];
        if (op != IntrinsicOp::Invalid)
        {
            return op;
        }
    }

    return IntrinsicOp::Invalid;
}

void CPPSourceEmitter::emitTypeDefinition(IRType* inType)
{
    IRType* type = _cloneType(inType);
    if (m_typeEmittedMap.TryGetValue(type))
    {
        return;
    }

    if (type->getModule() != m_uniqueModule)
    {
        // If defined in a different module, we assume they are emitted already. (Assumed to
        // be a nominal type)
        return;
    }

    SourceWriter* writer = getSourceWriter();

    switch (type->op)
    {
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            int count = int(GetIntVal(vecType->getElementCount()));

            SLANG_ASSERT(count > 0 && count < 4);

            UnownedStringSlice typeName = _getTypeName(type);
            UnownedStringSlice elemName = _getTypeName(vecType->getElementType());

            writer->emit("struct ");
            writer->emit(typeName);
            writer->emit("\n{\n");
            writer->indent();

            writer->emit(elemName);
            writer->emit(" ");
            for (int i = 0; i < count; ++i)
            {
                if (i > 0)
                {
                    writer->emit(", ");
                }
                writer->emitChar(s_elemNames[i]);
            }
            writer->emit(";\n");

            writer->dedent();
            writer->emit("};\n\n");

            m_typeEmittedMap.Add(type, true);
            break;
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            const auto rowCount = int(GetIntVal(matType->getRowCount()));
            const auto colCount = int(GetIntVal(matType->getColumnCount()));

            IRType* vecType = _getVecType(matType->getElementType(), colCount);
            emitTypeDefinition(vecType);

            UnownedStringSlice typeName = _getTypeName(type);
            UnownedStringSlice rowTypeName = _getTypeName(vecType);

            writer->emit("struct ");
            writer->emit(typeName);
            writer->emit("\n{\n");
            writer->indent();

            writer->emit(rowTypeName);
            writer->emit(" rows[");
            writer->emit(rowCount);
            writer->emit("];\n");

            writer->dedent();
            writer->emit("};\n\n");

            m_typeEmittedMap.Add(type, true);
            break;
        }
        case kIROp_PtrType:
        case kIROp_RefType:
        {
            // We don't need to output a definition for these types
            break;
        }
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_HLSLRWStructuredBufferType:
        {
            // We don't need to output a definition for these with C++ templates
            // For C we may need to (or do casting at point of usage)
            break;
        }
        default:
        {
            if (IRBasicType::isaImpl(type->op))
            {
                // Don't emit anything for built in types
                return;
            }
            SLANG_ASSERT(!"Unhandled type");
            break;
        }
    }
}

UnownedStringSlice CPPSourceEmitter::_getTypeName(IRType* inType)
{
    if (isNominalOp(inType->op))
    {
        StringSlicePool::Handle handle;
        // NOTE! This is somewhat unusual -> we are going to add types which aren't cloned and belong to
        // m_uniqueModule. We *assume* nominal types are de-duped
        if (!m_typeNameMap.TryGetValue(inType, handle))
        {
            auto name = getName(inType);
            handle = m_slicePool.add(name);
            m_typeNameMap.Add(inType, handle);
        }
        return m_slicePool.getSlice(handle);
    }
    
    IRType* type = _cloneType(inType);

    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_typeNameMap.TryGetValue(type, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    handle = _calcTypeName(type);

    m_typeNameMap.Add(type, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

StringSlicePool::Handle CPPSourceEmitter::_calcTypeName(IRType* type)
{
    switch (type->op)
    {
        case kIROp_HalfType:
        {
            // Special case half
            return m_slicePool.add(getBuiltinTypeName(kIROp_FloatType));
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            auto vecCount = int(GetIntVal(vecType->getElementCount()));
            const IROp elemType = vecType->getElementType()->op;

            StringBuilder builder;
            builder << "Vec";
            UnownedStringSlice postFix = _getCTypeVecPostFix(elemType);

            builder << postFix;
            if (postFix.size() > 1)
            {
                builder << "_";
            }
            builder << vecCount;
            return m_slicePool.add(builder);
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            auto elementType = matType->getElementType();
            const auto rowCount = int(GetIntVal(matType->getRowCount()));
            const auto colCount = int(GetIntVal(matType->getColumnCount()));

            // Make sure there is the vector name too
            _getTypeName(_getVecType(elementType, colCount));

            StringBuilder builder;

            builder << "Mat";
            const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(elementType->op));
            builder << postFix;
            if (postFix.size() > 1)
            {
                builder << "_";
            }
            builder << rowCount;
            builder << colCount;

            return m_slicePool.add(builder);
        }
        case kIROp_HLSLRWStructuredBufferType:
        {
            auto bufType = static_cast<IRHLSLRWStructuredBufferType*>(type);

            StringBuilder builder;
            builder << "RWStructuredBuffer<";
            builder << _getTypeName(bufType->getElementType());
            builder << ">";

            return m_slicePool.add(builder);
        }
        case kIROp_ArrayType:
        {
            auto arrayType = static_cast<IRArrayType*>(type);
            auto elementType = arrayType->getElementType();
            int elementCount = int(GetIntVal(arrayType->getElementCount()));

            StringBuilder builder;
            builder << "FixedArray<";
            builder << _getTypeName(elementType);
            builder << ", " << elementCount << ">";

            return m_slicePool.add(builder);
        }
        default:
        {
            if (IRBasicType::isaImpl(type->op))
            {
                return m_slicePool.add(getBuiltinTypeName(type->op));
            }
            break;
        }
    }

    return StringSlicePool::kNullHandle;
}

void CPPSourceEmitter::useType(IRType* type)
{
    _getTypeName(type);
}

IRInst* CPPSourceEmitter::_clone(IRInst* inst)
{
    if (inst == nullptr)
    {
        return nullptr;
    }

    // If it's in this module then we don't need to clone
    if (inst->getModule() == m_uniqueModule)
    {
        return inst;
    }

    if (isNominalOp(inst->op))
    {
        // If it's nominal we don't bother copying, as we assumed it is already de-duped
        return inst;
    }
    
    if (IRInst*const* newInstPtr = m_cloneMap.TryGetValue(inst))
    {
        return *newInstPtr;
    }
    
    // It would be nice if I could use ir-clone.cpp to do this -> but it doesn't clone
    // operands. We wouldn't want to clone decorations, and it can't clone IRConstant(!) so
    // it's no use

    IRInst* clone = nullptr;

    switch (inst->op)
    {
        case kIROp_IntLit:
        {
            auto intLit = static_cast<IRConstant*>(inst);
            IRType* cloneType = _cloneType(intLit->getDataType());
            clone = m_irBuilder.getIntValue(cloneType, intLit->value.intVal);
            break;
        }
        default:
        {
            if (IRBasicType::isaImpl(inst->op))
            {
                clone = m_irBuilder.getType(inst->op);
            }
            else
            {
                IRType* irType = dynamicCast<IRType>(inst);
                if (irType)
                {
                    auto cloneType = _cloneType(inst->getFullType());
                    Index operandCount = Index(inst->getOperandCount());

                    List<IRInst*> cloneOperands;
                    cloneOperands.setCount(operandCount);

                    for (Index i = 0; i < operandCount; ++i)
                    {
                        cloneOperands[i] = _clone(inst->getOperand(i));
                    }

                    clone = m_irBuilder.findOrEmitHoistableInst(cloneType, inst->op, operandCount, cloneOperands.getBuffer());
                }
                else
                {
                    // This cloning style only works on insts that are not unique
                    auto cloneType = _cloneType(inst->getFullType());
            
                    Index operandCount = Index(inst->getOperandCount());
                    clone = m_irBuilder.emitIntrinsicInst(cloneType, inst->op, operandCount, nullptr);
                    for (Index i = 0; i < operandCount; ++i)
                    {
                        auto cloneOperand = _clone(inst->getOperand(i));
                        clone->getOperands()[i].init(clone, cloneOperand);
                    }
                }
            }
            break;
        }
    }

    m_cloneMap.Add(inst, clone);
    return clone;
}

static IRBasicType* _getElementType(IRType* type)
{
    switch (type->op)
    {
        case kIROp_VectorType:      type = static_cast<IRVectorType*>(type)->getElementType(); break;
        case kIROp_MatrixType:      type = static_cast<IRMatrixType*>(type)->getElementType(); break;
        default:                    break;
    }
    return dynamicCast<IRBasicType>(type);
}

/* static */CPPSourceEmitter::TypeDimension CPPSourceEmitter::_getTypeDimension(IRType* type, bool vecSwap)
{
    switch (type->op)
    {
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            const int elemCount = int(GetIntVal(vecType->getElementCount()));
            return (!vecSwap) ? TypeDimension{1, elemCount} : TypeDimension{ elemCount, 1};
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);
            const int colCount = int(GetIntVal(matType->getColumnCount()));
            const int rowCount = int(GetIntVal(matType->getRowCount()));
            return TypeDimension{rowCount, colCount};
        }
        default: return TypeDimension{1, 1};
    }
}

/* static */void CPPSourceEmitter::_emitAccess(const UnownedStringSlice& name, const TypeDimension& dimension, int row, int col, SourceWriter* writer)
{
    writer->emit(name);
    const int comb = (dimension.colCount > 1 ? 2 : 0) | (dimension.rowCount > 1 ? 1 : 0);
    switch (comb)
    {
        case 0:
        {
            break;
        }
        case 1:
        case 2:
        {
            // Vector
            int index = (row > col) ? row : col;
            writer->emit(".");
            writer->emitChar(s_elemNames[index]);
            break;
        }
        case 3:
        {
            // Matrix
            writer->emit(".rows[");
            writer->emit(row);
            writer->emit("].");
            writer->emitChar(s_elemNames[col]);
            break;
        }
    }
}

static bool _isOperator(const UnownedStringSlice& funcName)
{
    const char c = funcName[0];
    return !((c >= 'a' && c <='z') || (c >= 'A' && c <= 'Z') || c == '_');
}

void CPPSourceEmitter::_emitAryDefinition(const SpecializedIntrinsic& specOp)
{
    auto info = getOperationInfo(specOp.op);
    auto funcName = info.funcName;
    SLANG_ASSERT(funcName.size() > 0);

    const bool isOperator = _isOperator(funcName);

    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp.signatureType;
    const int numParams = int(funcType->getParamCount());
    SLANG_ASSERT(numParams <= 3);

    bool areAllScalar = true;
    TypeDimension paramDims[3];
    for (int i = 0; i < numParams; ++i)
    {
        paramDims[i]= _getTypeDimension(funcType->getParamType(i), false);
        areAllScalar = areAllScalar && paramDims[i].isScalar();
    }

    // If all are scalar, then we don't need to emit a definition
    if (areAllScalar)
    {
        return;
    }

    IRType* retType = specOp.returnType;
    TypeDimension retDim = _getTypeDimension(retType, false);

    UnownedStringSlice scalarFuncName(funcName);
    if (isOperator)
    {
        StringBuilder builder;
        builder << "operator";
        builder << funcName;
        _emitSignature(builder.getUnownedSlice(), specOp);
    }
    else
    {
        scalarFuncName = _getScalarFuncName(specOp.op, _getElementType(funcType->getParamType(0)));
        _emitSignature(funcName, specOp);
    }
    
    writer->emit("\n{\n");
    writer->indent();

    emitType(retType);
    writer->emit(" r;\n");

    for (int i = 0; i < retDim.rowCount; ++i)
    {
        for (int j = 0; j < retDim.colCount; ++j)
        {
            _emitAccess(UnownedStringSlice::fromLiteral("r"), retDim, i, j, writer);
            writer->emit(" = ");
            if (isOperator)
            {
                switch (numParams)
                {
                    case 1:
                    {
                        writer->emit(funcName);
                        _emitAccess(UnownedStringSlice::fromLiteral("a"), paramDims[0], i, j, writer);
                        break;
                    }
                    case 2:
                    {
                        _emitAccess(UnownedStringSlice::fromLiteral("a"), paramDims[0], i, j, writer);
                        writer->emit(" ");
                        writer->emit(funcName);
                        writer->emit(" ");
                        _emitAccess(UnownedStringSlice::fromLiteral("b"), paramDims[1], i, j, writer);
                        break;
                    }
                    default: SLANG_ASSERT(!"Unhandled");
                }
            }
            else
            {
                writer->emit(scalarFuncName);
                writer->emit("(");
                for (int k = 0; k < numParams; k++)
                {
                    if (k > 0)
                    {
                        writer->emit(", ");
                    }
                    char c = char('a' + k);
                    _emitAccess(UnownedStringSlice(&c, 1), paramDims[k], i, j, writer);
                }
                writer->emit(")");
            }
            writer->emit(";\n");
        }
    }

    writer->emit("return r;\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitAnyAllDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 1);
    IRType* paramType0 = funcType->getParamType(0);

    SourceWriter* writer = getSourceWriter();

    IRType* elementType = _getElementType(paramType0);
    SLANG_ASSERT(elementType);
    IRType* retType = specOp.returnType;
    auto retTypeName = _getTypeName(retType);

    IROp style = _getTypeStyle(elementType->op);

    const TypeDimension dim = _getTypeDimension(paramType0, false);

    _emitSignature(funcName, specOp);
    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");

    for (int i = 0; i < dim.rowCount; ++i)
    {
        for (int j = 0; j < dim.colCount; ++j)
        {
            if (i > 0 || j > 0)
            {
                if (specOp.op == IntrinsicOp::All)
                {
                    writer->emit(" && ");
                }
                else
                {
                    writer->emit(" || ");
                }
            }

            switch (style)
            {
                case kIROp_BoolType:
                {
                    _emitAccess(UnownedStringSlice::fromLiteral("a"), dim, i, j, writer);
                    break;
                }
                case kIROp_IntType:
                {
                    writer->emit("(");
                    _emitAccess(UnownedStringSlice::fromLiteral("a"), dim, i, j, writer);
                    writer->emit(" != 0)");
                    break;
                }
                case kIROp_FloatType:
                {
                    writer->emit("(");
                    _emitAccess(UnownedStringSlice::fromLiteral("a"), dim, i, j, writer);
                    writer->emit(" != 0.0)");
                    break;
                }
            }
        }
    }

    writer->emit(";\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitSignature(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    IRFuncType* funcType = specOp.signatureType;
    const int paramsCount = int(funcType->getParamCount());
    IRType* retType = specOp.returnType;

    SourceWriter* writer = getSourceWriter();

    emitType(retType);
    writer->emit(" ");
    writer->emit(funcName);
    writer->emit("(");

    for (int i = 0; i < paramsCount; ++i)
    {
        if (i > 0)
        {
            writer->emit(", ");
        }

        // We can't pass as const& for vector, scalar, array types, as they are pass by value
        // For types passed by reference, we should do something different
        IRType* paramType = funcType->getParamType(i);
#if 0
        writer->emit("const ");
#endif
        emitType(paramType);
#if 0
        if (dynamicCast<IRBasicType>(paramType))
        {
            writer->emit(" ");
        }
        else
        {
            writer->emit("& ");
        }
#else

        writer->emit(" ");
#endif

        writer->emitChar(char('a' + i));
    }
    writer->emit(")");
}

void CPPSourceEmitter::_emitVecMatMulDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);
    IRType* paramType0 = funcType->getParamType(0);
    IRType* paramType1 = funcType->getParamType(1);
    IRType* retType = specOp.returnType;

    SourceWriter* writer = getSourceWriter();

    _emitSignature(funcName, specOp);

    writer->emit("\n{\n");
    writer->indent();

    emitType(retType);
    writer->emit(" r;\n");

    TypeDimension dimA = _getTypeDimension(paramType0, false);
    TypeDimension dimB = _getTypeDimension(paramType1, true);
    TypeDimension resultDim = _getTypeDimension(retType, paramType1->op == kIROp_VectorType);

    for (int i = 0; i < resultDim.rowCount; ++i)
    {
        for (int j = 0; j < resultDim.colCount; ++j)
        {
            _emitAccess(UnownedStringSlice::fromLiteral("r"), resultDim, i, j, writer);
            writer->emit(" = ");

            for (int k = 0; k < dimA.colCount; k++)
            {
                if (k > 0)
                {
                    writer->emit(" + ");
                }
                _emitAccess(UnownedStringSlice::fromLiteral("a"), dimA, i, k, writer);
                writer->emit(" * ");
                _emitAccess(UnownedStringSlice::fromLiteral("b"), dimB, k, j, writer);
            }

            writer->emit(";\n");
        }
    }

    writer->emit("return r;\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitCrossDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    _emitSignature(funcName, specOp);

    SourceWriter* writer = getSourceWriter();

    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");
    emitType(specOp.returnType);
    writer->emit("{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; \n");

    writer->dedent();
    writer->emit("}\n\n");
}

UnownedStringSlice CPPSourceEmitter::_getAndEmitSpecializedOperationDefinition(IntrinsicOp op, IRType*const* argTypes, Int argCount, IRType* retType)
{
    SpecializedIntrinsic specOp;
    specOp.op = op;
    specOp.returnType = retType;
    specOp.signatureType = m_irBuilder.getFuncType(argCount, argTypes, m_irBuilder.getVoidType());

    emitSpecializedOperationDefinition(specOp);
    return  _getFuncName(specOp);
}

void CPPSourceEmitter::_emitLengthDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 1);
    IRType* paramType0 = funcType->getParamType(0);

    SLANG_ASSERT(paramType0->op == kIROp_VectorType);

    IRBasicType* elementType = as<IRBasicType>(static_cast<IRVectorType*>(paramType0)->getElementType());

    IRType* dotArgs[] = { paramType0, paramType0 };
    UnownedStringSlice dotFuncName = _getAndEmitSpecializedOperationDefinition(IntrinsicOp::Dot, dotArgs, SLANG_COUNT_OF(dotArgs), elementType);

    UnownedStringSlice sqrtName = _getScalarFuncName(IntrinsicOp::Sqrt, elementType);

    _emitSignature(funcName, specOp);

    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");
    writer->emit(sqrtName);
    writer->emit("(");
    writer->emit(dotFuncName);
    writer->emit("(a, a));\n");
   
    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitNormalizeDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{    
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 1);
    IRType* paramType0 = funcType->getParamType(0);

    SLANG_ASSERT(paramType0->op == kIROp_VectorType);

    IRBasicType* elementType = as<IRBasicType>(static_cast<IRVectorType*>(paramType0)->getElementType());

    IRType* dotArgs[] = { paramType0, paramType0 };
    UnownedStringSlice dotFuncName = _getAndEmitSpecializedOperationDefinition(IntrinsicOp::Dot, dotArgs, SLANG_COUNT_OF(dotArgs), elementType);
    UnownedStringSlice rsqrtName = _getScalarFuncName(IntrinsicOp::RecipSqrt, elementType);
    IRType* vecMulScalarArgs[] = { paramType0, elementType };
    UnownedStringSlice vecMulScalarName = _getAndEmitSpecializedOperationDefinition(IntrinsicOp::Mul, vecMulScalarArgs, SLANG_COUNT_OF(vecMulScalarArgs), paramType0);

    TypeDimension dimA = _getTypeDimension(paramType0, false);

    // Assumes C++

    _emitSignature(funcName, specOp);

    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");

    // Assumes C++ here
    writer->emit("a * ");
    writer->emit(rsqrtName);
    writer->emit("(");
    writer->emit(dotFuncName);
    writer->emit("(a, a));\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitReflectDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);
    IRType* paramType0 = funcType->getParamType(0);

    SLANG_ASSERT(paramType0->op == kIROp_VectorType);

    IRBasicType* elementType = as<IRBasicType>(static_cast<IRVectorType*>(paramType0)->getElementType());

    // Make sure we have all these functions defined before emtting 
    IRType* dotArgs[] = { paramType0, paramType0 };
    UnownedStringSlice dotFuncName = _getAndEmitSpecializedOperationDefinition(IntrinsicOp::Dot, dotArgs, SLANG_COUNT_OF(dotArgs), elementType);

    IRType* subArgs[] = { paramType0, paramType0};
    UnownedStringSlice subFuncName = _getAndEmitSpecializedOperationDefinition(IntrinsicOp::Sub, subArgs, SLANG_COUNT_OF(subArgs), paramType0);

    IRType* vecMulScalarArgs[] = { paramType0, elementType };
    UnownedStringSlice vecMulScalarFuncName = _getAndEmitSpecializedOperationDefinition(IntrinsicOp::Mul, vecMulScalarArgs, SLANG_COUNT_OF(vecMulScalarArgs), paramType0);

    // Assumes C++

    _emitSignature(funcName, specOp);
    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return a - b * 2.0 * ");
    writer->emit(dotFuncName);
    writer->emit("(a, b);\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::emitSpecializedOperationDefinition(const SpecializedIntrinsic& specOp)
{
    // Check if it's been emitted already, if not add it.
    if (!m_intrinsicEmittedMap.AddIfNotExists(specOp, true))
    {
        return;
    }

    switch (specOp.op)
    {
        case IntrinsicOp::VecMatMul:
        case IntrinsicOp::Dot:
        {
            return _emitVecMatMulDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::Any:
        case IntrinsicOp::All:
        {
            return _emitAnyAllDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::Cross:
        {
            return _emitCrossDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::Normalize:
        {
            return _emitNormalizeDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::Length:
        {
            return _emitLengthDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::Reflect:
        {
            return _emitReflectDefinition(_getFuncName(specOp), specOp);
        }
        default:
        {
            const auto& info = getOperationInfo(specOp.op);
            if (info.numOperands >= 1 && info.numOperands <= 3)
            {
                return _emitAryDefinition(specOp);
            }
            break;
        }
    }

    SLANG_ASSERT(!"Unhandled");
}

IRType* CPPSourceEmitter::_getVecType(IRType* elementType, int elementCount)
{
    elementType = _cloneType(elementType);
    return m_irBuilder.getVectorType(elementType, m_irBuilder.getIntValue(m_irBuilder.getIntType(), elementCount));
}

CPPSourceEmitter::SpecializedIntrinsic CPPSourceEmitter::getSpecializedOperation(IntrinsicOp op, IRType*const* inArgTypes, int argTypesCount, IRType* retType)
{
    SpecializedIntrinsic specOp;
    specOp.op = op;

    List<IRType*> argTypes;
    argTypes.setCount(argTypesCount);

    for (int i = 0; i < argTypesCount; ++i)
    {
        argTypes[i] = _cloneType(inArgTypes[i]->getCanonicalType());
    }

    specOp.returnType = _cloneType(retType);
    specOp.signatureType = m_irBuilder.getFuncType(argTypes, m_irBuilder.getBasicType(BaseType::Void));

    return specOp;
}

void CPPSourceEmitter::emitCall(const SpecializedIntrinsic& specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec)
{
    SLANG_UNUSED(inOuterPrec);
    SourceWriter* writer = getSourceWriter();

    // Getting the name means that this op is registered as used
    
    switch (specOp.op)
    {
        case IntrinsicOp::Init:
        {
            // For C++ we don't need an init function
            // For C we'll need the construct function for the return type
            //UnownedStringSlice name = _getFuncName(specOp);

            IRType* retType = specOp.returnType;

            switch (retType->op)
            {
                case kIROp_VectorType:
                {
                    // Get the type name
                    emitType(retType);
                    writer->emitChar('{');

                    for (int i = 0; i < numOperands; ++i)
                    {
                        if (i > 0)
                        {
                            writer->emit(", ");
                        }
                        emitOperand(operands[i].get(), getInfo(EmitOp::General));
                    }

                    writer->emitChar('}');
                    break;
                }
                case kIROp_MatrixType:
                {
                    IRMatrixType* matType = static_cast<IRMatrixType*>(retType);

                    //int colsCount = int(GetIntVal(matType->getColumnCount()));
                    int rowsCount = int(GetIntVal(matType->getRowCount()));

                    SLANG_ASSERT(rowsCount == numOperands);

                    emitType(retType);
                    writer->emitChar('{');

                    for (int j = 0; j < rowsCount; ++j)
                    {
                        if (j > 0)
                        {
                            writer->emit(", ");
                        }
                        emitOperand(operands[j].get(), getInfo(EmitOp::General));
                    }

                    writer->emitChar('}');
                    break;
                }
                default:
                {
                    if (IRBasicType::isaImpl(retType->op))
                    {
                        SLANG_ASSERT(numOperands == 1);

                        writer->emit(getBuiltinTypeName(retType->op));
                        writer->emitChar('(');

                        emitOperand(operands[0].get(), getInfo(EmitOp::General));

                        writer->emitChar(')');
                        break;
                    }

                    SLANG_ASSERT(!"Not handled");
                }
            }
            break;
        }
        case IntrinsicOp::Swizzle:
        {
            // For C++ we don't need to emit a swizzle function
            // For C we need a construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);
            const Index elementCount = Index(swizzleInst->getElementCount());

            if (elementCount == 1)
            {
                defaultEmitInstExpr(inst, inOuterPrec);
            }
            else
            {
                // TODO(JS): Not sure this is correct on the parens handling front
                IRType* retType = specOp.returnType;
                emitType(retType);
                writer->emit("{");

                for (Index i = 0; i < elementCount; ++i)
                {
                    if (i > 0)
                    {
                        writer->emit(", ");
                    }

                    auto outerPrec = getInfo(EmitOp::General);

                    auto prec = getInfo(EmitOp::Postfix);
                    emitOperand(swizzleInst->getBase(), leftSide(outerPrec, prec));

                    writer->emit(".");

                    IRInst* irElementIndex = swizzleInst->getElementIndex(i);
                    SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                    IRConstant* irConst = (IRConstant*)irElementIndex;
                    UInt elementIndex = (UInt)irConst->value.intVal;
                    SLANG_RELEASE_ASSERT(elementIndex < 4);

                    writer->emitChar(s_elemNames[elementIndex]);
                }

                writer->emit("}");
            }
            break;
        }
        default:
        {
            const auto& info = getOperationInfo(specOp.op);
            // Make sure that the return type is available
            bool isOperator = _isOperator(info.funcName);

            UnownedStringSlice funcName = _getFuncName(specOp);

            useType(specOp.returnType);
            // add that we want a function
            SLANG_ASSERT(numOperands == info.numOperands);

            if (isOperator)
            {
                // Just do the default output
                defaultEmitInstExpr(inst, inOuterPrec);
            }
            else
            {
                writer->emit(funcName);
                writer->emitChar('(');

                for (int i = 0; i < numOperands; ++i)
                {
                    if (i > 0)
                    {
                        writer->emit(", ");
                    }
                    emitOperand(operands[i].get(), getInfo(EmitOp::General));
                }

                writer->emitChar(')');
            }
            break;
        }
    }
}

StringSlicePool::Handle CPPSourceEmitter::_calcScalarFuncName(IntrinsicOp op, IRBasicType* type)
{
    StringBuilder builder;
    builder << _getTypePrefix(type->op) << "_" << getOperationInfo(op).funcName;
    return m_slicePool.add(builder);
}

UnownedStringSlice CPPSourceEmitter::_getScalarFuncName(IntrinsicOp op, IRBasicType* type)
{
    return m_slicePool.getSlice(_calcScalarFuncName(op, type));
}

UnownedStringSlice CPPSourceEmitter::_getFuncName(const SpecializedIntrinsic& specOp)
{
    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_intrinsicNameMap.TryGetValue(specOp, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    handle = _calcFuncName(specOp);
    m_intrinsicNameMap.Add(specOp, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

StringSlicePool::Handle CPPSourceEmitter::_calcFuncName(const SpecializedIntrinsic& specOp)
{
    if (specOp.isScalar())
    {
        IRType* paramType = specOp.signatureType->getParamType(0);
        IRBasicType* basicType = as<IRBasicType>(paramType);
        SLANG_ASSERT(basicType);
        return _calcScalarFuncName(specOp.op, basicType);
    }
    else
    {
        const auto& info = getOperationInfo(specOp.op);
        if (info.funcName.size())
        {
            if (!_isOperator(info.funcName))
            {
                return m_slicePool.add(info.funcName);
            }
        }
        return m_slicePool.add(info.name);
    }
}

void CPPSourceEmitter::emitOperationCall(IntrinsicOp op, IRInst* inst, IRUse* operands, int operandCount, IRType* retType, const EmitOpInfo& inOuterPrec)
{
    if (operandCount > 8)
    {
        List<IRType*> argTypes;
        argTypes.setCount(operandCount);
        for (int i = 0; i < operandCount; ++i)
        {
            // Hmm.. I'm assuming here that the operands exactly match the usage (ie no casting)
            argTypes[i] = operands[i].get()->getDataType();
        }
        SpecializedIntrinsic specOp = getSpecializedOperation(op, argTypes.getBuffer(), operandCount, retType);
        emitCall(specOp, inst, operands, operandCount, inOuterPrec);
    }
    else
    {
        IRType* argTypes[8];
        for (int i = 0; i < operandCount; ++i)
        {
            // Hmm.. I'm assuming here that the operands exactly match the usage (ie no casting)
            argTypes[i] = operands[i].get()->getDataType();
        }
        SpecializedIntrinsic specOp = getSpecializedOperation(op, argTypes, operandCount, retType);
        emitCall(specOp, inst, operands, operandCount, inOuterPrec);
    }
}

/* !!!!!!!!!!!!!!!!!!!!!! CPPSourceEmitter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

CPPSourceEmitter::CPPSourceEmitter(const Desc& desc):
    Super(desc)
{
    m_sharedIRBuilder.module = nullptr;
    m_sharedIRBuilder.session = desc.compileRequest->getSession();

    m_irBuilder.sharedBuilder = &m_sharedIRBuilder;

    m_uniqueModule = m_irBuilder.createModule();
    m_sharedIRBuilder.module = m_uniqueModule;

    m_irBuilder.setInsertInto(m_irBuilder.getModule()->getModuleInst());

    // Add all the operations with names (not ops like -, / etc) to the lookup map
    for (int i = 0; i < SLANG_COUNT_OF(s_operationInfos); ++i)
    {
        const auto& info = s_operationInfos[i];
        UnownedStringSlice slice = info.funcName;

        if (slice.size() > 0 && slice[0] >= 'a' && slice[0] <= 'z')
        {
            auto handle = m_slicePool.add(slice);
            Index index = Index(handle);
            // Make sure there is space
            if (index >= m_intrinsicOpMap.getCount())
            {
                Index oldSize = m_intrinsicOpMap.getCount();
                m_intrinsicOpMap.setCount(index + 1);
                for (Index j = oldSize; j < index; j++)
                {
                    m_intrinsicOpMap[j] = IntrinsicOp::Invalid;
                }
            }
            m_intrinsicOpMap[index] = IntrinsicOp(i);
        }
    }
}

void CPPSourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    SLANG_UNUSED(varDecl);
    SLANG_UNUSED(type);
    SLANG_ASSERT(!"Not implemented");
}

void CPPSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, EntryPointLayout* entryPointLayout)
{
    SLANG_UNUSED(irFunc);

    auto profile = m_effectiveProfile;
    auto stage = entryPointLayout->profile.GetStage();

    switch (stage)
    {
        case Stage::Compute:
        {
            static const UInt kAxisCount = 3;
            UInt sizeAlongAxis[kAxisCount];

            // TODO: this is kind of gross because we are using a public
            // reflection API function, rather than some kind of internal
            // utility it forwards to...
            spReflectionEntryPoint_getComputeThreadGroupSize(
                (SlangReflectionEntryPoint*)entryPointLayout,
                kAxisCount,
                &sizeAlongAxis[0]);

            // TODO(JS): We might want to store this information such that it can be used to execute
            m_writer->emit("// [numthreads(");
            for (int ii = 0; ii < 3; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                m_writer->emit(sizeAlongAxis[ii]);
            }
            m_writer->emit(")]\n");
            break;
        }
        default: break;
    }

    m_writer->emit("SLANG_PRELUDE_EXPORT\n");
}

void CPPSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    emitSimpleType(_getVecType(elementType, int(elementCount)));
}

void CPPSourceEmitter::emitSimpleTypeImpl(IRType* inType)
{
     
    UnownedStringSlice slice = _getTypeName(_cloneType(inType));
    m_writer->emit(slice);
}

void CPPSourceEmitter::emitTypeImpl(IRType* type, const StringSliceLoc* nameLoc)
{
    UnownedStringSlice slice = _getTypeName(type);
    m_writer->emit(slice);

    if (nameLoc)
    {
        m_writer->emit(" ");
        m_writer->emitName(*nameLoc);
    }
}

void CPPSourceEmitter::emitIntrinsicCallExpr(IRCall* inst, IRFunc* func, EmitOpInfo const& inOuterPrec)
{
    auto outerPrec = inOuterPrec;
    bool needClose = false;

    // For a call with N arguments, the instruction will
    // have N+1 operands. We will start consuming operands
    // starting at the index 1.
    UInt operandCount = inst->getOperandCount();
    UInt argCount = operandCount - 1;
    UInt operandIndex = 1;

    // Our current strategy for dealing with intrinsic
    // calls is to "un-mangle" the mangled name, in
    // order to figure out what the user was originally
    // calling. This is a bit messy, and there might
    // be better strategies (including just stuffing
    // a pointer to the original decl onto the callee).

    // If the intrinsic the user is calling is a generic,
    // then the mangled name will have been set on the
    // outer-most generic, and not on the leaf value
    // (which is `func` above), so we need to walk
    // upwards to find it.
    //
    IRInst* valueForName = func;
    for (;;)
    {
        auto parentBlock = as<IRBlock>(valueForName->parent);
        if (!parentBlock)
            break;

        auto parentGeneric = as<IRGeneric>(parentBlock->parent);
        if (!parentGeneric)
            break;

        valueForName = parentGeneric;
    }

    // If we reach this point, we are assuming that the value
    // has some kind of linkage, and thus a mangled name.
    //
    auto linkageDecoration = valueForName->findDecoration<IRLinkageDecoration>();
    SLANG_ASSERT(linkageDecoration);
    
    // We will use the `MangledLexer` to
    // help us split the original name into its pieces.
    MangledLexer lexer(linkageDecoration->getMangledName());

    // We'll read through the qualified name of the
    // symbol (e.g., `Texture2D<T>.Sample`) and then
    // only keep the last segment of the name (e.g.,
    // the `Sample` part).
    auto name = lexer.readSimpleName();

    // We will special-case some names here, that
    // represent callable declarations that aren't
    // ordinary functions, and thus may use different
    // syntax.
    if (name == "operator[]")
    {
        // The user is invoking a built-in subscript operator

        auto prec = getInfo(EmitOp::Postfix);
        needClose = maybeEmitParens(outerPrec, prec);

        emitOperand(inst->getOperand(operandIndex++), leftSide(outerPrec, prec));
        m_writer->emit("[");
        emitOperand(inst->getOperand(operandIndex++), getInfo(EmitOp::General));
        m_writer->emit("]");

        if (operandIndex < operandCount)
        {
            m_writer->emit(" = ");
            emitOperand(inst->getOperand(operandIndex++), getInfo(EmitOp::General));
        }

        maybeCloseParens(needClose);
        return;
    }

    auto prec = getInfo(EmitOp::Postfix);
    needClose = maybeEmitParens(outerPrec, prec);

    // The mangled function name currently records
    // the number of explicit parameters, and thus
    // doesn't include the implicit `this` parameter.
    // We can compare the argument and parameter counts
    // to figure out whether we have a member function call.
    UInt paramCount = lexer.readParamCount();

    if (argCount != paramCount)
    {
        // Looks like a member function call
        emitOperand(inst->getOperand(operandIndex), leftSide(outerPrec, prec));
        m_writer->emit(".");
        operandIndex++;
    }
    else
    {
        IntrinsicOp op = getOperationByName(name);
        if (op != IntrinsicOp::Invalid)
        {
            IRUse* operands = inst->getOperands() + operandIndex;
            emitOperationCall(op, inst, operands, int(operandCount - operandIndex), inst->getDataType(), inOuterPrec);
            return;
        }
    }
  
    m_writer->emit(name);
    m_writer->emit("(");
    bool first = true;
    for (; operandIndex < operandCount; ++operandIndex)
    {
        if (!first) m_writer->emit(", ");
        emitOperand(inst->getOperand(operandIndex), getInfo(EmitOp::General));
        first = false;
    }
    m_writer->emit(")");
    maybeCloseParens(needClose);
}

bool CPPSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    SLANG_UNUSED(inOuterPrec);

    switch (inst->op)
    {
        case kIROp_Construct:
        case kIROp_makeVector:
        case kIROp_MakeMatrix:
        {
            emitOperationCall(IntrinsicOp::Init, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
            return true;
        }
        case kIROp_Mul_Matrix_Matrix:
        case kIROp_Mul_Matrix_Vector:
        case kIROp_Mul_Vector_Matrix:
        {
            emitOperationCall(IntrinsicOp::VecMatMul, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
            return true;
        }
        case kIROp_Dot:
        {
            emitOperationCall(IntrinsicOp::Dot, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
            return true;
        }
        case kIROp_swizzle:
        {
            emitOperationCall(IntrinsicOp::Swizzle, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
            return true;
        }
        case kIROp_Call:
        {
            auto funcValue = inst->getOperand(0);

            // Does this function declare any requirements.
            handleCallExprDecorationsImpl(funcValue);

            // We want to detect any call to an intrinsic operation,
            // that we can emit it directly without mangling, etc.
            if (auto irFunc = asTargetIntrinsic(funcValue))
            {
                emitIntrinsicCallExpr(static_cast<IRCall*>(inst), irFunc, inOuterPrec);
                return true;
            }

            return false;
        }
        default:
        {
            IntrinsicOp op = getOperation(inst->op);
            if (op != IntrinsicOp::Invalid)
            {
                emitOperationCall(op, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
                return true;
            }
            return false;
        }
    }
}

void CPPSourceEmitter::emitPreprocessorDirectivesImpl()
{
    SourceWriter* writer = getSourceWriter();

    writer->emit("\n");

    writer->emit("#include <slang-cpp-prelude.h>\n\n");

    // Emit the type definitions
    for (const auto& keyValue : m_typeNameMap)
    {
        emitTypeDefinition(keyValue.Key);
    }

    // Emit all the intrinsics that were used

    for (const auto& keyValue : m_intrinsicNameMap)
    {
        emitSpecializedOperationDefinition(keyValue.Key);
    }

    // Lets take a look at layout

    ProgramLayout* programLayout = m_programLayout;

    if (programLayout)
    {


    }
}

} // namespace Slang
