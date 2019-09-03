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


We can stick pointers to these in a structure lets call it 'Context'. On C++ we could make all the functions 'methods', and then
we don't need to pass around the context as a parameter. For C this doesn't work, so it might be worth just biting the bullet and
just adding the context to the output.

Issues:

* How does this work with layout? The layout if it's going to specify offsets will need to know that they will be allocated into each
of these structs AND that the order they are placed needs to be consistent.

* When variables access one of these sources, we will now need code that will add the dereferencing. Hopefully this can be done by looking
at the type of the variable, and then adding the appropriate access via part of emit.

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
    if (m_target == CodeGenTarget::CPPSource)
    {
        // All types are templates in C++
        return;
    }

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
    IRType* type = _cloneType(inType);

    StringSlicePool::Handle handle = StringSlicePool::kNullHandle;
    if (m_typeNameMap.TryGetValue(type, handle))
    {
        return m_slicePool.getSlice(handle);
    }

    if (type->op == kIROp_MatrixType)
    {
        auto matType = static_cast<IRMatrixType*>(type);

        auto elementType = matType->getElementType();
        const auto rowCount = int(GetIntVal(matType->getRowCount()));
        const auto colCount = int(GetIntVal(matType->getColumnCount()));

        // Make sure the vector type the matrix is built on is added
        useType(_getVecType(elementType, colCount));
    }

    StringBuilder builder;
    if (SLANG_SUCCEEDED(_calcTypeName(type, m_target, builder)))
    {
        handle = m_slicePool.add(builder);
    }

    m_typeNameMap.Add(type, handle);

    SLANG_ASSERT(handle != StringSlicePool::kNullHandle);
    return m_slicePool.getSlice(handle);
}

SlangResult CPPSourceEmitter::_calcTextureTypeName(IRTextureTypeBase* texType, StringBuilder& outName)
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

