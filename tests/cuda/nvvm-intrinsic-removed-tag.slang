//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute
// The removed semantic spelling is diagnosed at the existing semantic-name lookup.
// CHECK: unknown intrinsic assembly semantic 'nvvmThreadIndex'
uint3 oldThreadIndex()
{
    __intrinsic_asm(nvvmThreadIndex) "(threadIdx)";
}
[CUDAKernel]
void computeMain(uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output)
{
    output[0] = oldThreadIndex().x;
}
