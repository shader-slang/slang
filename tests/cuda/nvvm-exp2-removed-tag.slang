//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmExp2'
float oldExp2(float value)
{
    __intrinsic_asm(nvvmExp2) "$P_exp2($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldExp2(asfloat(words[0])));
}