SlangResult CPPSourceEmitter::_calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out)
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
            const IROp elemType = vecType->getElementType()->op;

            if (target == CodeGenTarget::CPPSource)
            {
                out << "Vector<" << getBuiltinTypeName(elemType) << ", " << vecCount << ">";
            }
            else
            {             
                out << "Vec";
                UnownedStringSlice postFix = _getCTypeVecPostFix(elemType);

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

            if (target == CodeGenTarget::CPPSource)
            {
                out << "Matrix<" << getBuiltinTypeName(elementType->op) << ", " << rowCount << ", " << colCount << ">";
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
            SLANG_RETURN_ON_FAIL(_calcTypeName(elementType, target, out));
            out << ", " << elementCount << ">";
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
                    return _calcTextureTypeName(texType, out);   
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
    _getTypeName(type);
}

IRInst* CPPSourceEmitter::_clone(IRInst* inst)
{
    if (inst == nullptr)
    {
        return nullptr;
    }

    IRModule* module = inst->getModule();
    // All inst's must belong to a module
    SLANG_ASSERT(module);

    // If it's in this module then we don't need to clone
    if (module == m_uniqueModule)
    {
        return inst;
    }

    if (IRInst*const* newInstPtr = m_cloneMap.TryGetValue(inst))
    {
        return *newInstPtr;
    }

    if (isNominalOp(inst->op))
    {
        // TODO(JS)
        // This is arguably problematic - I'm adding an instruction from another module to the map, to be it's self.
        // I did have code which created a copy of the nominal instruction and name hint, but because nominality means
        // 'same address' other code would generate a different name for that instruction (say as compared to being a member in
        // the original instruction)
        //
        // Because I use findOrAddInst which doesn't hoist instructions, the hoisting doesn't rely on parenting, that would
        // break.

        // If nominal, we just use the original inst
        m_cloneMap.Add(inst, inst);
        return inst;
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
        case kIROp_StringLit:
        {
            auto stringLit = static_cast<IRStringLit*>(inst);
            clone =  m_irBuilder.getStringValue(stringLit->getStringSlice());
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

                    //clone = m_irBuilder.findOrEmitHoistableInst(cloneType, inst->op, operandCount, cloneOperands.getBuffer());

                    UInt operandCounts[1] = { UInt(operandCount) };
                    IRInst*const* listOperands[1] = { cloneOperands.getBuffer() };

                    clone = m_irBuilder.findOrAddInst(cloneType, inst->op, 1, operandCounts, listOperands);
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
    if (funcName.size() > 0)
    {
        const char c = funcName[0];
        return !((c >= 'a' && c <='z') || (c >= 'A' && c <= 'Z') || c == '_');
    }
    return false;
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

void CPPSourceEmitter::_emitGetAtDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    SourceWriter* writer = getSourceWriter();

    IRFuncType* funcType = specOp.signatureType;
    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(0);

    IRType* retType = specOp.returnType;
    emitType(retType);
    m_writer->emit("& ");

    writer->emit(funcName);
    writer->emit("(");

    emitType(funcType->getParamType(0));
    writer->emit("& a,  ");
    emitType(funcType->getParamType(1));
    writer->emit(" b)\n{\n");

    writer->indent();

    IRVectorType* vectorType = as<IRVectorType>(srcType);
    int vecSize = int(GetIntVal(vectorType->getElementCount()));

    writer->emit("assert(b >= 0 && b < ");
    writer->emit(vecSize);
    writer->emit(");\n");

    writer->emit("return (&a.x)[b];\n");

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

void CPPSourceEmitter::_emitConstructConvertDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    SourceWriter* writer = getSourceWriter();
    IRFuncType* funcType = specOp.signatureType;

    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(1);
    IRType* retType = specOp.returnType;

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

void CPPSourceEmitter::_emitConstructFromScalarDefinition(const UnownedStringSlice& funcName, const SpecializedIntrinsic& specOp)
{
    SourceWriter* writer = getSourceWriter();
    IRFuncType* funcType = specOp.signatureType;

    SLANG_ASSERT(funcType->getParamCount() == 2);

    IRType* srcType = funcType->getParamType(1);
    IRType* retType = specOp.returnType;

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
        case IntrinsicOp::ConstructConvert:
        {
            return _emitConstructConvertDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::ConstructFromScalar:
        {
            return _emitConstructFromScalarDefinition(_getFuncName(specOp), specOp);
        }
        case IntrinsicOp::GetAt:
        {
            return _emitGetAtDefinition(_getFuncName(specOp), specOp);
        }
        default:
        {
            const auto& info = getOperationInfo(specOp.op);
            const int paramCount = (info.numOperands < 0) ? int(specOp.signatureType->getParamCount()) : info.numOperands;

            if (paramCount >= 1 && paramCount <= 3)
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
            // Currently only works for C++ (we use {} constuction) - which means we don't need to generate a function.
            // For C we need to generate suitable construction function
            auto swizzleInst = static_cast<IRSwizzle*>(inst);
            const Index elementCount = Index(swizzleInst->getElementCount());

            // TODO(JS): Not 100% sure this is correct on the parens handling front
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
            SLANG_ASSERT(info.numOperands < 0 || numOperands == info.numOperands);

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
        switch (specOp.op)
        {
            case IntrinsicOp::ConstructConvert:
            {
                // Work out the function name
                IRFuncType* signatureType = specOp.signatureType;
                SLANG_ASSERT(signatureType->getParamCount() == 2);

                IRType* dstType = signatureType->getParamType(0);
                //IRType* srcType = signatureType->getParamType(1);

                StringBuilder builder;
                builder << "convert_";
                // I need a function that is called that will construct this
                if (SLANG_FAILED(_calcTypeName(dstType, CodeGenTarget::CSource, builder)))
                {
                    return StringSlicePool::kNullHandle;
                }
                return m_slicePool.add(builder);
            }
            case IntrinsicOp::ConstructFromScalar:
            {
                // Work out the function name
                IRFuncType* signatureType = specOp.signatureType;
                SLANG_ASSERT(signatureType->getParamCount() == 2);

                IRType* dstType = signatureType->getParamType(0);
                
                StringBuilder builder;
                builder << "constructFromScalar_";
                // I need a function that is called that will construct this
                if (SLANG_FAILED(_calcTypeName(dstType, CodeGenTarget::CSource, builder)))
                {
                    return StringSlicePool::kNullHandle;
                }
                return m_slicePool.add(builder);
            }
            case IntrinsicOp::GetAt:
            {
                return m_slicePool.add(UnownedStringSlice::fromLiteral("getAt"));
            }
            case IntrinsicOp::SetAt:
            {
                return m_slicePool.add(UnownedStringSlice::fromLiteral("setAt"));
            }
            default: break;
        }

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
    switch (op)
    {
        case IntrinsicOp::ConstructFromScalar:
        {
            SLANG_ASSERT(operandCount == 1);
            IRType* dstType = inst->getDataType();
            IRType* srcType = _getElementType(dstType);
            IRType* argTypes[2] = { dstType, srcType };

            SpecializedIntrinsic specOp = getSpecializedOperation(op, argTypes, 2, retType);

            emitCall(specOp, inst, operands, operandCount, inOuterPrec);
            return;
        }
        case IntrinsicOp::ConstructConvert:
        {
            SLANG_ASSERT(inst->getOperandCount() == 1);
            IRType* argTypes[2] = {inst->getDataType(), inst->getOperand(0)->getDataType() };

            SpecializedIntrinsic specOp = getSpecializedOperation(op, argTypes, 2, retType);

            IRFuncType* signatureType = specOp.signatureType;
            SLANG_UNUSED(signatureType);

            SLANG_ASSERT(signatureType->getParamType(0) != signatureType->getParamType(1));

            emitCall(specOp, inst, operands, operandCount, inOuterPrec);
            return;
        }
        default: break;
    }

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

void CPPSourceEmitter::emitSimpleFuncImpl(IRFunc* func)
{
    auto resultType = func->getResultType();

    auto name = getFuncName(func);

    // Deal with decorations that need
    // to be emitted as attributes

    // We are going to ignore the parameters passed and just pass in the Context

    auto entryPointLayout = asEntryPoint(func);
    if (entryPointLayout)
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
        case kIROp_constructVectorFromScalar:
        {
            SLANG_ASSERT(inst->getOperandCount() == 1);
            IRType* dstType = inst->getDataType();

            // Check it's a vector
            SLANG_ASSERT(dstType->op == kIROp_VectorType);
            // Source must be a scalar
            SLANG_ASSERT(as<IRBasicType>(inst->getOperand(0)->getDataType()));

            emitOperationCall(IntrinsicOp::ConstructFromScalar, inst, inst->getOperands(), int(inst->getOperandCount()), dstType, inOuterPrec);
            return true;
        }
        case kIROp_Construct:
        {
            IRType* dstType = inst->getDataType();
            IRType* srcType = inst->getOperand(0)->getDataType();

            if ((dstType->op == kIROp_VectorType || dstType->op == kIROp_MatrixType) &&
                inst->getOperandCount() == 1)
            {
                if (as<IRBasicType>(srcType))
                {
                    emitOperationCall(IntrinsicOp::ConstructFromScalar, inst, inst->getOperands(), int(inst->getOperandCount()), dstType, inOuterPrec);
                }
                else
                {
                    SLANG_ASSERT(_getElementType(dstType) != _getElementType(srcType));
                    // If it's constructed from a type conversion
                    emitOperationCall(IntrinsicOp::ConstructConvert, inst, inst->getOperands(), int(inst->getOperandCount()), dstType, inOuterPrec);
                }
            }
            else
            {
                emitOperationCall(IntrinsicOp::Init, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
            }
            return true;
        }
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
                }
                else
                {
                    SLANG_ASSERT(dstType->op == kIROp_VectorType);
                    emitOperationCall(IntrinsicOp::ConstructFromScalar, inst, inst->getOperands(), 1, dstType, inOuterPrec);
                }
            }
            else
            {
                const Index elementCount = Index(swizzleInst->getElementCount());
                if (elementCount == 1)
                {
                    // If just one thing is extracted then the . syntax will just work 
                    defaultEmitInstExpr(inst, inOuterPrec);
                }
                else
                {
                    // Will need to generate a swizzle method
                    emitOperationCall(IntrinsicOp::Swizzle, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
                }
            }

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
        case kIROp_getElement:
        case kIROp_getElementPtr:
        {
            IRInst* target = inst->getOperand(0);
            if (target->getDataType()->op == kIROp_VectorType)
            {
                // Specially handle this
                emitOperationCall(IntrinsicOp::GetAt, inst, inst->getOperands(), int(inst->getOperandCount()), inst->getDataType(), inOuterPrec);
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

            if (inst->findDecorationImpl(kIROp_EntryPointDecoration))
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
                auto semanticNameSpelling = varLayout->systemValueSemantic;
                if (semanticNameSpelling.getLength())
                {
                    semanticNameSpelling = semanticNameSpelling.toLower();

                    if (semanticNameSpelling == "sv_dispatchthreadid")
                    {
                        
                        m_writer->emit("dispatchThreadID");
                        return;
                    }
                    else if (semanticNameSpelling == "sv_groupid")
                    {
                        m_writer->emit("varyingInput.groupID");
                        return;
                    }
                    else if (semanticNameSpelling == "sv_groupthreadid")
                    {
                        m_writer->emit("varyingInput.groupThreadID");
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

static bool _isVariable(IROp op)
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

struct GlobalParamInfo
{
    typedef GlobalParamInfo ThisType;
    bool operator<(const ThisType& rhs) const { return offset < rhs.offset; }
    bool operator==(const ThisType& rhs) const { return offset == rhs.offset; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs);  }

    IRInst* inst;
    UInt offset;
    UInt size;
};

void CPPSourceEmitter::emitModuleImpl(IRModule* module)
{
    List<EmitAction> actions;
    computeEmitActions(module, actions);

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

    IRGlobalParam* entryPointGlobalParams = nullptr;

    // Output the global parameters in a 'UniformState' structure
    {
        m_writer->emit("struct UniformState\n{\n");
        m_writer->indent();

        List<GlobalParamInfo> params;

        for (auto action : actions)
        {
            if (action.level == EmitAction::Level::Definition && action.inst->op == kIROp_GlobalParam)
            {
                auto inst = action.inst;

                if (inst->findDecorationImpl(kIROp_EntryPointDecoration))
                {
                    // Should only be one instruction marked this way
                    SLANG_ASSERT(entryPointGlobalParams == nullptr);
                    entryPointGlobalParams = as<IRGlobalParam>(inst);
                    continue;
                }

                VarLayout* varLayout = CLikeSourceEmitter::getVarLayout(action.inst);
                SLANG_ASSERT(varLayout);
                const VarLayout::ResourceInfo* varInfo = varLayout->FindResourceInfo(LayoutResourceKind::Uniform);
                TypeLayout* typeLayout = varLayout->getTypeLayout();
                TypeLayout::ResourceInfo* typeInfo = typeLayout->FindResourceInfo(LayoutResourceKind::Uniform);

                GlobalParamInfo paramInfo;
                paramInfo.inst = action.inst;
                // Index is the byte offset for uniform
                paramInfo.offset = varInfo ? varInfo->index : 0;
                paramInfo.size = typeInfo ? typeInfo->count.raw : 0;

                params.add(paramInfo);
            }
        }

        // We want to sort by layout offset, and insert suitable padding
        params.sort();

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
        m_writer->dedent();
        m_writer->emit("\n};\n\n");
    }

    // Output the 'Context' which will be used for execution
    {
        m_writer->emit("struct Context\n{\n");
        m_writer->indent();

        m_writer->emit("UniformState* uniformState;\n");
        m_writer->emit("ComputeVaryingInput varyingInput;\n");
        m_writer->emit("uint3 dispatchThreadID;\n");

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

            auto entryPointLayout = asEntryPoint(func);
            if (entryPointLayout)
            {
                auto resultType = func->getResultType();
                auto name = getFuncName(func);

                // Emit the actual function
                emitEntryPointAttributes(func, entryPointLayout);
                emitType(resultType, name);

                m_writer->emit("(ComputeVaryingInput* varyingInput, UniformEntryPointParams* params, UniformState* uniformState)\n{\n");
                emitSemantics(func);

                m_writer->indent();
                // Initialize when constructing so that globals are zeroed
                m_writer->emit("Context context = {};\n");
                m_writer->emit("context.uniformState = uniformState;\n");
                m_writer->emit("context.varyingInput = *varyingInput;\n");

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
                
                // Emit dispatchThreadID
                if (entryPointLayout->profile.GetStage() == Stage::Compute)
                {
                    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sv-dispatchthreadid
                    // SV_DispatchThreadID is the sum of SV_GroupID * numthreads and GroupThreadID.

                    static const UInt kAxisCount = 3;
                    UInt sizeAlongAxis[kAxisCount];

                    // TODO: this is kind of gross because we are using a public
                    // reflection API function, rather than some kind of internal
                    // utility it forwards to...
                    spReflectionEntryPoint_getComputeThreadGroupSize((SlangReflectionEntryPoint*)entryPointLayout, kAxisCount, &sizeAlongAxis[0]);

                    m_writer->emit("context.dispatchThreadID = {\n");
                    m_writer->indent();

                    StringBuilder builder;
                    
                    for (int i = 0; i < kAxisCount; ++i)
                    {
                        builder.Clear();
                        const char elem[2] = {s_elemNames[i], 0};
                        builder << "varyingInput->groupID." << elem << " * " << sizeAlongAxis[i] << " + varyingInput->groupThreadID." << elem;
                        if (i < kAxisCount - 1)
                        {
                            builder << ",";
                        }
                        builder << "\n";
                        m_writer->emit(builder);
                    }

                    m_writer->dedent();
                    m_writer->emit("};\n");
                }

                m_writer->emit("context._");
                m_writer->emit(name);
                m_writer->emit("();\n");
                m_writer->dedent();
                m_writer->emit("}\n");
            }
        }
    }
}

} // namespace Slang
