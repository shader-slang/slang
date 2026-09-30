//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmSqrt'
float oldSqrt(float value)
{
    __intrinsic_asm(nvvmSqrt) "$P_sqrt($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldSqrt(asfloat(words[0])));
}
