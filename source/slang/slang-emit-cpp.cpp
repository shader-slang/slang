// slang-emit-cpp.cpp
#include "slang-emit-cpp.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include "slang-ir-clone.h"

#include <assert.h>

namespace Slang {

static const char s_elemNames[] = "xyzw";

#if 0
static UnownedStringSlice _getCTypeName(IROp op)
{
    switch (op)
    {
        case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("Bool");
        case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I32");
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F32");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
        default:                    return UnownedStringSlice::fromLiteral("?");
    }
}
#endif



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

CPPEmitHandler::CPPEmitHandler(const CLikeSourceEmitter::Desc& desc)
{
    m_sharedIRBuilder.module = nullptr;
    m_sharedIRBuilder.session = desc.compileRequest->getSession();

    m_irBuilder.sharedBuilder = &m_sharedIRBuilder;

    m_uniqueModule = m_irBuilder.createModule();
    m_sharedIRBuilder.module = m_uniqueModule;

    m_irBuilder.setInsertInto(m_irBuilder.getModule()->getModuleInst());
}


/* static */ UnownedStringSlice CPPEmitHandler::getBuiltinTypeName(IROp op)
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

static const CPPEmitHandler::OperationInfo s_operationInfos[] =
{
#define SLANG_CPP_OPERATION_INFO(x, funcName) { UnownedStringSlice::fromLiteral(#x), UnownedStringSlice::fromLiteral(funcName) },
    SLANG_CPP_OPERATION(SLANG_CPP_OPERATION_INFO)
};

/* static */const CPPEmitHandler::OperationInfo& CPPEmitHandler::getOperationInfo(Operation op)
{
    return s_operationInfos[int(op)];
}

/* static */CPPEmitHandler::Operation CPPEmitHandler::getOperation(IROp op)
{
    switch (op)
    {
        case kIROp_Add:     return Operation::Add;
        case kIROp_Mul:     return Operation::Mul;
        case kIROp_Sub:     return Operation::Sub;
        case kIROp_Div:     return Operation::Div;
        case kIROp_Lsh:     return Operation::Lsh;
        case kIROp_Rsh:     return Operation::Rsh;
        case kIROp_Mod:     return Operation::Mod;
        default:            return Operation::Invalid;
    }
}

CPPEmitHandler::Operation CPPEmitHandler::getOperationByName(const UnownedStringSlice& slice)
{
    if (slice == "dot")
    {
        return Operation::Dot;
    }

    SLANG_UNUSED(slice);
    return Operation::Invalid;
}

void CPPEmitHandler::emitTypeDefinition(IRType* inType, CPPSourceEmitter* emitter)
{
    IRType* type = _cloneType(inType);
    if (m_typeEmittedMap.TryGetValue(type))
    {
        return;
    }

    SourceWriter* writer = emitter->getSourceWriter();

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
            emitTypeDefinition(vecType, emitter);

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
        case kIROp_HLSLRWStructuredBufferType:
        {
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

UnownedStringSlice CPPEmitHandler::_getTypeName(IRType* inType)
{
    // TODO(JS): Arguably this doesn't entirely work.
    // I don't want to _clone a thing that is nominal, and not needed to be generated. But then we have
    // We don't add it to the map, because the type is form an external module...

    // We don't handle nominally named things
    switch (inType->op)
    {
        case kIROp_StructType:
        {
            return UnownedStringSlice::fromLiteral("");
        }
        default: break;
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

StringSlicePool::Handle CPPEmitHandler::_calcTypeName(IRType* type)
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

void CPPEmitHandler::useType(IRType* type)
{
    _getTypeName(type);
}

void CPPEmitHandler::emitType(IRType* inType, CPPSourceEmitter* emitter)
{
    SourceWriter* writer = emitter->getSourceWriter();

    IRType* type = _cloneType(inType);


    UnownedStringSlice slice = _getTypeName(type);
    writer->emit(slice);
    
    //SLANG_DIAGNOSE_UNEXPECTED(emitter->getSink(), SourceLoc(), "unhandled type for cpp target");
}

void CPPEmitHandler::emitPreamble(CPPSourceEmitter* emitter)
{
    SourceWriter* writer = emitter->getSourceWriter();

    writer->emit("\n");
    writer->emit("#include \"slang-cpp-prelude.h\"\n\n");
    
    // Emit the type definitions
    for (const auto& keyValue : m_typeNameMap)
    {
        emitTypeDefinition(keyValue.Key, emitter);
    }

    // Emit all the intrinsics that were used

    for (const auto& keyValue : m_specializeOperationNameMap)
    {
        emitSpecializedOperationDefinition(keyValue.Key, emitter);
    }
}

IRInst* CPPEmitHandler::_clone(IRInst* inst)
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
#if 0
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(inst);
            clone = m_irBuilder.getMatrixType(_cloneType(matType->getElementType()),  _clone(matType->getRowCount()), _clone(matType->getColumnCount()));
            break;
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(inst);
            clone = m_irBuilder.getVectorType(_cloneType(vecType->getElementType()), _clone(vecType->getElementCount()));
            break;
        }
#endif
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

/* static */CPPEmitHandler::Dimension CPPEmitHandler::_getDimension(IRType* type, bool vecSwap)
{
    switch (type->op)
    {
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            const int elemCount = int(GetIntVal(vecType->getElementCount()));
            return (!vecSwap) ? Dimension{1, elemCount} : Dimension{ elemCount, 1};
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);
            const int colCount = int(GetIntVal(matType->getColumnCount()));
            const int rowCount = int(GetIntVal(matType->getRowCount()));
            return Dimension{rowCount, colCount};
        }
        default: return Dimension{1, 1};
    }
}

/* static */void CPPEmitHandler::_emitAccess(const UnownedStringSlice& name, const Dimension& dimension, int row, int col, SourceWriter* writer)
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

void CPPEmitHandler::_emitParameter(char nameChar, IRType* type, const Dimension& dim, CPPSourceEmitter* emitter)
{
    SourceWriter* writer = emitter->getSourceWriter();

    writer->emit("const ");
    emitType(type, emitter);

    if (dim.isScalar())
    {
       writer->emit(" ");
    }
    else
    {
        writer->emit("& ");
    }
    writer->emitChar(nameChar);
}

void CPPEmitHandler::_emitBinaryOp(const SpecializedOperation& specOp, CPPSourceEmitter* emitter)
{
    auto info = getOperationInfo(specOp.op);
    auto funcName = info.funcName;
    SLANG_ASSERT(funcName.size() > 0);

    SourceWriter* writer = emitter->getSourceWriter();

    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);
    IRType* paramType0 = funcType->getParamType(0);
    IRType* paramType1 = funcType->getParamType(1);
    IRType* retType = specOp.returnType;

    // If both inputs are scalar, no definition is needed
    
    const Dimension dimA = _getDimension(paramType0, false);
    const Dimension dimB = _getDimension(paramType1, false);
    const Dimension resultDim = _getDimension(retType, false);

    // If both basic then we are done
    if (dimA.isScalar() && dimB.isScalar())
    {
        return;
    }

    emitType(retType, emitter);

    writer->emit(" operator");
    writer->emit(funcName);
    writer->emit("(");
    _emitParameter('a', paramType0, dimA, emitter);
    writer->emit(", ");
    _emitParameter('b', paramType1, dimB, emitter);
    writer->emit(")\n{\n");
    writer->indent();

    emitType(retType, emitter);
    writer->emit(" r;\n");

    for (int i = 0; i < resultDim.rowCount; ++i)
    {
        for (int j = 0; j < resultDim.colCount; ++j)
        {
            _emitAccess(UnownedStringSlice::fromLiteral("r"), resultDim, i, j, writer);
            writer->emit(" = ");

            _emitAccess(UnownedStringSlice::fromLiteral("a"), dimA, i, j, writer);
            writer->emit(" ");
            writer->emit(funcName);
            writer->emit(" ");
            _emitAccess(UnownedStringSlice::fromLiteral("b"), dimB, i, j, writer);
            writer->emit(";\n");
        }
    }
    
    writer->emit("return r;\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPEmitHandler::_emitVecMatMul(const UnownedStringSlice& funcName, const SpecializedOperation& specOp, CPPSourceEmitter* emitter)
{
    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);
    IRType* paramType0 = funcType->getParamType(0);
    IRType* paramType1 = funcType->getParamType(1);
    IRType* retType = specOp.returnType;

    SourceWriter* writer = emitter->getSourceWriter();

    emitType(retType, emitter);
    writer->emit(" ");
    writer->emit(funcName);
    writer->emit("(");
    writer->emit("const ");
    emitType(paramType0, emitter);
    writer->emit("& a, const ");
    emitType(paramType1, emitter);
    writer->emit("& b)\n{\n");
    writer->indent();

    emitType(retType, emitter);
    writer->emit(" r;\n");

    Dimension dimA = _getDimension(paramType0, false);
    Dimension dimB = _getDimension(paramType1, true);
    Dimension resultDim = _getDimension(retType, paramType1->op == kIROp_VectorType);

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

void CPPEmitHandler::emitSpecializedOperationDefinition(const SpecializedOperation& specOp, CPPSourceEmitter* emitter)
{
    SLANG_UNUSED(emitter);
    SLANG_UNUSED(specOp);
    // Do nothing for now

    switch (specOp.op)
    {
        case Operation::VecMatMul:
        {
            return _emitVecMatMul(UnownedStringSlice::fromLiteral("VecMatMul"), specOp, emitter);
        }
        case Operation::Dot:
        {
            return _emitVecMatMul(UnownedStringSlice::fromLiteral("Dot"), specOp, emitter);
        }
        case Operation::Mul:
        case Operation::Div:
        case Operation::Add:
        case Operation::Sub:
        case Operation::Lsh:
        case Operation::Rsh:
        case Operation::Mod:
        {
            return _emitBinaryOp(specOp, emitter);
        }
        default: break;
    }
}

IRType* CPPEmitHandler::_getVecType(IRType* elementType, int elementCount)
{
    elementType = _cloneType(elementType);
    return m_irBuilder.getVectorType(elementType, m_irBuilder.getIntValue(m_irBuilder.getIntType(), elementCount));
}

void CPPEmitHandler::emitVectorTypeName(IRType* elementType, int elementCount, CPPSourceEmitter* emitter)
{
    emitType(_getVecType(elementType, elementCount), emitter);
}

CPPEmitHandler::SpecializedOperation CPPEmitHandler::getSpecializedOperation(Operation op, IRType*const* inArgTypes, int argTypesCount, IRType* retType)
{
    SpecializedOperation specOp;
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

void CPPEmitHandler::emitCall(const SpecializedOperation& specOp, IRInst* inst, const IRUse* operands, int numOperands, CLikeSourceEmitter::IREmitMode mode, const EmitOpInfo& inOuterPrec, CPPSourceEmitter* emitter)
{
    SLANG_UNUSED(inOuterPrec);
    SourceWriter* writer = emitter->getSourceWriter();

    // Getting the name means that this op is registered as used
    
    switch (specOp.op)
    {
        case Operation::Init:
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
                    emitType(retType, emitter);
                    writer->emitChar('{');

                    for (int i = 0; i < numOperands; ++i)
                    {
                        if (i > 0)
                        {
                            writer->emit(", ");
                        }
                        emitter->emitOperand(operands[i].get(), mode, getInfo(EmitOp::General));
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

                    emitType(retType, emitter);
                    writer->emitChar('{');

                    for (int j = 0; j < rowsCount; ++j)
                    {
                        if (j > 0)
                        {
                            writer->emit(", ");
                        }
                        emitter->emitOperand(operands[j].get(), mode, getInfo(EmitOp::General));
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

                        emitter->emitOperand(operands[0].get(), mode, getInfo(EmitOp::General));

                        writer->emitChar(')');
                        break;
                    }

                    SLANG_ASSERT(!"Not handled");
                }
            }
            break;
        }
        case Operation::Swizzle:
        {
            // For C++ we don't need to emit a swizzle function
            // For C we need a construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);
            const Index elementCount = Index(swizzleInst->getElementCount());

            if (elementCount == 1)
            {
                emitter->defaultEmitInstExpr(inst, mode, inOuterPrec);
            }
            else
            {
                // TODO(JS): Not sure this is correct on the parens handling front
                IRType* retType = specOp.returnType;
                emitType(retType, emitter);
                writer->emit("{");

                for (Index i = 0; i < elementCount; ++i)
                {
                    if (i > 0)
                    {
                        writer->emit(", ");
                    }

                    auto outerPrec = getInfo(EmitOp::General);

                    auto prec = getInfo(EmitOp::Postfix);
                    emitter->emitOperand(swizzleInst->getBase(), mode, leftSide(outerPrec, prec));

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
        case Operation::Mul:
        case Operation::Div:
        case Operation::Add:
        case Operation::Sub:
        case Operation::Lsh:
        case Operation::Rsh:
        case Operation::Mod:
        {
            SLANG_ASSERT(numOperands == 2);

            // add that we want a function
            _getFuncName(specOp);
            // Just do the default output
            emitter->defaultEmitInstExpr(inst, mode, inOuterPrec);
            break;
        }
        default:
        {
            UnownedStringSlice name = _getFuncName(specOp);
            writer->emit(name);
            writer->emitChar('(');

            for (int i = 0; i < numOperands; ++i)
            {
                if (i > 0)
                {
                    writer->emit(", ");
                }
                emitter->emitOperand(operands[i].get(), mode, getInfo(EmitOp::General));
            }

            writer->emitChar(')');
            break;
        }
    }
}

UnownedStringSlice CPPEmitHandler::_getFuncName(const SpecializedOperation& specOp)
{
    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_specializeOperationNameMap.TryGetValue(specOp, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    handle = _calcFuncName(specOp);
    m_specializeOperationNameMap.Add(specOp, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

StringSlicePool::Handle CPPEmitHandler::_calcFuncName(const SpecializedOperation& specOp)
{
    if (specOp.isScalar())
    {
        StringBuilder builder;
        builder << "HLSLIntrinsic::";
        builder << getOperationInfo(specOp.op).name;
        return m_slicePool.add(builder);
    }
    else
    {
        return m_slicePool.add(getOperationInfo(specOp.op).name);
    }
}

void CPPEmitHandler::emitOperationCall(Operation op, IRInst* inst, IRUse* operands, int operandCount, IRType* retType, CLikeSourceEmitter::IREmitMode mode, const EmitOpInfo& inOuterPrec, CPPSourceEmitter* emitter)
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
        CPPEmitHandler::SpecializedOperation specOp = getSpecializedOperation(op, argTypes.getBuffer(), operandCount, retType);
        emitCall(specOp, inst, operands, operandCount, mode, inOuterPrec, emitter);
    }
    else
    {
        IRType* argTypes[8];
        for (int i = 0; i < operandCount; ++i)
        {
            // Hmm.. I'm assuming here that the operands exactly match the usage (ie no casting)
            argTypes[i] = operands[i].get()->getDataType();
        }
        CPPEmitHandler::SpecializedOperation specOp = getSpecializedOperation(op, argTypes, operandCount, retType);
        emitCall(specOp, inst, operands, operandCount, mode, inOuterPrec, emitter);
    }
}

/* !!!!!!!!!!!!!!!!!!!!!! CPPSourceEmitter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

CPPSourceEmitter::CPPSourceEmitter(const Desc& desc, CPPEmitHandler* emitHandler):
    Super(desc),
    m_emitHandler(emitHandler)
{
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
    SLANG_UNUSED(entryPointLayout);
    //SLANG_ASSERT(!"Not implemented");
}

void CPPSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    m_emitHandler->emitVectorTypeName(elementType, int(elementCount), this);
}

void CPPSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->op)
    {
        case kIROp_StructType:
        {
            m_writer->emit(getName(type));
            break;
        }
        default:
        {
            m_emitHandler->emitType(type, this);
            break;
        }
    }
}

void CPPSourceEmitter::emitIntrinsicCallExpr(IRCall* inst, IRFunc* func, IREmitMode mode, EmitOpInfo const& inOuterPrec)
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

        emitOperand(inst->getOperand(operandIndex++), mode, leftSide(outerPrec, prec));
        m_writer->emit("[");
        emitOperand(inst->getOperand(operandIndex++), mode, getInfo(EmitOp::General));
        m_writer->emit("]");

        if (operandIndex < operandCount)
        {
            m_writer->emit(" = ");
            emitOperand(inst->getOperand(operandIndex++), mode, getInfo(EmitOp::General));
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
        emitOperand(inst->getOperand(operandIndex), mode, leftSide(outerPrec, prec));
        m_writer->emit(".");
        operandIndex++;
    }
    else
    {
        CPPEmitHandler::Operation op = m_emitHandler->getOperationByName(name);
        if (op != CPPEmitHandler::Operation::Invalid)
        {
            IRUse* operands = inst->getOperands() + operandIndex;

            m_emitHandler->emitOperationCall(op, inst, operands, int(operandCount - operandIndex), inst->getDataType(), mode, inOuterPrec, this);
            return;
        }
    }
  
    m_writer->emit(name);
    m_writer->emit("(");
    bool first = true;
    for (; operandIndex < operandCount; ++operandIndex)
    {
        if (!first) m_writer->emit(", ");
        emitOperand(inst->getOperand(operandIndex), mode, getInfo(EmitOp::General));
        first = false;
    }
    m_writer->emit(")");
    maybeCloseParens(needClose);
}

bool CPPSourceEmitter::tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec)
{
    typedef CPPEmitHandler::Operation Operation;
    SLANG_UNUSED(inOuterPrec);

    switch (inst->op)
    {
        case kIROp_Construct:
        case kIROp_makeVector:
        case kIROp_MakeMatrix:
        {
            m_emitHandler->emitOperationCall(Operation::Init, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), mode, inOuterPrec, this);
            return true;
        }
        case kIROp_Mul_Matrix_Matrix:
        case kIROp_Mul_Matrix_Vector:
        case kIROp_Mul_Vector_Matrix:
        {
            m_emitHandler->emitOperationCall(Operation::VecMatMul, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), mode, inOuterPrec, this);
            return true;
        }
        case kIROp_Dot:
        {
            m_emitHandler->emitOperationCall(Operation::Dot, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), mode, inOuterPrec, this);
            return true;
        }
        case kIROp_swizzle:
        {
            m_emitHandler->emitOperationCall(Operation::Swizzle, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), mode, inOuterPrec, this);
            return true;
        }
        case kIROp_Add:
        case kIROp_Mul:
        case kIROp_Sub:
        case kIROp_Div:
        case kIROp_Lsh:
        case kIROp_Rsh:
        case kIROp_Mod:
        {
            Operation op = CPPEmitHandler::getOperation(inst->op);
            SLANG_ASSERT(op != Operation::Invalid);
            m_emitHandler->emitOperationCall(op, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), mode, inOuterPrec, this);
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
                emitIntrinsicCallExpr(static_cast<IRCall*>(inst), irFunc, mode, inOuterPrec);
                return true;
            }

            return false;
        }
        default:
        {
            
            return false;
        }
    }
}

void CPPSourceEmitter::emitPreprocessorDirectivesImpl()
{
    m_emitHandler->emitPreamble(this);
}

} // namespace Slang
