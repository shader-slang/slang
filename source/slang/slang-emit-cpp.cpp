// slang-emit-cpp.cpp
#include "slang-emit-cpp.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include "slang-ir-clone.h"

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

void CPPSourceEmitter::emitTypeDefinition(IRType* inType)
{
    if (m_target == CodeGenTarget::CPPSource)
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
            break;
        }
        case kIROp_MatrixType:
        {
            auto matType = static_cast<IRMatrixType*>(type);

            const auto rowCount = int(GetIntVal(matType->getRowCount()));
            const auto colCount = int(GetIntVal(matType->getColumnCount()));

            IRType* vecType = m_typeSet.addVectorType(matType->getElementType(), colCount);
            
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
    switch (type->op)
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
            auto vecCount = int(GetIntVal(vecType->getElementCount()));
            auto elemType = vecType->getElementType();

            if (target == CodeGenTarget::CPPSource || target == CodeGenTarget::CUDASource)
            {
                out << "Vector<" << _getTypeName(elemType) << ", " << vecCount << ">";
            }
            else
            {             
                out << "Vec";
                UnownedStringSlice postFix = _getCTypeVecPostFix(elemType->op);

                out << postFix;
                if (postFix.size() > 1)
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
            const auto rowCount = int(GetIntVal(matType->getRowCount()));
            const auto colCount = int(GetIntVal(matType->getColumnCount()));

            if (target == CodeGenTarget::CPPSource || target == CodeGenTarget::CUDASource)
            {
                out << "Matrix<" << _getTypeName(elementType) << ", " << rowCount << ", " << colCount << ">";
            }
            else
            {
                out << "Mat";
                const UnownedStringSlice postFix = _getCTypeVecPostFix(_getCType(elementType->op));
                out  << postFix;
                if (postFix.size() > 1)
                {
                    out << "_";
                }
                out << rowCount;
                out << colCount;
            }
            return SLANG_OK;
        }
        case kIROp_ArrayType:
        {
            auto arrayType = static_cast<IRArrayType*>(type);
            auto elementType = arrayType->getElementType();
            int elementCount = int(GetIntVal(arrayType->getElementCount()));

            out << "FixedArray<";
            SLANG_RETURN_ON_FAIL(calcTypeName(elementType, target, out));
            out << ", " << elementCount << ">";
            return SLANG_OK;
        }
        case kIROp_UnsizedArrayType:
        {
            auto arrayType = static_cast<IRUnsizedArrayType*>(type);
            auto elementType = arrayType->getElementType();

            out << "Array<";
            SLANG_RETURN_ON_FAIL(calcTypeName(elementType, target, out));
            out << ">";
            return SLANG_OK;
        }
        default:
        {
            if (isNominalOp(type->op))
            {
                out << getName(type);
                return SLANG_OK;
            }

            if (IRBasicType::isaImpl(type->op))
            {
                out << getBuiltinTypeName(type->op);
                return SLANG_OK;
            }

            if (auto texType = as<IRTextureTypeBase>(type))
            {
                // We don't support TextureSampler, so ignore that
                if (texType->op != kIROp_TextureSamplerType)
                {
                    return _calcCPPTextureTypeName(texType, out);   
                }
            }

            // If _getResourceTypePrefix returns something, we assume can output any specialization after it in order.
            {
                UnownedStringSlice prefix = _getResourceTypePrefix(type->op);
                if (prefix.size() > 0)
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
    if (type->op == kIROp_PtrType)
    {
        // TODO(JS):
        // If it's a pointer type we ignore. We may want to strip but in practice it's
        // probably not necessary.
        return;
    }

    _getTypeName(type);
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
    if (funcName.size() > 0)
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
    SLANG_ASSERT(funcName.size() > 0);

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
        scalarFuncName = _getScalarFuncName(specOp->op, _getElementType(funcType->getParamType(0)));
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

void CPPSourceEmitter::_emitVecMatMulDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);
    IRType* paramType0 = funcType->getParamType(0);
    IRType* paramType1 = funcType->getParamType(1);
    IRType* retType = specOp->returnType;

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

void CPPSourceEmitter::_emitCrossDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    _emitSignature(funcName, specOp);

    SourceWriter* writer = getSourceWriter();

    writer->emit("\n{\n");
    writer->indent();

    writer->emit("return ");
    if (m_target == CodeGenTarget::CUDASource)
    {
        m_writer->emit("make_");
        emitType(specOp->returnType);
        writer->emit("( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x ); \n");
    }
    else
    {
        emitType(specOp->returnType);
        writer->emit("{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; \n");
    }

    
    writer->dedent();
    writer->emit("}\n\n");
}

UnownedStringSlice CPPSourceEmitter::_getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op op, IRType*const* argTypes, Int argCount, IRType* retType)
{
    HLSLIntrinsic intrinsic;
    m_intrinsicSet.calcIntrinsic(op, retType, argTypes, argCount, intrinsic);
    auto specOp = m_intrinsicSet.add(intrinsic);
    _maybeEmitSpecializedOperationDefinition(specOp);
    return  _getFuncName(specOp);
}

