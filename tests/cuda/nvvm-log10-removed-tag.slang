//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmLog10'
float oldLog10(float value)
{
    __intrinsic_asm(nvvmLog10) "$P_log10($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldLog10(asfloat(words[0])));
}
