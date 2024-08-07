#include "slang-emit-wgsl.h"

namespace Slang {

void WGSLSourceEmitter::emitParameterGroupImpl(IRGlobalParam* /* varDecl */, IRUniformParameterGroupType* /* type */)
{
    // TODO: Implement
    SLANG_ASSERT(false);
}

void WGSLSourceEmitter::emitEntryPointAttributesImpl(IRFunc* irFunc, IREntryPointDecoration* entryPointDecor)
{
    auto stage = entryPointDecor->getProfile().getStage();

    switch (stage)
    {

    case Stage::Compute:
    {
        m_writer->emit("@compute\n");

        {
            Int sizeAlongAxis[kThreadGroupAxisCount];
            getComputeThreadGroupSize(irFunc, sizeAlongAxis);

            m_writer->emit("@workgroup_size(");
            for (int ii = 0; ii < kThreadGroupAxisCount; ++ii)
                {
                    if (ii != 0) m_writer->emit(", ");
                    m_writer->emit(sizeAlongAxis[ii]);
                }
            m_writer->emit(")\n");
        }
    }
    break;

    default:
        SLANG_ABORT_COMPILATION("unsupported stage.");
    }

}

void WGSLSourceEmitter::emitSimpleTypeImpl(IRType* type)
{
    switch (type->getOp())
    {
        case kIROp_VoidType:
            // There is no void type in WGSL.
            // A return type of "void" is expressed by skipping the end part of the 'function_header' term:
            // "
            // function_header :
            //   'fn' ident '(' param_list ? ')' ( '->' attribute * template_elaborated_ident ) ?
            // "
            // In other words, in WGSL we should never even get to the point where we're asking to emit 
            SLANG_UNEXPECTED("'void' type emitted");
            return;
        case kIROp_BoolType:
        case kIROp_Int8Type:
        case kIROp_IntType:
        case kIROp_Int64Type:
        case kIROp_UInt8Type:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_FloatType:
        case kIROp_DoubleType:
        case kIROp_HalfType:
        {
            m_writer->emit(getDefaultBuiltinTypeName(type->getOp()));
            return;
        }
        case kIROp_Int16Type:
            m_writer->emit("short");
            return;
        case kIROp_UInt16Type:
            m_writer->emit("ushort");
            return;
        case kIROp_IntPtrType:
            m_writer->emit("int64_t");
            return;
        case kIROp_UIntPtrType:
            m_writer->emit("uint64_t");
            return;
        case kIROp_StructType:
            m_writer->emit(getName(type));
            return;

        case kIROp_VectorType:
        {
            auto vecType = (IRVectorType*)type;
            emitVectorTypeNameImpl(vecType->getElementType(), getIntVal(vecType->getElementCount()));
            return;
        }
        case kIROp_MatrixType:
        {
            auto matType = (IRMatrixType*)type;

            // Similar to GLSL, Metal's column-major is really our row-major.
            m_writer->emit("matrix<");
            emitType(matType->getElementType());
            m_writer->emit(",");
            emitVal(matType->getRowCount(), getInfo(EmitOp::General));
            m_writer->emit(",");
            emitVal(matType->getColumnCount(), getInfo(EmitOp::General));
            m_writer->emit("> ");           
            return;
        }
        case kIROp_SamplerStateType:
        case kIROp_SamplerComparisonStateType:
        {
            m_writer->emit("sampler");
            return;
        }
        case kIROp_NativeStringType:
        case kIROp_StringType: 
        {
            m_writer->emit("int"); 
            return;
        }
        case kIROp_ParameterBlockType:
        case kIROp_ConstantBufferType:
        {
            emitSimpleTypeImpl((IRType*)type->getOperand(0));
            m_writer->emit(" constant*");
            return;
        }
        case kIROp_PtrType:
        case kIROp_InOutType:
        case kIROp_OutType:
        case kIROp_RefType:
        case kIROp_ConstRefType:
        {
            auto ptrType = cast<IRPtrTypeBase>(type);
            emitType((IRType*)ptrType->getValueType());
            switch ((AddressSpace)ptrType->getAddressSpace())
            {
            case AddressSpace::Global:
                m_writer->emit(" device");
                m_writer->emit("*");
                break;
            case AddressSpace::Uniform:
                m_writer->emit(" constant");
                m_writer->emit("*");
                break;
            case AddressSpace::ThreadLocal:
                m_writer->emit(" thread");
                m_writer->emit("*");
                break;
            case AddressSpace::GroupShared:
                m_writer->emit(" threadgroup");
                m_writer->emit("*");
                break;
            case AddressSpace::MetalObjectData:
                m_writer->emit(" object_data");
                // object data is passed by reference
                m_writer->emit("&");
                break;
            }
            return;
        }
        case kIROp_ArrayType:
        {
            m_writer->emit("array<");
            emitType((IRType*)type->getOperand(0));
            m_writer->emit(", ");
            emitVal(type->getOperand(1), getInfo(EmitOp::General));
            m_writer->emit(">");
            return;
        }
        case kIROp_MetalMeshGridPropertiesType:
        {
            m_writer->emit("mesh_grid_properties ");
            return;
        }
        default:
            break;
    }

}

void WGSLSourceEmitter::emitVectorTypeNameImpl(IRType* /* elementType */, IRIntegerValue /* elementCount */)
{
    // TODO: Implement
    SLANG_ASSERT(false);
}

// TODO: Remove?
void WGSLSourceEmitter::emitGlobalInstImpl(IRInst* inst)
{
    CLikeSourceEmitter::emitGlobalInstImpl(inst);
}    
    
} // namespace Slang
