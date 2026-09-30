//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmTrunc'
float oldTrunc(float value)
{
    __intrinsic_asm(nvvmTrunc) "$P_trunc($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldTrunc(asfloat(words[0])));
}
