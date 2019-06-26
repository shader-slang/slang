// slang-emit-cpp.cpp
#include "slang-emit-cpp.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

CPPSourceEmitter::CPPSourceEmitter(const Desc& desc):
    Super(desc)
{

#if 0
    
    auto session = desc.compileRequest->getSession();

    auto module = desc.compileRequest->getProgram()->getOrCreateIRModule(nullptr);
    m_sharedBuilder.session = session;
    m_sharedBuilder.module = module;
    m_builder.sharedBuilder = &m_sharedBuilder;
#endif
}
namespace { // anonymous

struct OrderType
{
    OrderType(IRType* inType):
        order(_calcOrder(inType)),
        type(inType)
    {
    }

    OrderType() {}

    static uint32_t _calcOrder(IRType* inType)
    {
        switch (inType->op)
        {
            case kIROp_VectorType:
            {
                IRVectorType* vec = static_cast<IRVectorType*>(inType);
                return (uint32_t(vec->getElementType()->op) << 8) + uint32_t(GetIntVal(vec->getElementCount()));
            }
            case kIROp_MatrixType:
            {
                IRMatrixType* mat = static_cast<IRMatrixType*>(inType);
                return uint32_t(0x01000000) + (uint32_t(mat->getElementType()->op) << 8) + uint32_t(GetIntVal(mat->getColumnCount()) << 4) + uint32_t(GetIntVal(mat->getRowCount()));
            }
            default: return 0;
        }
    }

    SLANG_FORCE_INLINE bool operator<(const OrderType& rhs) const { return order < rhs.order; }
    SLANG_FORCE_INLINE bool operator==(const OrderType& rhs) const { return order == rhs.order; }
    SLANG_FORCE_INLINE bool operator!=(const OrderType& rhs) const { return order == rhs.order; }

    uint32_t order;
    IRType* type;
};


} // anonymous

void CPPSourceEmitter::emitPreprocessorDirectivesImpl()
{
    // Can emit built in type definitions here
    // Can emit intrinsics here too

    List<OrderType> types;
    for (const auto keyValue : m_typeNameMap)
    {
        IRType* inst = keyValue.Key;
        if (inst->op == kIROp_VectorType || inst->op == kIROp_MatrixType)
        {
            types.add(inst);
        }
    }

    // Sort 
    types.sort();

    // Dump out all of the types in order

    for (const auto& orderType : types)
    {
        IRType* type = orderType.type;

        switch (type->op)
        {
            case kIROp_VectorType:
            {
                IRVectorType* vec = static_cast<IRVectorType*>(type);
                IRType* elemType = vec->getElementType();
                int count = int(GetIntVal(vec->getElementCount()));

                SLANG_ASSERT(count > 0 && count < 4);

                UnownedStringSlice typeName = _getTypeName(type);
                UnownedStringSlice elemName = _getTypeName(elemType);

                const char elemNames[] = "xyzw";

                m_writer->emit("struct ");
                m_writer->emit(typeName);
                m_writer->emit("\n{\n");
                m_writer->indent();

                m_writer->emit(elemName);
                m_writer->emit(" ");
                for (int i = 0; i < count; ++i)
                {
                    if (i > 0)
                    {
                        m_writer->emit(", ");
                    }
                    m_writer->emitChar(elemNames[i]);
                }
                m_writer->emit(";\n");

                m_writer->dedent();
                m_writer->emit("};\n\n");
                break;
            }
            case kIROp_MatrixType:
            {
                IRMatrixType* mat = static_cast<IRMatrixType*>(type);
                IRType* elemType = mat->getElementType();

                const auto rowCount = GetIntVal(mat->getRowCount());
                const auto colCount = GetIntVal(mat->getColumnCount());

                UnownedStringSlice typeName = _getTypeName(type);
                UnownedStringSlice rowTypeName = _getTypeName(_getVectorType(elemType, int(colCount)));

                m_writer->emit("struct ");
                m_writer->emit(typeName);
                m_writer->emit("\n{\n");
                m_writer->indent();

                m_writer->emit(rowTypeName);
                m_writer->emit(" rows[");
                m_writer->emit(rowCount);
                m_writer->emit("];\n");

                m_writer->dedent();
                m_writer->emit("};\n\n");
                break;
            }
            default:
            {
                SLANG_ASSERT(!"Unhandled type");
                break;
            }
        }
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

StringSlicePool::Handle CPPSourceEmitter::_calcTypeName(IRType* type)
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
            return m_slicePool.add(getDefaultBuiltinTypeName(type->op));
        }
        case kIROp_HalfType:    
        {
            return m_slicePool.add(getDefaultBuiltinTypeName(kIROp_FloatType));
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            auto vecSize = GetIntVal(vecType->getElementCount());

            StringBuilder builder;
            builder << "Vec";
            UnownedStringSlice postFix = _getCTypeVecPostFix(vecType->getElementType()->op);

            builder << postFix;
            if (postFix.size() > 1)
            {
                builder << "_";
            }
            builder << vecSize;
            return m_slicePool.add(builder);
        }
        case kIROp_MatrixType:
        {
            auto matType = (IRMatrixType*)type;

            auto elemType = matType->getElementType();

            const auto rowCount = GetIntVal(matType->getRowCount());
            const auto colCount = GetIntVal(matType->getColumnCount());

            // Make sure there is the vector name too
            _getTypeName(_getVectorType(elemType, int(colCount)));
            
            StringBuilder builder;

            builder << "Mat";
            const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(elemType->op));
            builder << postFix;
            if (postFix.size() > 1)
            {
                builder << "_";
            }
            builder << rowCount;
            builder << colCount;

            return m_slicePool.add(builder);
        }
        default: break;
    }
    
    return StringSlicePool::kNullHandle;
}

UnownedStringSlice CPPSourceEmitter::_getTypeName(IRType* type)
{
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
    //SLANG_ASSERT(!"Not implemented");
}

IRType* CPPSourceEmitter::_getVectorType(IRType* elemType, int count)
{
#if 1
    SharedIRBuilder sharedBuilder;
    sharedBuilder.session = m_compileRequest->getSession();
    sharedBuilder.module = m_module;
    
    IRBuilder builder;
    builder.sharedBuilder = &sharedBuilder;

    IRInst* elemCountInst = builder.getIntValue(builder.getIntType(), count);
    return builder.getVectorType(elemType, elemCountInst);

#else

    // Construct as a vector type, such that we have in the map
    IRInst* elemCountInst = m_builder.getIntValue(m_builder.getIntType(), count);
    return m_builder.getVectorType(elemType, elemCountInst);

#endif
}

void CPPSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    return emitSimpleTypeImpl(_getVectorType(elementType, int(elementCount)));
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
        case kIROp_HalfType:
        case kIROp_VectorType:
        case kIROp_MatrixType:
        {
            UnownedStringSlice slice = _getTypeName(type);
            m_writer->emit(slice);
            return;
        }
        case kIROp_StructType:
        {
            m_writer->emit(getName(type));
            return;
        }
        case kIROp_HLSLRWStructuredBufferType:
        {
            m_writer->emit("RWStructuredBuffer");
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
        case kIROp_Call:
        {

            return false;
        }
        default:
            return false;
    }
}

} // namespace Slang
