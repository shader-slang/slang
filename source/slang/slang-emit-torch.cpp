// slang-emit-torch.cpp
#include "slang-emit-torch.h"

#include "../core/slang-writer.h"

#include "slang-emit-source-writer.h"
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang
{
bool TorchCppSourceEmitter::tryEmitInstExprImpl(IRInst* inst, const EmitOpInfo& inOuterPrec)
{
    switch (inst->getOp())
    {
    default:
    {
        return Super::tryEmitInstExprImpl(inst, inOuterPrec);
    }
    case kIROp_MakeTensorView:
    {
        m_writer->emit("make_tensor_view(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(", ");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit(")");
        return true;
    }
    case kIROp_CudaKernelLaunch:
    {
        m_writer->emit("cudaLaunchKernel(");
        // func
        m_writer->emit("(const void*)(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit("), ");

        // gridDim
        m_writer->emit("slang_bit_cast<dim3>(");
        emitOperand(inst->getOperand(1), getInfo(EmitOp::General));
        m_writer->emit("), ");

        // blockDim
        m_writer->emit("slang_bit_cast<dim3>(");
        emitOperand(inst->getOperand(2), getInfo(EmitOp::General));
        m_writer->emit("), ");

        // args
        emitOperand(inst->getOperand(3), getInfo(EmitOp::General));
        m_writer->emit(", ");

        // shared mem
        m_writer->emit("slangGetCudaKernelSharedMemSize((const void*)(");
        emitOperand(inst->getOperand(0), getInfo(EmitOp::General));
        m_writer->emit(")), ");

        // stream
        m_writer->emit("((cudaStream_t)");
        emitOperand(inst->getOperand(4), getInfo(EmitOp::General));
        m_writer->emit("))");
        return true;
    }
    case kIROp_TorchGetCudaStream:
    {
        m_writer->emit("at::cuda::getCurrentCUDAStream()");
        return true;
    }
    case kIROp_AllocateTorchTensor:
    {
        /*
        Emit something like:
            ```
              torch::Tensor out = torch::empty({ dimX, dimY, dimZ, ... },
                torch::TensorOptions().device(torch::kCUDA).dtype(torch::kFloat32));
            ```
        */
        m_writer->emit("torch::empty({ ");
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            if (i > 0)
                m_writer->emit(", ");
            auto arg = inst->getOperand(i);
            emitOperand(arg, getInfo(EmitOp::General));
        }
        m_writer->emit("}, torch::TensorOptions().device(torch::kCUDA).dtype(torch::");

        // Get the element type of the tensor.
        auto instType = as<IRTorchTensorType>(inst->getDataType())->getOperand(0);

        // If instType is a vector type, then we need to get the element type.
        if (auto vectorType = as<IRVectorType>(instType))
        {
            instType = vectorType->getElementType();
        }

        switch (instType->getOp())
        {
        case kIROp_FloatType:
            m_writer->emit("kFloat32");
            break;
        case kIROp_HalfType:
            m_writer->emit("kFloat16");
            break;
        case kIROp_DoubleType:
            m_writer->emit("kFloat64");
            break;
        case kIROp_UInt8Type:
            m_writer->emit("kUInt8");
            break;
        case kIROp_UInt16Type:
            m_writer->emit("kUInt16");
            break;
        case kIROp_UIntType:
            m_writer->emit("kUInt32");
            break;
        case kIROp_UInt64Type:
            m_writer->emit("kUInt64");
            break;
        case kIROp_Int8Type:
            m_writer->emit("kInt8");
            break;
        case kIROp_Int16Type:
            m_writer->emit("kInt16");
            break;
        case kIROp_IntType:
            m_writer->emit("kInt32");
            break;
        case kIROp_Int64Type:
            m_writer->emit("kInt64");
            break;
        default:
            SLANG_UNEXPECTED("unknown scalar type in allocTorchTensor");
            break;
        }
        m_writer->emit("))");
        return true;
    }
    }
}

SlangResult TorchCppSourceEmitter::calcTypeName(IRType* type, CodeGenTarget target, StringBuilder& out)
{
    switch (type->getOp())
    {
    default:
        return Super::calcTypeName(type, target, out);
    case kIROp_TensorViewType:
    {
        out << "TensorView";
        return SLANG_OK;
    }
    case kIROp_TorchTensorType:
    {
        out << "torch::Tensor";
        return SLANG_OK;
    }
    case kIROp_TorchKernelMemoryAllocatorType:
    {
        out << "CudaTaskMemoryAllocator";
        return SLANG_OK;
    }
    }
}

void TorchCppSourceEmitter::emitModuleImpl(IRModule* module, DiagnosticSink* sink)
{
    Super::emitModuleImpl(module, sink);

    // Emit PyBind declarations.
    m_writer->emit("PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {\n");
    m_writer->indent();
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func) continue;
        auto decor = func->findDecoration<IRTorchEntryPointDecoration>();
        if (!decor) continue;
        m_writer->emit("m.def(");
        emitStringLiteral(decor->getFunctionName());
        m_writer->emit(", &");
        m_writer->emit(decor->getFunctionName());
        m_writer->emit(", ");
        emitStringLiteral(decor->getFunctionName());
        m_writer->emit(");\n");
    }
    m_writer->dedent();
    m_writer->emit("}\n");

}

} // namespace Slang
