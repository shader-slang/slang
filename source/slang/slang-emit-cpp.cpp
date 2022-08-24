// slang-emit-cpp.cpp
#include "slang-emit-cpp.h"

#include "../core/slang-writer.h"
#include "../core/slang-token-reader.h"
#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include "slang-ir-clone.h"

#include "../compiler-core/slang-artifact-desc-util.h"

#include <assert.h>

/*
ABI
---

In terms of ABI we need to discuss the variety of variables/resources that need to be defined by the host for appropriate execution
of the output code. 

https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-syntax

Broadly we could categorize these as..

1) Varying entry point parameters (or 'varying')
2) Uniform entry point parameters
3) Uniform globals 
4) Thread shared (such as group shared) or ('thread shared')
5) Thread local ('static')

If we can invoke a bunch of threads as a single invocation we could effectively have the ThreadShared not part of the ABI, but something
that is say allocated on the stack before the threads are kicked off. If we kick of threads individually then we would need to pass this
in as part of ABI. NOTE that it isn't right in so far as memory barriers etc couldn't work, as each thread would run to completion, but
we aren't going to worry about barriers for now. 

On 1 - there could be potentially input and outputs (perhaps in out?). On CPU I guess that's fine. 

On 2 and 3 they are effectively the same, and so for now 2+3 will be referred to together as 'uniforms'.
They should be copied into a single structure that has a well known order. 

On 1 these are parameters that vary on an invocation. Thus a caller might call many times with same globals structure
and different varying entry point parameters.

On 5 - This would be a global that can be set and then accessed within the context of single thread

So in order of rate of change

1 : Probably change on every invocation (in the future such an invocation might be behind the API)
2 + 3 : Changes per group of 'threads' executed together
4 : Does not change between invocations 
5 : Could be placed on the stack, and so not necessarily part of the ABI

For now we are only going to implement something 'Compute shader'-like. Doing so makes the varying parameter always the same.

So for now we would need to pass in

ComputeVaryingInput - Fixed because we are doing compute shader
Uniform             - All the uniform data in a big blob, both from uniform entry point parameters, and uniform globals

When called we can have a structure that holds the thread local variables, and these two pointers.
*/

namespace Slang {

static const char s_xyzwNames[] = "xyzw";

static UnownedStringSlice _getTypePrefix(IROp op)
{
    switch (op)
    {
        case kIROp_BoolType:        return UnownedStringSlice::fromLiteral("Bool");
        case kIROp_IntType:         return UnownedStringSlice::fromLiteral("I32");
        case kIROp_UIntType:        return UnownedStringSlice::fromLiteral("U32");
        case kIROp_FloatType:       return UnownedStringSlice::fromLiteral("F32");
        case kIROp_Int64Type:       return UnownedStringSlice::fromLiteral("I64");
        case kIROp_UInt64Type:      return UnownedStringSlice::fromLiteral("U64");
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

static bool _isCppTarget(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::HostCPPSource:
        return true;
    default:
        return false;
    }
}

static bool _isCppOrCudaTarget(CodeGenTarget target)
{
    switch (target)
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::CUDASource:
        return true;
    default:
        return false;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!! CPPEmitHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

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
        case kIROp_CharType:    return UnownedStringSlice("char");

        default:                return UnownedStringSlice();
    }
}

void CPPSourceEmitter::emitTypeDefinition(IRType* inType)
{
    if (_isCppTarget(m_target))
    {
        // All types are templates in C++
        return;
    }

    IRType* type = m_typeSet.getType(inType);
    if (!m_typeSet.isOwned(type))
    {
        // If defined in a different module, we assume they are emitted already. (Assumed to
        // be a nominal type)
        return;
    }

    SourceWriter* writer = getSourceWriter();

    switch (type->getOp())
    {
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);

            const UnownedStringSlice* elemNames = getVectorElementNames(vecType);

            int count = int(getIntVal(vecType->getElementCount()));

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
                writer->emit(elemNames[i]);
            }
            writer->emit(";\n");

            writer->dedent();
            writer->emit("};\n\n");
            break;
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            const auto rowCount = int(getIntVal(matType->getRowCount()));
            const auto colCount = int(getIntVal(matType->getColumnCount()));

            IRType* vecType = m_typeSet.addVectorType(matType->getElementType(), colCount);
            
            UnownedStringSlice typeName = _getTypeName(type);
            UnownedStringSlice rowTypeName = _getTypeName(vecType);

            writer->emit("template<>\n");
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
            if (IRBasicType::isaImpl(type->getOp()))
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
    IRType* type = m_typeSet.getType(inType);

    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_typeNameMap.TryGetValue(type, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    StringBuilder builder;
    if (SLANG_SUCCEEDED(calcTypeName(type, m_target, builder)))
    {
        handle = m_slicePool.add(builder);
    }

    m_typeNameMap.Add(type, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

SlangResult CPPSourceEmitter::_calcCPPTextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName)
{
    switch (texType->getAccess())
    {
        case SLANG_RESOURCE_ACCESS_READ:
            break;
        case SLANG_RESOURCE_ACCESS_READ_WRITE:
            outName << "RW";
            break;
        case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
            outName << "RasterizerOrdered";
            break;
        case SLANG_RESOURCE_ACCESS_APPEND:
            outName << "Append";
            break;
        case SLANG_RESOURCE_ACCESS_CONSUME:
            outName << "Consume";
            break;
        case SLANG_RESOURCE_ACCESS_WRITE:
            if (texType->isFeedback())
            {
                outName << "Feedback";
            }
            break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource access mode");
            return SLANG_FAIL;
    }

    switch (texType->GetBaseShape())
    {
        case TextureFlavor::Shape::Shape1D:		outName << "Texture1D";		break;
        case TextureFlavor::Shape::Shape2D:		outName << "Texture2D";		break;
        case TextureFlavor::Shape::Shape3D:		outName << "Texture3D";		break;
        case TextureFlavor::Shape::ShapeCube:	outName << "TextureCube";	break;
        case TextureFlavor::Shape::ShapeBuffer: outName << "Buffer";         break;
        default:
            SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled resource shape");
            return SLANG_FAIL;
    }

    if (texType->isMultisample())
    {
        outName << "MS";
    }
    if (texType->isArray())
    {
        outName << "Array";
    }
    outName << "<" << _getTypeName(texType->getElementType()) << " >";

    return SLANG_OK;
}

static UnownedStringSlice _getResourceTypePrefix(IROp op)
{
    switch (op)
    {
        case kIROp_HLSLStructuredBufferType:            return UnownedStringSlice::fromLiteral("StructuredBuffer");
        case kIROp_HLSLRWStructuredBufferType:          return UnownedStringSlice::fromLiteral("RWStructuredBuffer");
        case kIROp_HLSLRWByteAddressBufferType:         return UnownedStringSlice::fromLiteral("RWByteAddressBuffer");
        case kIROp_HLSLByteAddressBufferType:           return UnownedStringSlice::fromLiteral("ByteAddressBuffer");
        case kIROp_SamplerStateType:                    return UnownedStringSlice::fromLiteral("SamplerState");
        case kIROp_SamplerComparisonStateType:                  return UnownedStringSlice::fromLiteral("SamplerComparisonState");
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:   return UnownedStringSlice::fromLiteral("RasterizerOrderedStructuredBuffer"); 
        case kIROp_HLSLAppendStructuredBufferType:              return UnownedStringSlice::fromLiteral("AppendStructuredBuffer");            
        case kIROp_HLSLConsumeStructuredBufferType:             return UnownedStringSlice::fromLiteral("ConsumeStructuredBuffer");           
        case kIROp_HLSLRasterizerOrderedByteAddressBufferType:  return UnownedStringSlice::fromLiteral("RasterizerOrderedByteAddressBuffer");
        case kIROp_RaytracingAccelerationStructureType:         return UnownedStringSlice::fromLiteral("RaytracingAccelerationStructure");   

        default:                                        return UnownedStringSlice();
    }
}

SlangResult CPPSourceEmitter::calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out)
{
    switch (type->getOp())
    {
        case kIROp_HalfType:
        {
            // Special case half
           out << getBuiltinTypeName(kIROp_FloatType);
           return SLANG_OK;
        }
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);
            auto vecCount = int(getIntVal(vecType->getElementCount()));
            auto elemType = vecType->getElementType();

            if (_isCppOrCudaTarget(target))
            {
                out << "Vector<" << _getTypeName(elemType) << ", " << vecCount << ">";
            }
            else
            {             
                out << "Vec";
                UnownedStringSlice postFix = _getCTypeVecPostFix(elemType->getOp());

                out << postFix;
                if (postFix.getLength() > 1)
                {
                    out << "_";
                }
                out << vecCount;
            }
            return SLANG_OK; 
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            auto elementType = matType->getElementType();
            const auto rowCount = int(getIntVal(matType->getRowCount()));
            const auto colCount = int(getIntVal(matType->getColumnCount()));

            if (_isCppOrCudaTarget(target))
            {
                out << "Matrix<" << _getTypeName(elementType) << ", " << rowCount << ", " << colCount << ">";
            }
            else
            {
                out << "Mat";
                const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(elementType->getOp()));
                out  << postFix;
                if (postFix.getLength() > 1)
                {
                    out << "_";
                }
                out << rowCount;
                out << colCount;
            }
            return SLANG_OK;
        }
        case kIROp_WitnessTableType:
        case kIROp_WitnessTableIDType:
        {
            // A witness table typed value translates to a pointer to the
            // struct of function pointers corresponding to the interface type.
            auto witnessTableType = static_cast<IRWitnessTableType*>(type);
            auto baseType = cast<IRType>(witnessTableType->getOperand(0));
            SLANG_RETURN_ON_FAIL(calcTypeName(baseType, target, out));
            out << "*";
            return SLANG_OK;
        }
        case kIROp_RawPointerType:
        case kIROp_RTTIPointerType:
        {
            out << "void*";
            return SLANG_OK;
        }
        case kIROp_AnyValueType:
        {
            out << "AnyValue<";
            auto anyValueType = static_cast<IRAnyValueType*>(type);
            out << getIntVal(anyValueType->getSize());
            out << ">";
            return SLANG_OK;
        }
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
        {
            auto groupType = cast<IRParameterGroupType>(type);
            auto elementType = groupType->getElementType();

            SLANG_RETURN_ON_FAIL(calcTypeName(elementType, target, out));
            out << "*";
            return SLANG_OK;
        }
        case kIROp_NativePtrType:
        case kIROp_PtrType:
        {
            auto elementType = (IRType*)type->getOperand(0);
            SLANG_RETURN_ON_FAIL(calcTypeName(elementType, target, out));
            out << "*";
            return SLANG_OK;
        }
        case kIROp_RTTIType:
        {
            out << "TypeInfo";
            return SLANG_OK;
        }
        case kIROp_RTTIHandleType:
        {
            out << "TypeInfo*";
            return SLANG_OK;
        }
        case kIROp_NativeStringType:
        {
            out << "const char*";
            return SLANG_OK;
        }
        case kIROp_StringType:
        {
            out << "String";
            return SLANG_OK;
        }
        case kIROp_ComPtrType:
        {
            auto comPtrType = static_cast<IRComPtrType*>(type);
            auto baseType = cast<IRType>(comPtrType->getOperand(0));

            out << "ComPtr<";
            SLANG_RETURN_ON_FAIL(calcTypeName(baseType, target, out));
            out << ">";
            return SLANG_OK;
        }
        case kIROp_ClassType:
        {
            out << "RefPtr<";
            out << getName(type);
            out << ">";
            return SLANG_OK;
        }
        default:
        {
            if (isNominalOp(type->getOp()))
            {
                out << getName(type);
                return SLANG_OK;
            }

            if (IRBasicType::isaImpl(type->getOp()))
            {
                out << getBuiltinTypeName(type->getOp());
                return SLANG_OK;
            }

            if (auto texType = as<IRTextureTypeBase>(type))
            {
                // We don't support TextureSampler, so ignore that
                if (texType->getOp() != kIROp_TextureSamplerType)
                {
                    return _calcCPPTextureTypeName(texType, out);   
                }
            }

            // If _getResourceTypePrefix returns something, we assume can output any specialization after it in order.
            {
                UnownedStringSlice prefix = _getResourceTypePrefix(type->getOp());
                if (prefix.getLength() > 0)
                {
                    auto oldWriter = m_writer;
                    SourceManager* sourceManager = oldWriter->getSourceManager();

                    // TODO(JS): This is a bit of a hack. We don't want to emit the result here,
                    // so we replace the writer, write out the type, grab the contents, and restore the writer

                    SourceWriter writer(sourceManager, LineDirectiveMode::None);
                    m_writer = &writer;

                    m_writer->emit(prefix);

                    // TODO(JS).
                    // Assumes ordering of types matches ordering of operands.

                    UInt operandCount = type->getOperandCount();
                    if (operandCount)
                    {
                        m_writer->emit("<");
                        for (UInt ii = 0; ii < operandCount; ++ii)
                        {
                            if (ii != 0)
                            {
                                m_writer->emit(", ");
                            }
                            emitVal(type->getOperand(ii), getInfo(EmitOp::General));
                        }
                        m_writer->emit(">");
                    }

                    out << writer.getContent();

                    m_writer = oldWriter;
                    return SLANG_OK;
                }
            }

            break;
        }
    }

    SLANG_DIAGNOSE_UNEXPECTED(getSink(), SourceLoc(), "unhandled type for C/C++ emit");
    return SLANG_FAIL;
}