void CPPSourceEmitter::_emitLengthDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 1);
    IRType* paramType0 = funcType->getParamType(0);

    SLANG_ASSERT(paramType0->op == kIROp_VectorType);

    IRBasicType* elementType = as<IRBasicType>(static_cast<IRVectorType*>(paramType0)->getElementType());

    IRType* dotArgs[] = { paramType0, paramType0 };
    UnownedStringSlice dotFuncName = _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op::Dot, dotArgs, SLANG_COUNT_OF(dotArgs), elementType);

    UnownedStringSlice sqrtName = _getScalarFuncName(HLSLIntrinsic::Op::Sqrt, elementType);

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

void CPPSourceEmitter::_emitGetAtDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(0);

    for (Index i = 0; i < 2; ++i)
    {
        UnownedStringSlice typePrefix = (i == 0) ? UnownedStringSlice::fromLiteral("const ") : UnownedStringSlice();

        emitFunctionPreambleImpl(nullptr);

        writer->emit(typePrefix);
        emitType(specOp->returnType);
        m_writer->emit("& ");

        writer->emit(funcName);
        writer->emit("(");

        writer->emit(typePrefix);
        emitType(funcType->getParamType(0));
        writer->emit("& a,  ");
        emitType(funcType->getParamType(1));
        writer->emit(" b)\n{\n");

        writer->indent();

        if (auto vectorType = as<IRVectorType>(srcType))
        {
            int vecSize = int(GetIntVal(vectorType->getElementCount()));

            writer->emit("assert(b >= 0 && b < ");
            writer->emit(vecSize);
            writer->emit(");\n");

            writer->emit("return (&a.x)[b];\n");
        }
        else if (auto matrixType = as<IRMatrixType>(srcType))
        {
            //int colCount = int(GetIntVal(matrixType->getColumnCount()));
            int rowCount = int(GetIntVal(matrixType->getRowCount()));

            writer->emit("assert(b >= 0 && b < ");
            writer->emit(rowCount);
            writer->emit(");\n");

            writer->emit("return a.rows[b];\n");
        }

        writer->dedent();
        writer->emit("}\n\n");
    }
}

void CPPSourceEmitter::_emitNormalizeDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{    
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 1);
    IRType* paramType0 = funcType->getParamType(0);

    SLANG_ASSERT(paramType0->op == kIROp_VectorType);

    IRBasicType* elementType = as<IRBasicType>(static_cast<IRVectorType*>(paramType0)->getElementType());

    IRType* dotArgs[] = { paramType0, paramType0 };
    UnownedStringSlice dotFuncName = _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op::Dot, dotArgs, SLANG_COUNT_OF(dotArgs), elementType);
    UnownedStringSlice rsqrtName = _getScalarFuncName(HLSLIntrinsic::Op::RecipSqrt, elementType);
    IRType* vecMulScalarArgs[] = { paramType0, elementType };
    UnownedStringSlice vecMulScalarName = _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op::Mul, vecMulScalarArgs, SLANG_COUNT_OF(vecMulScalarArgs), paramType0);

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

    TypeDimension dim = _getTypeDimension(srcType, false);

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

            emitType(dstElemType);
            writer->emit("(");
            _emitAccess(UnownedStringSlice::fromLiteral("a"), dim, i, j, writer);
            writer->emit(")");
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

