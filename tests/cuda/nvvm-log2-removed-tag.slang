//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmLog2'
float oldLog2(float value)
{
    __intrinsic_asm(nvvmLog2) "$P_log2($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldLog2(asfloat(words[0])));
}