void CPPSourceEmitter::useType(IRType* type)
{
    _getTypeName(type);
}

static IRBasicType* _getElementType(IRType* type)
{
    switch (type->getOp())
    {
        case kIROp_VectorType:      type = static_cast<IRVectorType*>(type)->getElementType(); break;
        case kIROp_MatrixType:      type = static_cast<IRMatrixType*>(type)->getElementType(); break;
        default:                    break;
    }
    return dynamicCast<IRBasicType>(type);
}

/* static */CPPSourceEmitter::TypeDimension CPPSourceEmitter::_getTypeDimension(IRType* type, bool vecSwap)
{
    switch (type->getOp())
    {
        case kIROp_PtrType:
        {
            type = static_cast<IRPtrType*>(type)->getValueType();
            break;
        }
        case kIROp_RefType:
        {
            type = static_cast<IRRefType*>(type)->getValueType();
            break;
        }
        default: break;
    }

    switch (type->getOp())
    {
        case kIROp_VectorType:
        {
            auto vecType = static_cast<IRVectorType*>(type);

            IRBasicType* elemBasicType = as<IRBasicType>(vecType->getElementType());
            const BaseType baseType = elemBasicType->getBaseType();

            const int elemCount = int(getIntVal(vecType->getElementCount()));
            return (!vecSwap) ? TypeDimension{baseType, 1, elemCount} : TypeDimension{ baseType, elemCount, 1};
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);
            const int colCount = int(getIntVal(matType->getColumnCount()));
            const int rowCount = int(getIntVal(matType->getRowCount()));

            IRBasicType* elemBasicType = as<IRBasicType>(matType->getElementType());
            const BaseType baseType = elemBasicType->getBaseType();

            return TypeDimension{baseType, rowCount, colCount};
        }
        default:
        {
            // Assume we don't know the type
            BaseType baseType = BaseType::Void;

            IRBasicType* basicType = as<IRBasicType>(type);
            if (basicType)
            {
                baseType = basicType->getBaseType();
            }

            return TypeDimension{baseType, 1, 1};
        }
    }
}

void CPPSourceEmitter::_emitAccess(const UnownedStringSlice& name, const TypeDimension& dimension, int row, int col, SourceWriter* writer)
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
        {
            // Vector, row count is biggest
            const UnownedStringSlice* elemNames = getVectorElementNames(dimension.elemType, dimension.rowCount);
            writer->emit(".");
            const int index = (row > col) ? row : col;
            writer->emit(elemNames[index]);
            break;
        }
        case 2:
        {
            // Vector cols biggest dimension
            const UnownedStringSlice* elemNames = getVectorElementNames(dimension.elemType, dimension.colCount);
            writer->emit(".");
            const int index = (row > col) ? row : col;
            writer->emit(elemNames[index]);
            break;
        }
        case 3:
        {            
            // Matrix
            const UnownedStringSlice* elemNames = getVectorElementNames(dimension.elemType, dimension.colCount);

            writer->emit(".rows[");
            writer->emit(row);
            writer->emit("].");
            writer->emit(elemNames[col]);
            break;
        }
    }
}

static bool _isOperator(const UnownedStringSlice& funcName)
{
    if (funcName.getLength() > 0)
    {
        const char c = funcName[0];
        return !((c >= 'a' && c <='z') || (c >= 'A' && c <= 'Z') || c == '_');
    }
    return false;
}

void CPPSourceEmitter::_emitAryDefinition(const HLSLIntrinsic* specOp)
{
    auto info = HLSLIntrinsic::getInfo(specOp->op);
    auto funcName = info.funcName;
    SLANG_ASSERT(funcName.getLength() > 0);

    const bool isOperator = _isOperator(funcName);

    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp->signatureType;
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

    IRType* retType = specOp->returnType;

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
        scalarFuncName = _getScalarFuncName(specOp->op, _getElementType(funcType->getParamType(0)));
        _emitSignature(funcName, specOp);
    }
    
    writer->emit("\n{\n");
    writer->indent();

    const bool hasReturnType = retType->getOp() != kIROp_VoidType;

    TypeDimension calcDim;
    if (hasReturnType)
    {
        emitType(retType);
        writer->emit(" r;\n");

        calcDim = _getTypeDimension(retType, false);
    }
    else
    {
        calcDim = _getTypeDimension(funcType->getParamType(0), false);        
    }

    for (int i = 0; i < calcDim.rowCount; ++i)
    {
        for (int j = 0; j < calcDim.colCount; ++j)
        {
            if (hasReturnType)
            {
                _emitAccess(UnownedStringSlice::fromLiteral("r"), calcDim, i, j, writer);
                writer->emit(" = ");
            }

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

    if (hasReturnType)
    {
        writer->emit("return r;\n");
    }

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitAnyAllDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 1);
    IRType* paramType0 = funcType->getParamType(0);

    SourceWriter* writer = getSourceWriter();

    IRType* elementType = _getElementType(paramType0);
    SLANG_ASSERT(elementType);
    IRType* retType = specOp->returnType;
    auto retTypeName = _getTypeName(retType);

    IROp style = _getTypeStyle(elementType->getOp());

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
                if (specOp->op == HLSLIntrinsic::Op::All)
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

void CPPSourceEmitter::_emitSignature(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    IRFuncType* funcType = specOp->signatureType;
    const int paramsCount = int(funcType->getParamCount());
    IRType* retType = specOp->returnType;

    emitFunctionPreambleImpl(nullptr);

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

UnownedStringSlice CPPSourceEmitter::_getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op op, IRType*const* argTypes, Int argCount, IRType* retType)
{
    HLSLIntrinsic intrinsic;
    m_intrinsicSet.calcIntrinsic(op, retType, argTypes, argCount, intrinsic);
    auto specOp = m_intrinsicSet.add(intrinsic);
    _maybeEmitSpecializedOperationDefinition(specOp);
    return  _getFuncName(specOp);
}

void CPPSourceEmitter::_emitGetAtDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(0);

    for (Index i = 0; i < 3; ++i)
    {
        UnownedStringSlice typePrefix = (i == 0) ? UnownedStringSlice::fromLiteral("const ") : UnownedStringSlice();
        bool lValue = (i != 2);

        emitFunctionPreambleImpl(nullptr);

        writer->emit(typePrefix);
        emitType(specOp->returnType);
        if (lValue)
            m_writer->emit("*");
        writer->emit(" ");
        writer->emit(funcName);
        writer->emit("(");

        writer->emit(typePrefix);
        emitType(funcType->getParamType(0));
        if (lValue)
            writer->emit("*");
        writer->emit(" a,  ");
        emitType(funcType->getParamType(1));
        writer->emit(" b)\n{\n");

        writer->indent();

        if (auto vectorType = as<IRVectorType>(srcType))
        {
            int vecSize = int(getIntVal(vectorType->getElementCount()));

            writer->emit("SLANG_PRELUDE_ASSERT(b >= 0 && b < ");
            writer->emit(vecSize);
            writer->emit(");\n");

            writer->emit("return ((");
            emitType(specOp->returnType);
            writer->emit("*)");

            if (lValue)
                writer->emit("a) + b;\n");
            else
                writer->emit("&a)[b];\n");
        }
        else if (auto matrixType = as<IRMatrixType>(srcType))
        {
            //int colCount = int(getIntVal(matrixType->getColumnCount()));
            int rowCount = int(getIntVal(matrixType->getRowCount()));

            writer->emit("SLANG_PRELUDE_ASSERT(b >= 0 && b < ");
            writer->emit(rowCount);
            writer->emit(");\n");

            if (lValue)
                writer->emit("return &(a->rows[b]);\n");
            else
                writer->emit("return a.rows[b];\n");
        }

        writer->dedent();
        writer->emit("}\n\n");
    }
}

