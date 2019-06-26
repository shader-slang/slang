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
}

/* static */ UnownedStringSlice CPPSourceEmitter::getBuiltinTypeName(IROp op)
{
    switch (op)
    {
        case kIROp_IntType:     return UnownedStringSlice("int32_t");
        case kIROp_UIntType:    return UnownedStringSlice("uint32_t");

        // Not clear just yet how we should handle half... we want all processing as float probly, but when reading/writing to memory converting
        case kIROp_HalfType:    return UnownedStringSlice("half");

        default:                return Super::getDefaultBuiltinTypeName(op);
    }
}

namespace { // anonymous

struct OrderType
{
    OrderType(const CPPSourceEmitter::HLSLType& hlslType):
        order(hlslType.getOrder()),
        type(hlslType)
    {
    }

    OrderType() {}

    SLANG_FORCE_INLINE bool operator<(const OrderType& rhs) const { return order < rhs.order; }
    SLANG_FORCE_INLINE bool operator==(const OrderType& rhs) const { return order == rhs.order; }
    SLANG_FORCE_INLINE bool operator!=(const OrderType& rhs) const { return order == rhs.order; }

    uint32_t order;
    CPPSourceEmitter::HLSLType type;
};


} // anonymous

void CPPSourceEmitter::emitPreprocessorDirectivesImpl()
{
    // Can emit built in type definitions here
    // Can emit intrinsics here too

    List<OrderType> types;
    for (const auto keyValue : m_typeNameMap)
    {
        HLSLType type = keyValue.Key;
        if (type.op == kIROp_VectorType || type.op == kIROp_MatrixType)
        {
            types.add(type);
        }
    }

    // Sort 
    types.sort();

    // Dump out all of the types in order

    for (const auto& orderType : types)
    {
        const auto& type = orderType.type;

        switch (type.op)
        {
            case kIROp_VectorType:
            {
                HLSLType elemType = HLSLType::makeBasic(IROp(type.elementType));
                int count = type.sizeOrColCount;
                
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
                const auto rowCount = type.rowCount;
                const auto colCount = type.sizeOrColCount;

                HLSLType vecType = HLSLType::makeVec(IROp(type.elementType), colCount);

                UnownedStringSlice typeName = _getTypeName(type);
                UnownedStringSlice rowTypeName = _getTypeName(vecType);
                
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


StringSlicePool::Handle CPPSourceEmitter::_calcTypeName(const HLSLType& type)
{
    switch (IROp(type.op))
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
            return m_slicePool.add(getBuiltinTypeName(IROp(type.op)));
        }
        case kIROp_HalfType:    
        {
            return m_slicePool.add(getBuiltinTypeName(kIROp_FloatType));
        }
        case kIROp_VectorType:
        {
            auto vecSize = type.sizeOrColCount;
            const IROp elemType = IROp(type.elementType);

            StringBuilder builder;
            builder << "Vec";
            UnownedStringSlice postFix = _getCTypeVecPostFix(elemType);

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
            const IROp elemType = IROp(type.elementType);
            
            const auto rowCount = type.rowCount;
            const auto colCount = type.sizeOrColCount;
            
            // Make sure there is the vector name too
            _getTypeName(HLSLType::makeVec(elemType, colCount));
            
            StringBuilder builder;

            builder << "Mat";
            const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(elemType));
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

UnownedStringSlice CPPSourceEmitter::_getFuncName(const HLSLFunction& func)
{
    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_funcNameMap.TryGetValue(func, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    handle = _calcFuncName(func);
    m_funcNameMap.Add(func, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

StringSlicePool::Handle CPPSourceEmitter::_calcFuncName(const HLSLFunction& func)
{
    StringBuilder builder;
    builder << "HLSLIntrinsic::";
    builder << m_slicePool.getSlice(func.name);
    return m_slicePool.add(builder);
}

UnownedStringSlice CPPSourceEmitter::_getTypeName(const HLSLType& type)
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


void CPPSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    UnownedStringSlice name = _getTypeName(HLSLType::makeVec(elementType->op, int(elementCount)));
    m_writer->emit(name);
}

CPPSourceEmitter::HLSLType CPPSourceEmitter::_getHLSLType(IRType* type)
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
        {
            return HLSLType::makeBasic(type->op);
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            auto size = GetIntVal(vecType->getElementCount());
            return HLSLType::makeVec(vecType->getElementType()->op, int(size));
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);
            auto colCount = GetIntVal(matType->getColumnCount());
            auto rowCount = GetIntVal(matType->getRowCount());

            return HLSLType::makeMatrix(matType->getElementType()->op, int(rowCount), int(colCount));
        }
        default: return HLSLType::makeInvalid();
    }
}

void CPPSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    HLSLType hlslType = _getHLSLType(type);

    if (!hlslType.isInvalid())
    {
        auto slice = _getTypeName(hlslType);
        m_writer->emit(slice);
        return;
    }

    switch (type->op)
    {
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

CPPSourceEmitter::HLSLFunction CPPSourceEmitter::_getHLSLFunc(const UnownedStringSlice& name, IRCall* inst, int operandIndex, int operandCount)
{
    HLSLFunction func;
    func.argsCount = 0;
    func.name = StringSlicePool::kNullHandle;

    const int maxArgs = SLANG_COUNT_OF(func.args);
    func.argsCount = 0;
    func.returnType = HLSLType::makeBasic(kIROp_VoidType);

    IRType* retType = inst->getDataType();
    if (retType)
    {
        retType = retType->getCanonicalType();
    }
    func.returnType = _getHLSLType(retType);
    if (func.returnType.isInvalid())
    {
        return func;
    }

    if (operandCount - operandIndex > maxArgs)
    {
        return func;
    }

    for (auto i = operandIndex; i < operandCount; ++i)
    {
        IRInst* operand = inst->getOperand(i);
        IRType* type = operand->getDataType();
        if (type)
        {
            IRType* canonicalType = type->getCanonicalType();

            HLSLType& arg = func.args[func.argsCount];
            arg = _getHLSLType(canonicalType);
            if (arg.isInvalid())
            {
                return func;
            }
            func.argsCount++;
        }
    }

    // Get the name
    func.name = m_slicePool.add(name);
    return func;
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
        HLSLFunction hlslFunc = _getHLSLFunc(name, inst, int(operandIndex), int(operandCount));
        if (hlslFunc.name != StringSlicePool::kNullHandle)
        {
            // Work out what the hlsl name
            name = _getFuncName(hlslFunc);
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
    SLANG_UNUSED(inOuterPrec);


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
            return false;
    }
}

} // namespace Slang
