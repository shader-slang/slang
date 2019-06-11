// slang-emit-cpp.cpp
#include "slang-emit-cpp.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {


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
            // Promote all these to Int16, we can just vary the call to make these work
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
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_DoubleType:      return UnownedStringSlice::fromLiteral("F64");
        default:                    return UnownedStringSlice::fromLiteral("?");
    }
}

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

void CPPSourceEmitter::_emitCVecType(IROp op, Int size)
{
    m_writer->emit("Vec");
    const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(op));
    m_writer->emit(postFix);
    if (postFix.size() > 1)
    {
        m_writer->emit("_");
    }
    m_writer->emit(size);
}

void CPPSourceEmitter::_emitCMatType(IROp op, IRIntegerValue rowCount, IRIntegerValue colCount)
{
    m_writer->emit("Mat");
    const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(op));
    m_writer->emit(postFix);
    if (postFix.size() > 1)
    {
        m_writer->emit("_");
    }
    m_writer->emit(rowCount);
    m_writer->emit(colCount);
}

void CPPSourceEmitter::_emitCFunc(BuiltInCOp cop, IRType* type)
{
    emitSimpleType(type);
    m_writer->emit("_");

    switch (cop)
    {
        case BuiltInCOp::Init:  m_writer->emit("init");
        case BuiltInCOp::Splat: m_writer->emit("splat"); break;
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
    SLANG_UNUSED(entryPointLayout);
    SLANG_ASSERT(!"Not implemented");
}

void CPPSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    _emitCVecType(elementType->op, Int(elementCount));
}

void CPPSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->op)
    {
        case kIROp_VoidType:
        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_Int64Type:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_FloatType:
        case kIROp_DoubleType:
        {
            m_writer->emit(getDefaultBuiltinTypeName(type->op));
            return;
        }
        case kIROp_HalfType:
        {
            m_writer->emit("float");
            return;
        }
        case kIROp_StructType:
            m_writer->emit(getName(type));
            return;

        case kIROp_VectorType:
        {
            auto vecType = (IRVectorType*)type;
            emitVectorTypeNameImpl(vecType->getElementType(), GetIntVal(vecType->getElementCount()));
            return;
        }
        case kIROp_MatrixType:
        {
            auto matType = (IRMatrixType*)type;

            const auto rowCount = GetIntVal(matType->getRowCount());
            const auto colCount = GetIntVal(matType->getColumnCount());

            _emitCMatType(matType->getElementType()->op, rowCount, colCount);

            return;
        }
    }

    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled type for cpp target");
}


bool CPPSourceEmitter::tryEmitInstExprImpl(IRInst* inst, IREmitMode mode, const EmitOpInfo& inOuterPrec)
{
    SLANG_UNUSED(inOuterPrec);

    //EmitOpInfo outerPrec = inOuterPrec;
    //bool needClose = false;
    switch (inst->op)
    {
        case kIROp_Construct:
        case kIROp_makeVector:
        case kIROp_MakeMatrix:
            // Simple constructor call
            if (inst->getOperandCount() == 1)
            {
                _emitCFunc(BuiltInCOp::Splat, inst->getDataType());
                emitArgs(inst, mode);
            }
            else
            {
                _emitCFunc(BuiltInCOp::Init, inst->getDataType());
                emitArgs(inst, mode);
            }
            return true;
        default:
            return false;
    }
}

} // namespace Slang