void CPPSourceEmitter::_emitConstructConvertDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();
    IRFuncType* funcType = specOp->signatureType;

    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(1);
    IRType* retType = specOp->returnType;

    emitFunctionPreambleImpl(nullptr);

    emitType(retType);
    writer->emit(" ");
    writer->emit(funcName);
    writer->emit("(");
    emitType(srcType);
    writer->emitChar(' ');
    writer->emitChar(char('a' + 0));
    writer->emit(")");

    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");
    emitType(retType);
    writer->emit("{ ");

  
    IRType* dstElemType = _getElementType(retType);
    //IRType* srcElemType = _getElementType(srcType);

    TypeDimension dim = _getTypeDimension(retType, false);

    UnownedStringSlice rowTypeName;
    if (dim.rowCount > 1)
    {
        IRType* rowType = m_typeSet.addVectorType(dstElemType, int(dim.colCount));
        rowTypeName = _getTypeName(rowType);
    }

    for (int i = 0; i < dim.rowCount; ++i)
    {
        if (dim.rowCount > 1)
        {
            if (i > 0)
            {
                writer->emit(", \n");
            }

            if (m_target == CodeGenTarget::CUDASource)
            {
                m_writer->emit("make_");
                writer->emit(rowTypeName);
                m_writer->emit("(");
            }
            else
            {
                writer->emit(rowTypeName);
                writer->emit("{ ");
            }
        }

        for (int j = 0; j < dim.colCount; ++j)
        {
            if (j > 0)
            {
                writer->emit(", ");
            }

            emitType(dstElemType);
            writer->emit("(");
            _emitAccess(UnownedStringSlice::fromLiteral("a"), dim, i, j, writer);
            writer->emit(")");
        }
        if (dim.rowCount > 1)
        {
            if (m_target == CodeGenTarget::CUDASource)
            {
                writer->emit(")");
            }
            else
            {
                writer->emit("}");
            }
        }
    }

    writer->emit("};\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_emitInitDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();
    IRFuncType* funcType = specOp->signatureType;

    emitFunctionPreambleImpl(nullptr);

    IRType* retType = specOp->returnType;
    
    _emitSignature(funcName, specOp);
    writer->emit("\n{\n");
    writer->indent();

    // Use C++ construction
    writer->emit("return ");
    emitType(retType);
    writer->emit("{ ");

    const Index paramCount = Index(funcType->getParamCount());

    if (IRVectorType* vecType = as<IRVectorType>(retType))
    {
        Index elementCount = Index(getIntVal(vecType->getElementCount()));

        Index paramIndex = 0;
        Index paramSubIndex = 0;

        for (Index i = 0; i < elementCount; ++i)
        {
            if (i > 0)
            {
                writer->emit(", ");
            }

            if (paramIndex >= paramCount)
            {
                writer->emit("0");
            }
            else
            {
                IRType* paramType = funcType->getParamType(paramIndex);

                if (IRVectorType* paramVecType = as<IRVectorType>(paramType))
                {
                    Index paramElementCount = Index(getIntVal(paramVecType->getElementCount()));

                    const UnownedStringSlice* elemNames = getVectorElementNames(paramVecType);

                    writer->emitChar('a' + char(paramIndex));
                    writer->emit(".");
                    writer->emit(elemNames[paramSubIndex]);

                    paramSubIndex ++;

                    if (paramSubIndex >= paramElementCount)
                    {
                        paramIndex++;
                        paramSubIndex = 0;
                    }
                }
                else
                {
                    writer->emitChar('a' + char(paramIndex));
                    paramIndex++;
                }
            }
        }
    }
    else
    {
        for (Index i = 0; i < paramCount; ++i)
        {
            if (i > 0)
            {
                writer->emit(", ");
            }
            writer->emitChar('a' + char(i));
        }
    }

    writer->emit("};\n");

    writer->dedent();
    writer->emit("}\n\n");
}


void CPPSourceEmitter::_emitConstructFromScalarDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();
    IRFuncType* funcType = specOp->signatureType;

    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(1);
    IRType* retType = specOp->returnType;

    emitFunctionPreambleImpl(nullptr);

    emitType(retType);
    writer->emit(" ");
    writer->emit(funcName);
    writer->emit("(");
    emitType(srcType);
    writer->emitChar(' ');
    writer->emitChar(char('a' + 0));
    writer->emit(")");

    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");
    emitType(retType);
    writer->emit("{ ");

    const TypeDimension dim = _getTypeDimension(retType, false);

    for (int i = 0; i < dim.rowCount; ++i)
    {
        if (dim.rowCount > 1)
        {
            if (i > 0)
            {
                writer->emit(", \n");
            }
            writer->emit("{ ");
        }
        for (int j = 0; j < dim.colCount; ++j)
        {
            if (j > 0)
            {
                writer->emit(", ");
            }
            writer->emit("a");
        }
        if (dim.rowCount > 1)
        {
            writer->emit("}");
        }
    }

    writer->emit("};\n");

    writer->dedent();
    writer->emit("}\n\n");
}

void CPPSourceEmitter::_maybeEmitSpecializedOperationDefinition(const HLSLIntrinsic* specOp)
{
    // Check if it's been emitted already, if not add it.
    if (!m_intrinsicEmitted.Add(specOp))
    {
        return;
    }
    emitSpecializedOperationDefinition(specOp);
}

