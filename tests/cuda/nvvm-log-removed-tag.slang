//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// CHECK: unknown intrinsic assembly semantic 'nvvmLog'
float oldLog(float value)
{
    __intrinsic_asm(nvvmLog) "$P_log($0)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> words)
{
    words[1] = asuint(oldLog(asfloat(words[0])));
}
