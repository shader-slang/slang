//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv-asm

// Ensure SPIRV emits Aligned operand for OpStore to PhysicalStorageBuffer pointers
// SPIRV: OpStore %{{[0-9]+}} %{{[0-9]+}} Aligned

struct Foo
{
    uint x, y, z, w;
};

[numthreads(1,1,1)]
void computeMain(uniform Foo* src, uniform Foo* dst, uint tid: SV_DispatchThreadID)
{
    dst[tid] = src[tid];
}