void CPPSourceEmitter::emitSpecializedOperationDefinition(const HLSLIntrinsic* specOp)
{
    typedef HLSLIntrinsic::Op Op;

    switch (specOp->op)
    {
        case Op::Init:
        {
            return _emitInitDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Any:
        case Op::All:
        {
            return _emitAnyAllDefinition(_getFuncName(specOp), specOp);
        }
        case Op::ConstructConvert:
        {
            return _emitConstructConvertDefinition(_getFuncName(specOp), specOp);
        }
        case Op::ConstructFromScalar:
        {
            return _emitConstructFromScalarDefinition(_getFuncName(specOp), specOp);
        }
        case Op::GetAt:
        {
            return _emitGetAtDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Swizzle:
        {
            // Don't have to output anything for swizzle for now
            return; 
        }
        default:
        {
            const auto& info = HLSLIntrinsic::getInfo(specOp->op);
            const int paramCount = (info.numOperands < 0) ? int(specOp->signatureType->getParamCount()) : info.numOperands;

            if (paramCount >= 1 && paramCount <= 3)
            {
                return _emitAryDefinition(specOp);
            }
            break;
        }
    }

    SLANG_ASSERT(!"Unhandled");
}

void CPPSourceEmitter::emitCall(const HLSLIntrinsic* specOp, IRInst* inst, const IRUse* operands, int numOperands, const EmitOpInfo& inOuterPrec)
{
    typedef HLSLIntrinsic::Op Op;

    SLANG_UNUSED(inOuterPrec);
    SourceWriter* writer = getSourceWriter();
    
    switch (specOp->op)
    {
        case Op::Init:
        {
            IRType* retType = specOp->returnType;
            if (IRBasicType::isaImpl(retType->getOp()))
            {
                SLANG_ASSERT(numOperands == 1);
                        
                writer->emit(_getTypeName(retType));
                writer->emitChar('(');

                emitOperand(operands[0].get(), getInfo(EmitOp::General));

                writer->emitChar(')');
                return;
            }
            break;
        }
        case Op::Swizzle:
        {
            // Currently only works for C++ (we use {} constuction) - which means we don't need to generate a function.
            // For C we need to generate suitable construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);
            const Index elementCount = Index(swizzleInst->getElementCount());

            IRType* srcType = swizzleInst->getBase()->getDataType();
            IRVectorType* srcVecType = as<IRVectorType>(srcType);

            const UnownedStringSlice* elemNames = getVectorElementNames(srcVecType);

            // TODO(JS): Not 100% sure this is correct on the parens handling front
            IRType* retType = specOp->returnType;
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
                SLANG_RELEASE_ASSERT(irElementIndex->getOp() == kIROp_IntLit);
                IRConstant* irConst = (IRConstant*)irElementIndex;
                UInt elementIndex = (UInt)irConst->value.intVal;
                SLANG_RELEASE_ASSERT(elementIndex < 4);

                writer->emit(elemNames[elementIndex]);
            }

            writer->emit("}");
            return;
        }
        default: break;
    }
    
    {
        const auto& info = HLSLIntrinsic::getInfo(specOp->op);
        // Make sure that the return type is available
        const bool isOperator = _isOperator(info.funcName);
        const UnownedStringSlice funcName = _getFuncName(specOp);

        switch (specOp->op)
        {
            case Op::ConstructFromScalar:
            {
                // We need to special case, because this may have come from a swizzle from a built in
                // type, in that case the only parameter we want is the first one 
                numOperands = 1;
                break;
            }

            default: break;
        }

        // add that we want a function
        SLANG_ASSERT(info.numOperands < 0 || numOperands == info.numOperands);

        useType(specOp->returnType);
            
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
    }
}

HLSLIntrinsic* CPPSourceEmitter::_addIntrinsic(HLSLIntrinsic::Op op, IRType* returnType, IRType*const* argTypes, Index argTypeCount)
{
    HLSLIntrinsic intrinsic;
    m_intrinsicSet.calcIntrinsic(op, returnType, argTypes, argTypeCount, intrinsic);
    HLSLIntrinsic* addedIntrinsic = m_intrinsicSet.add(intrinsic);
    _getFuncName(addedIntrinsic);
    return addedIntrinsic;
}

SlangResult CPPSourceEmitter::calcScalarFuncName(HLSLIntrinsic::Op op, IRBasicType* type, StringBuilder& outBuilder)
{
    outBuilder << _getTypePrefix(type->getOp()) << "_" << HLSLIntrinsic::getInfo(op).funcName;
    return SLANG_OK;   
}

UnownedStringSlice CPPSourceEmitter::_getScalarFuncName(HLSLIntrinsic::Op op, IRBasicType* type)
{
    /* TODO(JS): This is kind of fast and loose. That we don't know all the parameters that are taken or
    what the return type is, so we can't add to the HLSLIntrinsic map - we just generate the scalar
    function name and use it (whilst also adding to the slice pool, so that we can return an
    unowned slice). */

    StringBuilder builder;
    if (SLANG_FAILED(calcScalarFuncName(op, type, builder)))
    {
        SLANG_ASSERT(!"Unable to create scalar function name");
        return UnownedStringSlice();
    }

    // Add to the pool. 
    auto handle = m_slicePool.add(builder);
    return m_slicePool.getSlice(handle);
}

UnownedStringSlice CPPSourceEmitter::_getFuncName(const HLSLIntrinsic* specOp)
{
    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_intrinsicNameMap.TryGetValue(specOp, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    StringBuilder builder;
    if (SLANG_FAILED(calcFuncName(specOp, builder)))
    {
        SLANG_ASSERT(!"Unable to create function name");
        // Return an empty slice, as an error...
        return UnownedStringSlice();
    }

    handle = m_slicePool.add(builder);
    m_intrinsicNameMap.Add(specOp, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

SlangResult CPPSourceEmitter::calcFuncName(const HLSLIntrinsic* specOp, StringBuilder& outBuilder)
{
    typedef HLSLIntrinsic::Op Op;

    if (specOp->isScalar())
    {
        IRType* paramType = specOp->signatureType->getParamType(0);
        IRBasicType* basicType = as<IRBasicType>(paramType);
        if (basicType)
        {
            return calcScalarFuncName(specOp->op, basicType, outBuilder);
        }
        else
        {
            outBuilder << getName(paramType) << HLSLIntrinsic::getInfo(specOp->op).name;
            return SLANG_OK;
        }
    }
    else
    {
        switch (specOp->op)
        {
            case Op::ConstructConvert:
            {
                // Work out the function name
                IRFuncType* signatureType = specOp->signatureType;
                SLANG_ASSERT(signatureType->getParamCount() == 2);

                IRType* dstType = signatureType->getParamType(0);
                //IRType* srcType = signatureType->getParamType(1);

                outBuilder << "convert_";
                // I need a function that is called that will construct this
                SLANG_RETURN_ON_FAIL(calcTypeName(dstType, CodeGenTarget::CSource, outBuilder));
                return SLANG_OK;
            }
            case Op::ConstructFromScalar:
            {
                // Work out the function name
                IRFuncType* signatureType = specOp->signatureType;
                SLANG_ASSERT(signatureType->getParamCount() == 2);

                IRType* dstType = signatureType->getParamType(0);
                
                outBuilder << "constructFromScalar_";
                // I need a function that is called that will construct this
                SLANG_RETURN_ON_FAIL(calcTypeName(dstType, CodeGenTarget::CSource, outBuilder));
                return SLANG_OK;
            }
            case Op::GetAt:
            {
                outBuilder << "getAt";
                return SLANG_OK;
            }
            case Op::Init:
            {
                outBuilder << "make_";
                SLANG_RETURN_ON_FAIL(calcTypeName(specOp->returnType, CodeGenTarget::CSource, outBuilder));
                return SLANG_OK;
            }
            default: break;
        }

        const auto& info = HLSLIntrinsic::getInfo(specOp->op);
        if (info.funcName.getLength())
        {
            if (!_isOperator(info.funcName))
            {
                // If there is a standard default name, just use that
                outBuilder << info.funcName;
                return SLANG_OK;
            }
        }

        // Just use the name of the Op. This is probably wrong, but gives a pretty good idea of what the desired (presumably missing) op is.
        outBuilder << info.name;
        return SLANG_OK;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!! CPPSourceEmitter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

CPPSourceEmitter::CPPSourceEmitter(const Desc& desc):
    Super(desc),
    m_slicePool(StringSlicePool::Style::Default),
    m_typeSet(desc.codeGenContext->getSession()),
    m_opLookup(new HLSLIntrinsicOpLookup),
    m_intrinsicSet(&m_typeSet, m_opLookup)
{
    m_semanticUsedFlags = 0;
    //m_semanticUsedFlags = SemanticUsedFlag::GroupID | SemanticUsedFlag::GroupThreadID | SemanticUsedFlag::DispatchThreadID;

    
    const auto artifactDesc = ArtifactDescUtil::makeDescForCompileTarget(asExternal(getTarget()));

    // If we have runtime library we can convert to a terminated string slice    
    m_hasString = (artifactDesc.style == ArtifactStyle::Host);
}

void CPPSourceEmitter::emitParamTypeImpl(IRType* type, String const& name)
{
    emitType(type, name);
}

void CPPSourceEmitter::emitGlobalRTTISymbolPrefix()
{
    m_writer->emit("SLANG_PRELUDE_SHARED_LIB_EXPORT");
}

void CPPSourceEmitter::emitWitnessTable(IRWitnessTable* witnessTable)
{
    auto interfaceType = cast<IRInterfaceType>(witnessTable->getConformanceType());

    // Ignore witness tables for builtin interface types.
    if (isBuiltin(interfaceType))
        return;

    if (interfaceType->findDecoration<IRComInterfaceDecoration>())
    {
        pendingWitnessTableDefinitions.add(witnessTable);
        return;
    }

    // Declare a global variable for the witness table.
    m_writer->emit("extern \"C\" { ");
    emitGlobalRTTISymbolPrefix();
    m_writer->emit(" extern ");
    emitSimpleType(interfaceType);
    m_writer->emit(" ");
    m_writer->emit(getName(witnessTable));
    m_writer->emit("; }\n");

    // The actual definition of this witness table global variable
    // is deferred until the entire `Context` class is emitted, so
    // that the member functions are available for reference.
    // The witness table definition emission logic is defined in the
    // `_emitWitnessTableDefinitions` function.
    pendingWitnessTableDefinitions.add(witnessTable);
}

void CPPSourceEmitter::_emitWitnessTableDefinitions()
{
    for (auto witnessTable : pendingWitnessTableDefinitions)
    {
        auto interfaceType = cast<IRInterfaceType>(witnessTable->getConformanceType());
        if (interfaceType->findDecoration<IRComInterfaceDecoration>())
        {
            emitComWitnessTable(witnessTable);
            continue;
        }
        List<IRWitnessTableEntry*> sortedWitnessTableEntries = getSortedWitnessTableEntries(witnessTable);
        m_writer->emit("extern \"C\"\n{\n");
        m_writer->indent();
        emitGlobalRTTISymbolPrefix();
        m_writer->emit("\n");
        emitSimpleType(interfaceType);
        m_writer->emit(" ");
        m_writer->emit(getName(witnessTable));
        m_writer->emit(" = {\n");
        m_writer->indent();
        auto seqIdDecoration = witnessTable->findDecoration<IRSequentialIDDecoration>();
        if (seqIdDecoration)
            m_writer->emit((UInt)seqIdDecoration->getSequentialID());
        else
            m_writer->emit("0");
        for (Index i = 0; i < sortedWitnessTableEntries.getCount(); i++)
        {
            auto entry = sortedWitnessTableEntries[i];
            if (auto funcVal = as<IRFunc>(entry->satisfyingVal.get()))
            {
                m_writer->emit(",\n");
                m_writer->emit(getName(funcVal));
            }
            else if (auto witnessTableVal = as<IRWitnessTable>(entry->getSatisfyingVal()))
            {
                m_writer->emit(",\n");
                m_writer->emit("&");
                m_writer->emit(getName(witnessTableVal));
            }
            else if (entry->getSatisfyingVal() &&
                     entry->getSatisfyingVal()->getDataType()->getOp() == kIROp_RTTIHandleType)
            {
                m_writer->emit(",\n");
                emitInstExpr(entry->getSatisfyingVal(), getInfo(EmitOp::General));
            }
            else
            {
                SLANG_UNEXPECTED("unknown witnesstable entry type");
            }
        }
        m_writer->dedent();
        m_writer->emit("\n};\n");
        m_writer->dedent();
        m_writer->emit("\n}\n");
    }
}

void CPPSourceEmitter::emitComInterface(IRInterfaceType* interfaceType)
{
    auto comDecoration = interfaceType->findDecoration<IRComInterfaceDecoration>();
    auto guidInst = as<IRStringLit>(comDecoration->getOperand(0));
    SLANG_RELEASE_ASSERT(guidInst);
    auto guid = guidInst->getStringSlice();
    SLANG_RELEASE_ASSERT(guid.getLength() == 32);

    m_writer->emit("struct ");
    emitSimpleType(interfaceType);
    m_writer->emit(" : ");
    // Emit base types.
    bool isFirst = true;
    for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
        if (auto witnessTableType = as<IRWitnessTableTypeBase>(entry->getRequirementVal()))
        {
            if (isFirst)
            {
                isFirst = false;
            }
            else
            {
                m_writer->emit(", ");
            }
            emitType((IRType*)witnessTableType->getConformanceType());
        }
    }
    if (isFirst)
    {
        m_writer->emit("ISlangUnknown");
    }

    // Emit methods.
    m_writer->emit("\n{\n");
    m_writer->indent();
    // Emit GUID.
    m_writer->emit("SLANG_COM_INTERFACE(0x");
    m_writer->emit(guid.subString(0, 8));
    m_writer->emit(", 0x");
    m_writer->emit(guid.subString(8, 4));
    m_writer->emit(", 0x");
    m_writer->emit(guid.subString(12, 4));
    m_writer->emit(", { ");
    for (UInt i = 0; i < 8; i++)
    {
        if (i > 0)
            m_writer->emit(", ");
        m_writer->emit("0x");
        m_writer->emit(guid.subString(16 + i * 2, 2));
    }
    m_writer->emit(" })\n");

    for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
        if (auto funcVal = as<IRFuncType>(entry->getRequirementVal()))
        {
            m_writer->emit("virtual SLANG_NO_THROW ");
            emitType(funcVal->getResultType());
            m_writer->emit(" SLANG_MCALL ");
            m_writer->emit(getName(entry->getRequirementKey()));
            m_writer->emit("(");
            bool isFirstParam = true;
            for (UInt p = 1; p < funcVal->getParamCount(); p++)
            {
                auto paramType = funcVal->getParamType(p);
                if (!isFirstParam)
                    m_writer->emit(", ");
                else
                    isFirstParam = false;

                emitParamType(paramType, String("param") + String(p));
            }
            m_writer->emit(") = 0;\n");
        }
    }
    m_writer->dedent();
    m_writer->emit("};\n");
}

void CPPSourceEmitter::emitInterface(IRInterfaceType* interfaceType)
{
    // Skip built-in interfaces.
    if (isBuiltin(interfaceType))
        return;

    if (interfaceType->findDecoration<IRComInterfaceDecoration>())
    {
        emitComInterface(interfaceType);
        return;
    }

    m_writer->emit("struct ");
    emitSimpleType(interfaceType);
    m_writer->emit("\n{\n");
    m_writer->indent();
    m_writer->emit("uint32_t sequentialID;\n");
    for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
    {
        auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
        if (auto funcVal = as<IRFuncType>(entry->getRequirementVal()))
        {
            emitType(funcVal->getResultType());
            m_writer->emit(" (*");
            m_writer->emit(getName(entry->getRequirementKey()));
            m_writer->emit(")");
            m_writer->emit("(");
            bool isFirstParam = true;
            for (UInt p = 0; p < funcVal->getParamCount(); p++)
            {
                auto paramType = funcVal->getParamType(p);
                // Ingore TypeType-typed parameters for now.
                if (as<IRTypeType>(paramType))
                    continue;

                if (!isFirstParam)
                    m_writer->emit(", ");
                else
                    isFirstParam = false;

                emitParamType(paramType, String("param") + String(p));
            }
            m_writer->emit(");\n");
        }
        else if (auto witnessTableType = as<IRWitnessTableType>(entry->getRequirementVal()))
        {
            emitType((IRType*)witnessTableType->getConformanceType());
            m_writer->emit("* ");
            m_writer->emit(getName(entry->getRequirementKey()));
            m_writer->emit(";\n");
        }
        else if (entry->getRequirementVal()->getOp() == kIROp_RTTIHandleType)
        {
            m_writer->emit("TypeInfo* ");
            m_writer->emit(getName(entry->getRequirementKey()));
            m_writer->emit(";\n");
        }
    }
    m_writer->dedent();
    m_writer->emit("};\n");
}

void CPPSourceEmitter::emitRTTIObject(IRRTTIObject* rttiObject)
{
    m_writer->emit("extern \"C\" { ");
    emitGlobalRTTISymbolPrefix();
    m_writer->emit(" TypeInfo ");
    m_writer->emit(getName(rttiObject));
    m_writer->emit(" = {");
    auto typeSizeDecoration = rttiObject->findDecoration<IRRTTITypeSizeDecoration>();
    SLANG_ASSERT(typeSizeDecoration);
    m_writer->emit(typeSizeDecoration->getTypeSize());
    m_writer->emit("}; }\n");
}

bool CPPSourceEmitter::tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType)
{
    SLANG_UNUSED(varDecl);
    SLANG_UNUSED(varType);

    switch (varType->getOp())
    {
        case kIROp_StructType:
        {
            String name = getName(varDecl);

            UnownedStringSlice typeName = _getTypeName(varType);
            m_writer->emit(typeName);
            m_writer->emit("* ");
            m_writer->emit(name);
            m_writer->emit(";\n");
            return true;
        }
    }

    return false;
}

void CPPSourceEmitter::emitParameterGroupImpl(IRGlobalParam* varDecl, IRUniformParameterGroupType* type)
{
    // Output global parameters
    auto varLayout = getVarLayout(varDecl);
    SLANG_RELEASE_ASSERT(varLayout);

    String name = getName(varDecl);
    auto elementType = type->getElementType();

    switch (type->getOp())
    {
        case kIROp_ParameterBlockType:
        case kIROp_ConstantBufferType:
        {
            UnownedStringSlice typeName = _getTypeName(elementType);
            m_writer->emit(typeName);
            m_writer->emit("* ");
            m_writer->emit(name);
            m_writer->emit(";\n");
            break;
        }
        default:
        {            
            emitType(elementType, name);
            m_writer->emit(";\n");
            break;
        }
    }
}

void CPPSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    SLANG_UNUSED(entryPointDecor);

    auto profile = m_effectiveProfile;    
    auto stage = profile.getStage();

    switch (stage)
    {
        case Stage::Compute:
        {
            Int numThreads[kThreadGroupAxisCount];
            getComputeThreadGroupSize(irFunc, numThreads);
       
            // TODO(JS): We might want to store this information such that it can be used to execute
            m_writer->emit("// [numthreads(");
            for (int ii = 0; ii < kThreadGroupAxisCount; ++ii)
            {
                if (ii != 0) m_writer->emit(", ");
                m_writer->emit(numThreads[ii]);
            }
            m_writer->emit(")]\n");
            break;
        }
        default: break;
    }

    m_writer->emit("SLANG_PRELUDE_EXPORT\n");
}

void CPPSourceEmitter::emitSimpleFuncImpl(IRFunc* func)
{
    // Emit function decorations
    emitFuncDecorations(func);

    auto resultType = func->getResultType();

    auto name = getName(func);

    // Deal with decorations that need
    // to be emitted as attributes

    // We start by emitting the result type and function name.
    //
    if (IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>())
    {
        // Note: we currently emit multiple functions to represent an entry point
        // on CPU/CUDA, and these all bottleneck through the actual `IRFunc`
        // here as a workhorse.
        //
        // Because the workhorse function doesn't have the right signature to service
        // general-purpose calls, it is being emitted with a `_` prefix.
        //
        StringBuilder prefixName;
        prefixName << "_" << name;
        emitType(resultType, prefixName);
    }
    else
    {
        emitType(resultType, name);
    }

    // Next we emit the parameter list of the function.
    //
    m_writer->emit("(");
    auto firstParam = func->getFirstParam();
    for (auto pp = firstParam; pp; pp = pp->getNextParam())
    {
        // Ingore TypeType-typed parameters for now.
        // In the future we will pass around runtime type info
        // for TypeType parameters.
        if (as<IRTypeType>(pp->getFullType()))
            continue;

        if (pp != firstParam)
            m_writer->emit(", ");

        emitSimpleFuncParamImpl(pp);
    }
    m_writer->emit(")");

    emitSemantics(func);

    // TODO: encode declaration vs. definition
    if (isDefinition(func))
    {
        m_writer->emit("\n{\n");
        m_writer->indent();

        // Need to emit the operations in the blocks of the function
        emitFunctionBody(func);

        m_writer->dedent();
        m_writer->emit("}\n\n");
    }
    else
    {
        m_writer->emit(";\n\n");
    }
}

void CPPSourceEmitter::emitSimpleValueImpl(IRInst* inst)
{
    if (inst->getOp() == kIROp_FloatLit)
    {
        IRConstant* constantInst = static_cast<IRConstant*>(inst);
        switch (constantInst->getFloatKind())
        {
            case IRConstant::FloatKind::Nan:
            {
                // TODO(JS): 
                // It's not clear this will work on all targets.
                // In particular Visual Studio reports an error with this expression. 
                m_writer->emit("(0.0 / 0.0)");
                break;
            }
            case IRConstant::FloatKind::PositiveInfinity:
            {
                m_writer->emit("SLANG_INFINITY");
                break;
            }
            case IRConstant::FloatKind::NegativeInfinity:
            {
                m_writer->emit("(-SLANG_INFINITY)");
                break;
            }
            default:
            {
                m_writer->emit(constantInst->value.floatVal);

                // If the literal is a float, then we need to add 'f' at end, as
                // without literal suffix the value defaults to double.
                IRType* type = constantInst->getDataType();
                if (type && type->getOp() == kIROp_FloatType)
                {
                    m_writer->emitChar('f');
                }
                break;
            }
        }
    }
    else
    {
        Super::emitSimpleValueImpl(inst);
    }
}

void CPPSourceEmitter::emitSimpleFuncParamImpl(IRParam* param)
{
    CLikeSourceEmitter::emitSimpleFuncParamImpl(param);
}

void CPPSourceEmitter::emitVectorTypeNameImpl(IRType* elementType, IRIntegerValue elementCount)
{
    emitSimpleType(m_typeSet.addVectorType(elementType, int(elementCount)));
}

void CPPSourceEmitter::emitSimpleTypeImpl(IRType* inType)
{
    UnownedStringSlice slice = _getTypeName(m_typeSet.getType(inType));
    m_writer->emit(slice);
}

void CPPSourceEmitter::_emitType(IRType* type, DeclaratorInfo* declarator)
{
    switch (type->getOp())
    {
    default:
        CLikeSourceEmitter::_emitType(type, declarator);
        break;

    case kIROp_PtrType:
    case kIROp_InOutType:
    case kIROp_OutType:
        {
            auto ptrType = cast<IRPtrTypeBase>(type);
            PtrDeclaratorInfo ptrDeclarator(declarator);
            _emitType(ptrType->getValueType(), &ptrDeclarator);
        }
        break;
    case kIROp_RefType:
        {
            auto ptrType = cast<IRPtrTypeBase>(type);
            RefDeclaratorInfo refDeclarator(declarator);
            _emitType(ptrType->getValueType(), &refDeclarator);
        }
        break;
    case kIROp_ArrayType:
        {
            auto arrayType = static_cast<IRArrayType*>(type);
            auto elementType = arrayType->getElementType();
            int elementCount = int(getIntVal(arrayType->getElementCount()));

            m_writer->emit("FixedArray<");
            _emitType(elementType, nullptr);
            m_writer->emit(", ");
            m_writer->emit(elementCount);
            m_writer->emit("> ");
            emitDeclarator(declarator);
        }
        break;
    case kIROp_UnsizedArrayType:
        {
            auto arrayType = static_cast<IRUnsizedArrayType*>(type);
            auto elementType = arrayType->getElementType();

            m_writer->emit("Array<");
            _emitType(elementType, nullptr);
            m_writer->emit("> ");
            emitDeclarator(declarator);
        }
        break;
    case kIROp_FuncType:
        {
            auto funcType = cast<IRFuncType>(type);
            m_writer->emit("Slang_FuncType<");
            _emitType(funcType->getResultType(), nullptr);
            for (UInt i = 0; i < funcType->getParamCount(); i++)
            {
                m_writer->emit(", ");
                _emitType(funcType->getParamType(i), nullptr);
            }
            m_writer->emit("> ");
            emitDeclarator(declarator);
        }
        break;
    }
}

void CPPSourceEmitter::emitIntrinsicCallExprImpl(
    IRCall*                         inst,
    IRTargetIntrinsicDecoration*    targetIntrinsic,
    EmitOpInfo const&               inOuterPrec)
{
    typedef HLSLIntrinsic::Op Op;

    // TODO: Much of this logic duplicates code that is already
    // in `CLikeSourceEmitter::emitIntrinsicCallExpr`. The only
    // real difference is that when things bottom out on an ordinary
    // function call there is logic to look up a C/C++-backend-specific
    // opcode based on the function name, and emit using that.

    auto outerPrec = inOuterPrec;
    bool needClose = false;

    Index argCount = Index(inst->getArgCount());
    auto args = inst->getArgs();
    
    auto name = targetIntrinsic->getDefinition();

    // We will special-case some names here, that
    // represent callable declarations that aren't
    // ordinary functions, and thus may use different
    // syntax.
    if (name == ".operator[]")
    {
        SLANG_ASSERT(argCount == 2 || argCount == 3);

        // If the first item is either a matrix or a vector, we use 'getAt' logic
        IRType* targetType = args[0].get()->getDataType();
        if (targetType->getOp() == kIROp_VectorType || targetType->getOp() == kIROp_MatrixType)
        {
            // Work out the intrinsic used
            HLSLIntrinsic intrinsic;
            m_intrinsicSet.calcIntrinsic(HLSLIntrinsic::Op::GetAt, inst->getDataType(), args, 2, intrinsic);
            HLSLIntrinsic* specOp = m_intrinsicSet.add(intrinsic);

            if (argCount == 2)
            {
                // Load
                emitCall(specOp, inst, args, 2, inOuterPrec);
            }
            else
            {
                // Store
                auto prec = getInfo(EmitOp::Postfix);
                needClose = maybeEmitParens(outerPrec, prec);

                emitCall(specOp, inst, inst->getOperands(), 2, inOuterPrec);

                m_writer->emit(" = ");
                emitOperand(inst->getOperand(2), getInfo(EmitOp::General));

                maybeCloseParens(needClose);
            }
        }
        else
        {
            // The user is invoking a built-in subscript operator

            // Determine if we are calling the `ref` accessor:
            // `ref` accessor returns a pointer of element type.
            auto ptrType = as<IRPtrType>(inst->getFullType());
            auto resourceType = inst->getOperand(1)->getFullType();
            auto elementType = resourceType ? resourceType->getOperand(0) : nullptr;
            bool isRef = ptrType && ptrType->getValueType() == elementType;

            auto emitSubscript = [this, &args](EmitOpInfo _outerPrec)
            {
                auto prec = getInfo(EmitOp::Postfix);
                bool needCloseSubscript = maybeEmitParens(_outerPrec, prec);
                emitOperand(args[0].get(), leftSide(_outerPrec, prec));
                m_writer->emit("[");
                emitOperand(args[1].get(), getInfo(EmitOp::General));
                m_writer->emit("]");
                maybeCloseParens(needCloseSubscript);
            };

            if (isRef)
            {
                auto prefixPrec = getInfo(EmitOp::Prefix);
                needClose = maybeEmitParens(outerPrec, prefixPrec);
                m_writer->emit("&");
                outerPrec = rightSide(outerPrec, prefixPrec);
            }
            emitSubscript(outerPrec);
            maybeCloseParens(needClose);
            if (argCount == 3)
            {
                m_writer->emit(" = ");
                emitOperand(args[2].get(), getInfo(EmitOp::General));
            }
        }

        return;
    }

    {
        Op op = m_opLookup->getOpByName(name);
        if (op != Op::Invalid)
        {

            // Work out the intrinsic used
            HLSLIntrinsic intrinsic;
            m_intrinsicSet.calcIntrinsic(op, inst->getDataType(), args, argCount, intrinsic);
            HLSLIntrinsic* specOp = m_intrinsicSet.add(intrinsic);

            emitCall(specOp, inst, args, int(argCount), inOuterPrec);
            return;
        }
    }

    // Use default impl (which will do intrinsic special macro expansion as necessary)
    return Super::emitIntrinsicCallExprImpl(inst, targetIntrinsic, inOuterPrec);
}

void CPPSourceEmitter::emitLoopControlDecorationImpl(IRLoopControlDecoration* decl)
{
    if (decl->getMode() == kIRLoopControl_Unroll)
    {
        // This relies on a suitable definition in slang-cpp-prelude.h or defined in C++ compiler invocation.
        m_writer->emit("SLANG_UNROLL\n");
    }
}

const UnownedStringSlice* CPPSourceEmitter::getVectorElementNames(BaseType baseType, Index elemCount)
{
    SLANG_UNUSED(baseType);
    SLANG_UNUSED(elemCount);

    static const UnownedStringSlice elemNames[] =
    {
        UnownedStringSlice::fromLiteral("x"),
        UnownedStringSlice::fromLiteral("y"),
        UnownedStringSlice::fromLiteral("z"),
        UnownedStringSlice::fromLiteral("w"),
    };

    return elemNames;
}

const UnownedStringSlice* CPPSourceEmitter::getVectorElementNames(IRVectorType* vectorType)
{
    Index elemCount = Index(getIntVal(vectorType->getElementCount()));

    IRType* type = vectorType->getElementType()->getCanonicalType();
    IRBasicType* basicType = as<IRBasicType>(type);
    SLANG_ASSERT(basicType);
    return getVectorElementNames(basicType->getBaseType(), elemCount);
}

bool CPPSourceEmitter::_tryEmitInstExprAsIntrinsic(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    HLSLIntrinsic* specOp = m_intrinsicSet.add(inst);
    if (specOp)
    {
        if (inst->getOp() == kIROp_Call)
        {
            IRCall* call = static_cast<IRCall*>(inst);
            emitCall(specOp, inst, call->getArgs(), int(call->getArgCount()), inOuterPrec);
        }
        else
        {
            emitCall(specOp, inst, inst->getOperands(), int(inst->getOperandCount()), inOuterPrec);
        }
        return true;
    }
    return false;
}

bool CPPSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
        default:
        {
            return _tryEmitInstExprAsIntrinsic(inst, inOuterPrec);
        }
        case kIROp_swizzle:
        {
            // For C++ we don't need to emit a swizzle function
           // For C we need a construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);

            IRInst* baseInst = swizzleInst->getBase();
            IRType* baseType = baseInst->getDataType();

            // If we are swizzling from a built in type, 
            if (as<IRBasicType>(baseType))
            {
                // We can swizzle a scalar type to be a vector, or just a scalar
                IRType* dstType = swizzleInst->getDataType();
                if (as<IRBasicType>(dstType))
                {
                    // If the output is a scalar, then could only have been a .x, which we can just ignore the '.x' part
                    emitOperand(baseInst, inOuterPrec);
                    return true;
                }
            }
            else
            {
                const Index elementCount = Index(swizzleInst->getElementCount());
                if (elementCount == 1)
                {
                    // If just one thing is extracted then the . syntax will just work 
                    defaultEmitInstExpr(inst, inOuterPrec);
                    return true;
                }
            }
            // try doing automatically
            return _tryEmitInstExprAsIntrinsic(inst, inOuterPrec);
        }
        case kIROp_Call:
        {
            auto funcValue = inst->getOperand(0);

            // Does this function declare any requirements.
            handleRequiredCapabilities(funcValue);

            // try doing automatically
            return _tryEmitInstExprAsIntrinsic(inst, inOuterPrec);
        }
        case kIROp_lookup_interface_method:
        {
            emitInstExpr(inst->getOperand(0), inOuterPrec);
            m_writer->emit("->");
            m_writer->emit(getName(inst->getOperand(1)));
            return true;
        }
        case kIROp_GetSequentialID:
        {
            emitInstExpr(inst->getOperand(0), inOuterPrec);
            m_writer->emit("->sequentialID");
            return true;
        }
        case kIROp_WitnessTable:
        {
            m_writer->emit("(&");
            m_writer->emit(getName(inst));
            m_writer->emit(")");
            return true;
        }
        case kIROp_getAddr:
        {
            // Once we clean up the pointer emitting logic, we can
            // just use GetElementAddress instruction in place of
            // getAddr instruction, and this case can be removed.
            m_writer->emit("(&(");
            emitInstExpr(inst->getOperand(0), EmitOpInfo::get(EmitOp::General));
            m_writer->emit("))");
            return true;
        }
        case kIROp_RTTIObject:
        {
            m_writer->emit(getName(inst));
            return true;
        }
        case kIROp_Alloca:
        {
            m_writer->emit("alloca(");
            emitOperand(inst->getOperand(0), EmitOpInfo::get(EmitOp::Postfix));
            m_writer->emit("->typeSize)");
            return true;
        }
        case kIROp_BitCast:
        {
            m_writer->emit("(slang_bit_cast<");
            emitType(inst->getDataType());
            m_writer->emit(">(");
            emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
            m_writer->emit("))");
            return true;
        }
        case kIROp_StringLit:
        {
            auto handler = StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp);

            StringBuilder buf;
            const auto slice = as<IRStringLit>(inst)->getStringSlice();
            StringEscapeUtil::appendQuoted(handler, slice, buf);
            
            if (m_hasString)
            {
                m_writer->emit("toTerminatedSlice(");
                m_writer->emit(buf);
                m_writer->emit(")");
            }
            else
            {
                m_writer->emit(buf);
            }

            return true;
        }
        case kIROp_PtrLit:
        {
            auto ptrVal = as<IRPtrLit>(inst)->value.ptrVal;
            if (ptrVal == nullptr)
            {
                m_writer->emit("nullptr");
            }
            else
            {
                m_writer->emit("reinterpret_cast<");
                emitType(inst->getFullType());
                m_writer->emit(">(");
                m_writer->emitUInt64((uint64_t)ptrVal);
                m_writer->emit(")");
            }
            return true;
        }
        case kIROp_MakeExistential:
        case kIROp_MakeExistentialWithRTTI:
        {
            auto rsType = cast<IRComPtrType>(inst->getDataType());
            m_writer->emit("ComPtr<");
            m_writer->emit(getName(rsType->getOperand(0)));
            m_writer->emit(">(");
            m_writer->emit("static_cast<");
            m_writer->emit(getName(rsType->getOperand(0)));
            m_writer->emit("*>(");
            auto prec = getInfo(EmitOp::Postfix);
            emitOperand(inst->getOperand(0), leftSide(getInfo(EmitOp::General), prec));
            m_writer->emit(".Ptr()");
            m_writer->emit("))");
            return true;
        }
        case kIROp_GetValueFromBoundInterface:
        {
            m_writer->emit("static_cast<");
            m_writer->emit(getName(inst->getFullType()));
            m_writer->emit("*>(");
            auto prec = getInfo(EmitOp::Postfix);
            emitOperand(inst->getOperand(0), leftSide(getInfo(EmitOp::General), prec));
            m_writer->emit(".get()");
            m_writer->emit(")");
            return true;
        }
    }
}

