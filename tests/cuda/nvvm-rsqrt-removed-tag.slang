//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmRsqrt'
float oldRsqrt(float value)
{
    __intrinsic_asm(nvvmRsqrt) "$P_rsqrt($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldRsqrt(asfloat(words[0])));
}