void CPPSourceEmitter::_emitReflectDefinition(const UnownedStringSlice& funcName, const HLSLIntrinsic* specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp->signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);
    IRType* paramType0 = funcType->getParamType(0);

    SLANG_ASSERT(paramType0->op == kIROp_VectorType);

    IRBasicType* elementType = as<IRBasicType>(static_cast<IRVectorType*>(paramType0)->getElementType());

    // Make sure we have all these functions defined before emitting 
    IRType* dotArgs[] = { paramType0, paramType0 };
    UnownedStringSlice dotFuncName = _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op::Dot, dotArgs, SLANG_COUNT_OF(dotArgs), elementType);

    IRType* subArgs[] = { paramType0, paramType0};
    UnownedStringSlice subFuncName = _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op::Sub, subArgs, SLANG_COUNT_OF(subArgs), paramType0);

    IRType* vecMulScalarArgs[] = { paramType0, elementType };
    UnownedStringSlice vecMulScalarFuncName = _getAndEmitSpecializedOperationDefinition(HLSLIntrinsic::Op::Mul, vecMulScalarArgs, SLANG_COUNT_OF(vecMulScalarArgs), paramType0);

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
        case Op::VecMatMul:
        case Op::Dot:
        {
            return _emitVecMatMulDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Any:
        case Op::All:
        {
            return _emitAnyAllDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Cross:
        {
            return _emitCrossDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Normalize:
        {
            return _emitNormalizeDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Length:
        {
            return _emitLengthDefinition(_getFuncName(specOp), specOp);
        }
        case Op::Reflect:
        {
            return _emitReflectDefinition(_getFuncName(specOp), specOp);
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

    // Getting the name means that this op is registered as used
    
    switch (specOp->op)
    {
        case Op::Init:
        {
            // For C++ we don't need an init function
            // For C we'll need the construct function for the return type
            //UnownedStringSlice name = _getFuncName(specOp);

            IRType* retType = specOp->returnType;

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
                        
                        writer->emit(_getTypeName(retType));
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
        case Op::Swizzle:
        {
            // Currently only works for C++ (we use {} constuction) - which means we don't need to generate a function.
            // For C we need to generate suitable construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);
            const Index elementCount = Index(swizzleInst->getElementCount());

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
                SLANG_RELEASE_ASSERT(irElementIndex->op == kIROp_IntLit);
                IRConstant* irConst = (IRConstant*)irElementIndex;
                UInt elementIndex = (UInt)irConst->value.intVal;
                SLANG_RELEASE_ASSERT(elementIndex < 4);

                writer->emitChar(s_elemNames[elementIndex]);
            }

            writer->emit("}");
            break;
        }
        default:
        {
            const auto& info = HLSLIntrinsic::getInfo(specOp->op);
            // Make sure that the return type is available
            bool isOperator = _isOperator(info.funcName);

            UnownedStringSlice funcName = _getFuncName(specOp);

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
            break;
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
    outBuilder << _getTypePrefix(type->op) << "_" << HLSLIntrinsic::getInfo(op).funcName;
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
        SLANG_ASSERT(basicType);
        return calcScalarFuncName(specOp->op, basicType, outBuilder);
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
            default: break;
        }

        const auto& info = HLSLIntrinsic::getInfo(specOp->op);
        if (info.funcName.size())
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
    m_typeSet(desc.compileRequest->getSession()),
    m_opLookup(new HLSLIntrinsicOpLookup),
    m_intrinsicSet(&m_typeSet, m_opLookup)
{
    m_semanticUsedFlags = 0;
    //m_semanticUsedFlags = SemanticUsedFlag::GroupID | SemanticUsedFlag::GroupThreadID | SemanticUsedFlag::DispatchThreadID;
}

void CPPSourceEmitter::_emitInOutParamType(IRType* type, String const& name, IRType* valueType)
{
    StringSliceLoc nameAndLoc(name.getUnownedSlice());

    if (auto refType = as<IRRefType>(type))
    {
        m_writer->emit("const ");
    }

    UnownedStringSlice slice = _getTypeName(valueType);
    m_writer->emit(slice);
    m_writer->emit("& ");
    m_writer->emitName(nameAndLoc);
}

void CPPSourceEmitter::emitParamTypeImpl(IRType* type, String const& name)
{
    // An `out` or `inout` parameter will have been
    // encoded as a parameter of pointer type, so
    // we need to decode that here.
    //
    if (auto outType = as<IROutType>(type))
    {
        return _emitInOutParamType(type, name, outType->getValueType());
    }
    else if (auto inOutType = as<IRInOutType>(type))
    {
        return _emitInOutParamType(type, name, inOutType->getValueType());
    }
    else if (auto refType = as<IRRefType>(type))
    {
        return _emitInOutParamType(type, name, refType->getValueType());
    }

    emitType(type, name);
}

bool CPPSourceEmitter::tryEmitGlobalParamImpl(IRGlobalParam* varDecl, IRType* varType)
{
    SLANG_UNUSED(varDecl);
    SLANG_UNUSED(varType);

    switch (varType->op)
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

    switch (type->op)
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
    auto stage = profile.GetStage();

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
    auto resultType = func->getResultType();

    auto name = getName(func);

    // Deal with decorations that need
    // to be emitted as attributes

    // We are going to ignore the parameters passed and just pass in the Context

    if (IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>())
    {
        StringBuilder prefixName;
        prefixName << "_" << name;
        emitType(resultType, prefixName);
        m_writer->emit("()\n");
    }
    else
    {
        emitType(resultType, name);

        m_writer->emit("(");
        auto firstParam = func->getFirstParam();
        for (auto pp = firstParam; pp; pp = pp->getNextParam())
        {
            if (pp != firstParam)
                m_writer->emit(", ");

            emitSimpleFuncParamImpl(pp);
        }
        m_writer->emit(")");

        emitSemantics(func);
    }

    // TODO: encode declaration vs. definition
    if (isDefinition(func))
    {
        m_writer->emit("\n{\n");
        m_writer->indent();

        // HACK: forward-declare all the local variables needed for the
        // parameters of non-entry blocks.
        emitPhiVarDecls(func);

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
    switch (inst->op)
    {
        case kIROp_FloatLit:
        {
            IRConstant* constantInst = static_cast<IRConstant*>(inst);

            m_writer->emit(constantInst->value.floatVal);

            // If the literal is a float, then we need to add 'f' at end
            IRType* type = constantInst->getDataType();
            if (type && type->op == kIROp_FloatType )
            {
                m_writer->emitChar('f');
            }
            break;
        }
        default: Super::emitSimpleValueImpl(inst);
    }
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
        if (targetType->op == kIROp_VectorType || targetType->op == kIROp_MatrixType)
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
            auto prec = getInfo(EmitOp::Postfix);
            needClose = maybeEmitParens(outerPrec, prec);

            emitOperand(args[0].get(), leftSide(outerPrec, prec));
            m_writer->emit("[");
            emitOperand(args[1].get(), getInfo(EmitOp::General));
            m_writer->emit("]");

            if (argCount == 3)
            {
                m_writer->emit(" = ");
                emitOperand(args[2].get(), getInfo(EmitOp::General));
            }

            maybeCloseParens(needClose);
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

bool CPPSourceEmitter::_tryEmitInstExprAsIntrinsic(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    HLSLIntrinsic* specOp = m_intrinsicSet.add(inst);
    if (specOp)
    {
        if (inst->op == kIROp_Call)
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
    switch (inst->op)
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
            handleCallExprDecorationsImpl(funcValue);

            // try doing automatically
            return _tryEmitInstExprAsIntrinsic(inst, inOuterPrec);
        }
    }
}

// We want order of built in types (typically output nothing), vector, matrix, other types
// Types that aren't output have negative indices
static Index _calcTypeOrder(IRType* a)
{
    switch (a->op)
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

void CPPSourceEmitter::emitPreprocessorDirectivesImpl()
{
    SourceWriter* writer = getSourceWriter();

    writer->emit("\n");

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

    // Emit all the intrinsics that were used
    for (const auto& keyValue : m_intrinsicNameMap)
    {
        _maybeEmitSpecializedOperationDefinition(keyValue.Key);
    }
}

void CPPSourceEmitter::emitOperandImpl(IRInst* inst, EmitOpInfo const&  outerPrec)
{
    if (shouldFoldInstIntoUseSites(inst))
    {
        emitInstExpr(inst, outerPrec);
        return;
    }

    switch (inst->op)
    {
        case 0: // nothing yet
        case kIROp_GlobalParam:
        {            
            String name = getName(inst);

            if (inst->findDecorationImpl(kIROp_EntryPointParamDecoration))
            {
                // It's an entry point parameter
                // The parameter is held in a struct so always deref
                m_writer->emit("(*");
                m_writer->emit(name);
                m_writer->emit(")");
            }
            else
            {
                // It's in UniformState
                m_writer->emit("(");

                switch (inst->getDataType()->op)
                {
                    case kIROp_ParameterBlockType:
                    case kIROp_ConstantBufferType:
                    case kIROp_StructType:
                    {
                        m_writer->emit("*");
                        break;
                    }
                    default: break;
                }

                m_writer->emit("uniformState->");
                m_writer->emit(name);
                m_writer->emit(")");
            }
            break;
        }
        case kIROp_Param:
        {
            auto varLayout = getVarLayout(inst);

            if (varLayout)
            {
                if(auto systemValueSemantic = varLayout->findSystemValueSemanticAttr())
                {
                    String semanticNameSpelling = systemValueSemantic->getName();
                    semanticNameSpelling = semanticNameSpelling.toLower();

                    if (semanticNameSpelling == "sv_dispatchthreadid")
                    {
                        m_semanticUsedFlags |= SemanticUsedFlag::DispatchThreadID;
                        m_writer->emit("dispatchThreadID");
                        return;
                    }
                    else if (semanticNameSpelling == "sv_groupid")
                    {
                        m_semanticUsedFlags |= SemanticUsedFlag::GroupID;
                        m_writer->emit("groupID");
                        return;
                    }
                    else if (semanticNameSpelling == "sv_groupthreadid")
                    {
                        m_semanticUsedFlags |= SemanticUsedFlag::GroupThreadID;
                        m_writer->emit("calcGroupThreadID()");
                        return;
                    }
                }
            }

            ;   // Fall-thru
        }
        case kIROp_GlobalVar:
        default:
            // GlobalVar should be fine as should just be a member of Context
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

void CPPSourceEmitter::_emitEntryPointDefinitionStart(IRFunc* func, IRGlobalParam* entryPointGlobalParams, const String& funcName, const UnownedStringSlice& varyingTypeName)
{
    auto resultType = func->getResultType();
    
    auto entryPointDecl = func->findDecoration<IREntryPointDecoration>();
    SLANG_ASSERT(entryPointDecl);

    // Emit the actual function
    emitEntryPointAttributes(func, entryPointDecl);
    emitType(resultType, funcName);

    m_writer->emit("(");
    m_writer->emit(varyingTypeName);
    m_writer->emit("* varyingInput, UniformEntryPointParams* params, UniformState* uniformState)");
    emitSemantics(func);
    m_writer->emit("\n{\n");

    m_writer->indent();
    // Initialize when constructing so that globals are zeroed
    m_writer->emit("Context context = {};\n");
    m_writer->emit("context.uniformState = uniformState;\n");
    
    if (entryPointGlobalParams)
    {
        auto varDecl = entryPointGlobalParams;
        auto rawType = varDecl->getDataType();

        auto varType = rawType;

        m_writer->emit("context.");
        m_writer->emit(getName(varDecl));
        m_writer->emit(" =  (");
        emitType(varType);
        m_writer->emit("*)params; \n");
    }
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
        const char elem[2] = { s_elemNames[axis.axis], 0 };
        builder << "for (uint32_t " << elem << " = start." << elem << "; " << elem << " < start." << elem << " + " << axis.size << "; ++" << elem << ")\n{\n";
        m_writer->emit(builder);
        m_writer->indent();

        builder.Clear();
        builder << "context.dispatchThreadID." << elem << " = " << elem << ";\n";
        m_writer->emit(builder);
    }

    // just call at inner loop point
    m_writer->emit("context._");
    m_writer->emit(funcName);
    m_writer->emit("();\n");

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
        const char elem[2] = { s_elemNames[axis.axis], 0 };

        if (m_semanticUsedFlags & SemanticUsedFlag::GroupThreadID)
        {
            builder << "context.groupDispatchThreadID." << elem << " = start." << elem << ";\n";
        }
        if (m_semanticUsedFlags & SemanticUsedFlag::GroupID)
        {
            builder << "context.groupID." << elem << " += varyingInput->startGroupID." << elem << ";\n";
        }

        builder << "for (uint32_t " << elem << " = start." << elem << "; " << elem << " < end." << elem << "; ++" << elem << ")\n{\n";
        m_writer->emit(builder);
        m_writer->indent();

        builder.Clear();
        builder << "context.dispatchThreadID." << elem << " = " << elem << ";\n";

        if (m_semanticUsedFlags & (SemanticUsedFlag::GroupThreadID | SemanticUsedFlag::GroupID))
        {
            if (sizeAlongAxis[axis.axis] > 1)
            {
                builder << "const uint32_t next = context.groupDispatchThreadID." << elem << " + " << axis.size <<";\n";

                if (m_semanticUsedFlags & SemanticUsedFlag::GroupID)
                {
                    builder << "context.groupID." << elem << " += uint32_t(next == " << elem << ");\n";
                }
                if (m_semanticUsedFlags & SemanticUsedFlag::GroupThreadID)
                {
                    builder << "context.groupDispatchThreadID." << elem << " = (" << elem << " == next) ? next : context.groupDispatchThreadID." << elem << ";\n";
                }
            }
            else
            {
                if (m_semanticUsedFlags & SemanticUsedFlag::GroupThreadID)
                {
                    builder << "context.groupDispatchThreadID." << elem << " = " << elem << ";\n";
                }
                if (m_semanticUsedFlags & SemanticUsedFlag::GroupID)
                {
                    builder << "context.groupID." << elem << " = " << elem << ";\n";
                }
            }
        }

        m_writer->emit(builder);
    }

    // just call at inner loop point
    m_writer->emit("context._");
    m_writer->emit(funcName);
    m_writer->emit("();\n");

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
        const char elem[2] = { s_elemNames[i], 0 };
        builder << mulName << "." << elem << " * " << sizeAlongAxis[i];
        if (addName.size() > 0)
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
                emitFuncDecl(cast<IRFunc>(action.inst));
                break;

            case EmitAction::Level::Definition:
                if (_isVariable(action.inst->op) || _isFunction(action.inst->op))
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

void CPPSourceEmitter::_calcGlobalParams(const List<EmitAction>& actions, List<GlobalParamInfo>& outParams, IRGlobalParam** outEntryPointGlobalParams)
{
    outParams.clear();
    *outEntryPointGlobalParams = nullptr;

    IRGlobalParam* entryPointGlobalParams = nullptr;
    for (auto action : actions)
    {
        if (action.level == EmitAction::Level::Definition && action.inst->op == kIROp_GlobalParam)
        {
            auto inst = action.inst;

            if (inst->findDecorationImpl(kIROp_EntryPointParamDecoration))
            {
                // Should only be one instruction marked this way
                SLANG_ASSERT(entryPointGlobalParams == nullptr);
                entryPointGlobalParams = as<IRGlobalParam>(inst);
                continue;
            }

            IRVarLayout* varLayout = CLikeSourceEmitter::getVarLayout(action.inst);
            SLANG_ASSERT(varLayout);

            IRVarOffsetAttr* offsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::Uniform);
            IRTypeLayout* typeLayout = varLayout->getTypeLayout();
            IRTypeSizeAttr* sizeAttr = typeLayout->findSizeAttr(LayoutResourceKind::Uniform);

            GlobalParamInfo paramInfo;
            paramInfo.inst = action.inst;
            // Index is the byte offset for uniform
            paramInfo.offset = offsetAttr ? offsetAttr->getOffset() : 0;
            paramInfo.size = sizeAttr ? sizeAttr->getFiniteSize() : 0;

            outParams.add(paramInfo);
        }
    }

    // We want to sort by layout offset, and insert suitable padding
    outParams.sort();

    *outEntryPointGlobalParams = entryPointGlobalParams;
}

void CPPSourceEmitter::_emitUniformStateMembers(const List<EmitAction>& actions, IRGlobalParam** outEntryPointGlobalParams)
{
    List<GlobalParamInfo> params;
    _calcGlobalParams(actions, params, outEntryPointGlobalParams);

    int padIndex = 0;
    size_t offset = 0;
    for (const auto& paramInfo : params)
    {
        if (offset < paramInfo.offset)
        {
            // We want to output some padding
            StringBuilder builder;
            builder << "uint8_t _pad" << (padIndex++) << "[" << (paramInfo.offset - offset) << "];\n";
        }

        emitGlobalInst(paramInfo.inst);
        // Set offset after this 
        offset = paramInfo.offset + paramInfo.size;
    }
    m_writer->emit("\n");
}

void CPPSourceEmitter::emitModuleImpl(IRModule* module)
{
    // Setup all built in types used in the module
    m_typeSet.addAllBuiltinTypes(module);
    // If any matrix types are used, then we need appropriate vector types too.
    m_typeSet.addVectorForMatrixTypes();

    List<EmitAction> actions;
    computeEmitActions(module, actions);

    _emitForwardDeclarations(actions);

    IRGlobalParam* entryPointGlobalParams = nullptr;

    // Output the global parameters in a 'UniformState' structure
    {
        m_writer->emit("struct UniformState\n{\n");
        m_writer->indent();

        _emitUniformStateMembers(actions, &entryPointGlobalParams);

        m_writer->dedent();
        m_writer->emit("\n};\n\n");
    }

    // Output the 'Context' which will be used for execution
    {
        m_writer->emit("struct Context\n{\n");
        m_writer->indent();

        m_writer->emit("UniformState* uniformState;\n");

        
        m_writer->emit("uint3 dispatchThreadID;\n");

        //if (m_semanticUsedFlags & SemanticUsedFlag::GroupID)
        {
            // Note not always set!
            m_writer->emit("uint3 groupID;\n");
        }

        //if (m_semanticUsedFlags & SemanticUsedFlag::GroupThreadID)
        {
            m_writer->emit("uint3 groupDispatchThreadID;\n");

            m_writer->emit("uint3 calcGroupThreadID() const \n{\n");
            m_writer->indent();
            // groupThreadID = dispatchThreadID - groupDispatchThreadID
            m_writer->emit("uint3 v = { dispatchThreadID.x - groupDispatchThreadID.x, dispatchThreadID.y - groupDispatchThreadID.y, dispatchThreadID.z - groupDispatchThreadID.z };\n");
            m_writer->emit("return v;\n");
            m_writer->dedent();
            m_writer->emit("}\n");
        }

        if (entryPointGlobalParams)
        {
            emitGlobalInst(entryPointGlobalParams);
        }

        // Output all the thread locals 
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && action.inst->op == kIROp_GlobalVar)
            {
                emitGlobalInst(action.inst);
            }
        }

        // Finally output the functions as methods on the context
        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && _isFunction(action.inst->op))
            {
                emitGlobalInst(action.inst);
            }
        }

        m_writer->dedent();
        m_writer->emit("};\n\n");
    }

     // Finally we need to output dll entry points

    for (auto action : actions)
    {
        if (action.level == EmitAction::Level::Definition && _isFunction(action.inst->op))
        {
            IRFunc* func = as<IRFunc>(action.inst);

            IREntryPointDecoration* entryPointDecor = func->findDecoration<IREntryPointDecoration>();
          
            if (entryPointDecor && entryPointDecor->getProfile().GetStage() == Stage::Compute)
            {
                // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sv-dispatchthreadid
                // SV_DispatchThreadID is the sum of SV_GroupID * numthreads and GroupThreadID.

                Int groupThreadSize[kThreadGroupAxisCount];
                getComputeThreadGroupSize(func, groupThreadSize);
                
                String funcName = getName(func);

                {
                    StringBuilder builder;
                    builder << funcName << "_Thread";

                    String threadFuncName = builder;

                    _emitEntryPointDefinitionStart(func, entryPointGlobalParams, threadFuncName, UnownedStringSlice::fromLiteral("ComputeThreadVaryingInput"));

                    if (m_semanticUsedFlags & SemanticUsedFlag::GroupThreadID)
                    {
                        m_writer->emit("context.groupDispatchThreadID = ");
                        _emitInitAxisValues(groupThreadSize, UnownedStringSlice::fromLiteral("varyingInput->groupID"), UnownedStringSlice());
                    }
                    if (m_semanticUsedFlags & SemanticUsedFlag::GroupID)
                    {
                        m_writer->emit("context.groupID = varyingInput->groupID;\n");
                    }

                    // Emit dispatchThreadID
                    m_writer->emit("context.dispatchThreadID = ");
                    _emitInitAxisValues(groupThreadSize, UnownedStringSlice::fromLiteral("varyingInput->groupID"), UnownedStringSlice::fromLiteral("varyingInput->groupThreadID"));

                    m_writer->emit("context._");
                    m_writer->emit(funcName);
                    m_writer->emit("();\n");

                    _emitEntryPointDefinitionEnd(func);
                }

                // Emit the group version which runs for all elements in *single* thread group
                {
                    StringBuilder builder;
                    builder << getName(func);
                    builder << "_Group";

                    String groupFuncName = builder;

                    _emitEntryPointDefinitionStart(func, entryPointGlobalParams, groupFuncName, UnownedStringSlice::fromLiteral("ComputeVaryingInput"));

                    m_writer->emit("const uint3 start = ");
                    _emitInitAxisValues(groupThreadSize, UnownedStringSlice::fromLiteral("varyingInput->startGroupID"), UnownedStringSlice());

                    if (m_semanticUsedFlags & SemanticUsedFlag::GroupThreadID)
                    {
                        m_writer->emit("context.groupDispatchThreadID = start;\n");
                    }

                    if (m_semanticUsedFlags & SemanticUsedFlag::GroupID)
                    {
                        m_writer->emit("context.groupID = varyingInput->startGroupID;\n");
                    }
                    m_writer->emit("context.dispatchThreadID = start;\n");

                    _emitEntryPointGroup(groupThreadSize, funcName);
                    _emitEntryPointDefinitionEnd(func);
                }

                // Emit the main version - which takes a dispatch size
                {
                    _emitEntryPointDefinitionStart(func, entryPointGlobalParams, funcName, UnownedStringSlice::fromLiteral("ComputeVaryingInput"));

                    m_writer->emit("const uint3 start = ");
                    _emitInitAxisValues(groupThreadSize, UnownedStringSlice::fromLiteral("varyingInput->startGroupID"), UnownedStringSlice());
                    m_writer->emit("const uint3 end = ");
                    _emitInitAxisValues(groupThreadSize, UnownedStringSlice::fromLiteral("varyingInput->endGroupID"), UnownedStringSlice());

                    _emitEntryPointGroupRange(groupThreadSize, funcName);
                    _emitEntryPointDefinitionEnd(func);
                }
            }
        }
    }
}

} // namespace Slang