// We want order of built in types (typically output nothing), vector, matrix, other types
// Types that aren't output have negative indices
static Index _calcTypeOrder(IRType* a)
{
    switch (a->getOp())
    {
        case kIROp_FuncType:
        {
            return -2;
        }
        case kIROp_VectorType: return 1;
        case kIROp_MatrixType: return 2;
        default:
        {
            if (as<IRBasicType>(a))
            {
                return -1;
            }
            return 3;
        }
    }
}

void CPPSourceEmitter::emitPreModuleImpl()
{
    if (m_target == CodeGenTarget::CPPSource)
    {
        // TODO(JS): Previously this opened an anonymous scope for all generated functions
        // Unfortunately this is a problem if we are just emitting code that is externally available
        // and is not only accessible through entry points. So for now we disable

        // that this opens an anonymous scope. 
        // The scope is closed in `emitModuleImpl`

        //m_writer->emit("namespace { // anonymous \n\n");

        // When generating kernel code in C++, put all into an anonymous namespace
        // This includes any generated types, and generated intrinsics.
        m_writer->emit("#ifdef SLANG_PRELUDE_NAMESPACE\n");
        m_writer->emit("using namespace SLANG_PRELUDE_NAMESPACE;\n");
        m_writer->emit("#endif\n\n");
    }

    // Emit generated functions and types

    if (m_target == CodeGenTarget::CSource)
    {
        // For C output we need to emit type definitions.
        List<IRType*> types;
        m_typeSet.getTypes(types);

        // Remove ones we don't need to emit
        for (Index i = 0; i < types.getCount(); ++i)
        {
            if (_calcTypeOrder(types[i]) < 0)
            {
                types.fastRemoveAt(i);
                --i;
            }
        }

        // Sort them so that vectors come before matrices and everything else after that
        types.sort([&](IRType* a, IRType* b) { return _calcTypeOrder(a) < _calcTypeOrder(b); });

        // Emit the type definitions
        for (auto type : types)
        {
            emitTypeDefinition(type);
        }
    }

    {
        List<const HLSLIntrinsic*> intrinsics;
        m_intrinsicSet.getIntrinsics(intrinsics);

        // Emit all the intrinsics that were used
        for (auto intrinsic : intrinsics)
        {
            _maybeEmitSpecializedOperationDefinition(intrinsic);
        }
    }
}


