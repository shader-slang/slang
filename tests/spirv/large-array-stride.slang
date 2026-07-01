//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Check that explicit layout lowering preserves a large SPIR-V ArrayStride
// unsigned 32-bit literal instead of interpreting it as a signed integer or
// truncating it.
// The large array count is only used to produce a stride larger than the
// maximum 32-bit signed integer; this test is not intended to cover maximum
// array element count support.

// CHECK-DAG: OpDecorate %[[PTR:_ptr_StorageBuffer_LargeStride_std430[A-Za-z0-9_]*]] ArrayStride 4294967292
// CHECK-DAG: OpDecorate %[[RUNTIME:_runtimearr_LargeStride_std430[A-Za-z0-9_]*]] ArrayStride 4294967292

struct LargeStride
{
    float a[0x3fffffff];
};

RWStructuredBuffer<LargeStride> buffer;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = buffer[0].a[0];
}
