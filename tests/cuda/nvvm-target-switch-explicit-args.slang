//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute -emit-cuda-via-nvvm -capability nvvm -o -
//TEST:SIMPLE(filecheck=NVVM):-target ptx -entry computeMain -stage compute -emit-cuda-via-nvvm -capability cuda_sm_8_0 -o -

uint readValue()
{
    __target_switch
    {
    // Comma arguments belong to the original intrinsic-assembly contract and remain supported.
    case cuda: __intrinsic_asm "sizeof($[0])", uint;
    case nvvm: __intrinsic_asm "llvm.nvvm.read.ptx.sreg.tid.x";
    }
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output)
{
    output[0] = readValue();
}
// CUDA: sizeof(
// CUDA-NOT: llvm.nvvm
// NVVM: .visible .entry computeMain
// NVVM: %tid.x