void CPPSourceEmitter::emitGlobalInstImpl(IRInst* inst)
{
    if (as<IRGlobalVar>(inst) && inst->findDecoration<IRExternCppDecoration>())
    {
        // JS:
        // Turns out just doing extern "C" means something different on a variable
        // So we need to wrap in extern "C" { }
        m_writer->emit("extern \"C\" {\n");
        Super::emitGlobalInstImpl(inst);
        m_writer->emit("\n}\n");
    }
    else
    {
        Super::emitGlobalInstImpl(inst);
    }
}

static bool _isExported(IRInst* inst)
{
    for (auto decoration : inst->getDecorations())
    {
        const auto op = decoration->getOp();
        if (op == kIROp_PublicDecoration ||
            op == kIROp_HLSLExportDecoration)
        {
            return true;
        }
    }
    return false;
}

void CPPSourceEmitter::emitVarDecorationsImpl(IRInst* inst)
{
    if (as<IRGlobalVar>(inst) && _isExported(inst))
    {
        m_writer->emit("SLANG_PRELUDE_SHARED_LIB_EXPORT\n");
    }

    Super::emitVarDecorationsImpl(inst);
}

void CPPSourceEmitter::_getExportStyle(IRInst* inst, bool& outIsExport, bool& outIsExternC)
{
    outIsExport = false;
    outIsExternC = false;
    // Specially handle export, as we don't want to emit it multiple times
    if (getTargetReq()->isWholeProgramRequest())
    {
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
        {
            if (nameHint->getName() == "main")
            {
                // Don't output any decorations on main function.
                return;
            }
        }

        // If public/export made it externally visible
        for (auto decoration : inst->getDecorations())
        {
            const auto op = decoration->getOp();
            if (op == kIROp_ExternCppDecoration)
            {
                outIsExternC = true;
            }
            else if (op == kIROp_PublicDecoration ||
                op == kIROp_HLSLExportDecoration)
            {
                outIsExport = true;
            }
        }
    }
}

void CPPSourceEmitter::_maybeEmitExportLike(IRInst* inst)
{
    bool isExternC = false;
    bool isExported = false;
    _getExportStyle(inst, isExternC, isExported);
        
    // TODO(JS): Currently export *also* implies it's extern "C" and we can't list twice
    if (isExported)
    {
        m_writer->emit("SLANG_PRELUDE_EXPORT\n");
    }
    else if (isExternC)
    {
        // It's name is not manged.
        m_writer->emit("extern \"C\"\n");
    }
}

/* virtual */void CPPSourceEmitter::emitFuncDecorationsImpl(IRFunc* func)
{
    _maybeEmitExportLike(func);

    // Use the default for others
    Super::emitFuncDecorationsImpl(func);
}

void CPPSourceEmitter::emitOperandImpl(IRInst* inst, EmitOpInfo const&  outerPrec)
{
    if (shouldFoldInstIntoUseSites(inst))
    {
        emitInstExpr(inst, outerPrec);
        return;
    }

    switch (inst->getOp())
    {
        case kIROp_Var:
        case kIROp_GlobalVar:
            emitVarExpr(inst, outerPrec);
            break;
        default:
            m_writer->emit(getName(inst));
            break;
    }
}

/* static */bool CPPSourceEmitter::_isVariable(IROp op)
{
    switch (op)
    {
        case kIROp_GlobalVar:
        case kIROp_GlobalParam:
        //case kIROp_Var:
        {
            return true;
        }
        default: return false;
    }
}

static bool _isFunction(IROp op)
{
    return op == kIROp_Func;
}

void CPPSourceEmitter::_emitEntryPointDefinitionStart(IRFunc* func, const String& funcName, const UnownedStringSlice& varyingTypeName)
{
    auto resultType = func->getResultType();
    
    auto entryPointDecl = func->findDecoration<IREntryPointDecoration>();
    SLANG_ASSERT(entryPointDecl);

    // Emit the actual function
    emitEntryPointAttributes(func, entryPointDecl);
    emitType(resultType, funcName);

    m_writer->emit("(");
    m_writer->emit(varyingTypeName);
    m_writer->emit("* varyingInput, void* entryPointParams, void* globalParams)");
    emitSemantics(func);
    m_writer->emit("\n{\n");

    m_writer->indent();
}

void CPPSourceEmitter::_emitEntryPointDefinitionEnd(IRFunc* func)
{
    SLANG_UNUSED(func);
    m_writer->dedent();
    m_writer->emit("}\n");
}

namespace { // anonymous

struct AxisWithSize
{
    typedef AxisWithSize ThisType;
    bool operator<(const ThisType& rhs) const { return size < rhs.size || (size == rhs.size && axis < rhs.axis); }

    int axis;
    Int size;
};

} // anonymous

static void _calcAxisOrder(const Int sizeAlongAxis[CLikeSourceEmitter::kThreadGroupAxisCount], bool allowSingle, List<AxisWithSize>& out)
{
    out.clear();
    // Add in order z,y,x, so by default (if we don't sort), x will be the inner loop
    for (int i = CLikeSourceEmitter::kThreadGroupAxisCount - 1; i >= 0; --i)
    {
        if (allowSingle || sizeAlongAxis[i] > 1)
        {
            AxisWithSize axisWithSize;
            axisWithSize.axis = i;
            axisWithSize.size = sizeAlongAxis[i];
            out.add(axisWithSize);
        }
    }

    // The sort here works to make the axis with the highest value the inner most loop.
    // Disabled for now to make the order well defined as x, y, z
    // axes.sort();
}

void CPPSourceEmitter::_emitEntryPointGroup(const Int sizeAlongAxis[kThreadGroupAxisCount], const String& funcName)
{
    List<AxisWithSize> axes;
    _calcAxisOrder(sizeAlongAxis, false, axes);

    // Open all the loops
    StringBuilder builder;
    for (Index i = 0; i < axes.getCount(); ++i)
    {
        const auto& axis = axes[i];
        builder.Clear();
        const char elem[2] = { s_xyzwNames[axis.axis], 0 };
        builder << "for (uint32_t " << elem << " = 0; " << elem << " < " << axis.size << "; ++" << elem << ")\n{\n";
        m_writer->emit(builder);
        m_writer->indent();

        builder.Clear();
        builder << "threadInput.groupThreadID." << elem << " = " << elem << ";\n";
        m_writer->emit(builder);
    }

    // just call at inner loop point
    m_writer->emit("_");
    m_writer->emit(funcName);
    m_writer->emit("(&threadInput, entryPointParams, globalParams);\n");

    // Close all the loops
    for (Index i = Index(axes.getCount() - 1); i >= 0; --i)
    {
        m_writer->dedent();
        m_writer->emit("}\n");
    }
}

void CPPSourceEmitter::_emitEntryPointGroupRange(const Int sizeAlongAxis[kThreadGroupAxisCount], const String& funcName)
{
    List<AxisWithSize> axes;
    _calcAxisOrder(sizeAlongAxis, true, axes);

    // Open all the loops
    StringBuilder builder;
    for (Index i = 0; i < axes.getCount(); ++i)
    {
        const auto& axis = axes[i];
        builder.Clear();
        const char elem[2] = { s_xyzwNames[axis.axis], 0 };

        builder << "for (uint32_t " << elem << " = vi.startGroupID." << elem << "; " << elem << " < vi.endGroupID." << elem << "; ++" << elem << ")\n{\n";
        m_writer->emit(builder);
        m_writer->indent();

        m_writer->emit("groupVaryingInput.startGroupID.");
        m_writer->emit(elem);
        m_writer->emit(" = ");
        m_writer->emit(elem);
        m_writer->emit(";\n");
    }

    // just call at inner loop point
    m_writer->emit(funcName);
    m_writer->emit("_Group(&groupVaryingInput, entryPointParams, globalParams);\n");

    // Close all the loops
    for (Index i = Index(axes.getCount() - 1); i >= 0; --i)
    {
        m_writer->dedent();
        m_writer->emit("}\n");
    }
}
void CPPSourceEmitter::_emitInitAxisValues(const Int sizeAlongAxis[kThreadGroupAxisCount], const UnownedStringSlice& mulName, const UnownedStringSlice& addName)
{
    StringBuilder builder;

    m_writer->emit("{\n");
    m_writer->indent();
    for (int i = 0; i < kThreadGroupAxisCount; ++i)
    {
        builder.Clear();
        const char elem[2] = { s_xyzwNames[i], 0 };
        builder << mulName << "." << elem << " * " << sizeAlongAxis[i];
        if (addName.getLength() > 0)
        {
            builder << " + " << addName << "." << elem;
        }
        if (i < kThreadGroupAxisCount - 1)
        {
            builder << ",";
        }
        builder << "\n";
        m_writer->emit(builder);
    }
    m_writer->dedent();
    m_writer->emit("};\n");
}

void CPPSourceEmitter::_emitForwardDeclarations(const List<EmitAction>& actions)
{
    // Emit forward declarations. Don't emit variables that need to be grouped or function definitions (which will ref those types)
    for (auto action : actions)
    {
        switch (action.level)
        {
            case EmitAction::Level::ForwardDeclaration:
                {
                    switch (action.inst->getOp())
                    {
                    case kIROp_Func:
                    case kIROp_StructType:
                    case kIROp_InterfaceType:
                        emitForwardDeclaration(action.inst);
                        break;
                    default:
                        break;
                    }
                }
                break;

            case EmitAction::Level::Definition:
                if (_isVariable(action.inst->getOp()) || _isFunction(action.inst->getOp()))
                {
                    // Don't emit functions or variables that have to be grouped into structures yet
                }
                else
                {
                    emitGlobalInst(action.inst);
                }
                break;
        }
    }
}

void CPPSourceEmitter::emitModuleImpl(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    // Setup all built in types used in the module
    m_typeSet.addAllBuiltinTypes(module);
    // If any matrix types are used, then we need appropriate vector types too.
    m_typeSet.addVectorForMatrixTypes();

    List<EmitAction> actions;
    computeEmitActions(module, actions);
    
    _emitForwardDeclarations(actions);

    {
        // Output all the thread locals 
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && action.inst->getOp() == kIROp_GlobalVar)
            {
                emitGlobalInst(action.inst);
            }
        }

        // Finally output the functions as methods on the context
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && _isFunction(action.inst->getOp()))
            {
                emitGlobalInst(action.inst);
            }
        }
    }

    // Emit all witness table definitions.
    _emitWitnessTableDefinitions();

    // TODO(JS): 
    // Previously output code was placed in an anonymous namespace
    // Now that we can have any function available externally (not just entry points)
    // this doesn't work. 

    //if (m_target == CodeGenTarget::CPPSource)
    //{
        // Need to close the anonymous namespace when outputting for C++ kernel.
        //m_writer->emit("} // anonymous\n\n");
    //}

     // Finally we need to output dll entry points

    for (auto action : actions)
    {
        if (action.level == EmitAction::Level::Definition && _isFunction(action.inst->getOp()))
        {
            IRFunc* func = as<IRFunc>(action.inst);

            IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>();
          
            if (entryPointDecor && entryPointDecor->getProfile().getStage() == Stage::Compute)
            {
                Int groupThreadSize[kThreadGroupAxisCount];
                getComputeThreadGroupSize(func, groupThreadSize);
                
                String funcName = getName(func);

                {
                    StringBuilder builder;
                    builder << funcName << "_Thread";

                    String threadFuncName = builder;

                    _emitEntryPointDefinitionStart(func, threadFuncName, UnownedStringSlice::fromLiteral("ComputeThreadVaryingInput"));

                    m_writer->emit("_");
                    m_writer->emit(funcName);
                    m_writer->emit("(varyingInput, entryPointParams, globalParams);\n");

                    _emitEntryPointDefinitionEnd(func);
                }

                // Emit the group version which runs for all elements in *single* thread group
                {
                    StringBuilder builder;
                    builder << getName(func);
                    builder << "_Group";

                    String groupFuncName = builder;

                    _emitEntryPointDefinitionStart(func, groupFuncName, UnownedStringSlice::fromLiteral("ComputeVaryingInput"));

                    m_writer->emit("ComputeThreadVaryingInput threadInput = {};\n");
                    m_writer->emit("threadInput.groupID = varyingInput->startGroupID;\n");

                    _emitEntryPointGroup(groupThreadSize, funcName);
                    _emitEntryPointDefinitionEnd(func);
                }

                // Emit the main version - which takes a dispatch size
                {
                    _emitEntryPointDefinitionStart(func, funcName, UnownedStringSlice::fromLiteral("ComputeVaryingInput"));

                    m_writer->emit("ComputeVaryingInput vi = *varyingInput;\n");
                    m_writer->emit("ComputeVaryingInput groupVaryingInput = {};\n");

                    _emitEntryPointGroupRange(groupThreadSize, funcName);
                    _emitEntryPointDefinitionEnd(func);
                }
            }
        }
    }
}

} // namespace Slang
