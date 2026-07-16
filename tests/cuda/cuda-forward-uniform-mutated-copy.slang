//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Mutation-path emit shape: a helper that "mutates" its by-value uniform aggregate. The callee
// copies the pointee (`*b_0`) into a local before writing, so the forwarded address is only read -
// which is why the forward is sound with no qualifier or const_cast.

struct Big
{
    float v[128];
}

[noinline]
float mutate_and_read(Big b, uint i)
{
    b.v[0] = 7.0f;
    return b.v[i];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = mutate_and_read(big, indices[tid.x]);
}

// Helper copies the pointee into a local before the write, so the write never reaches the pointer.
// CHECK: mutate_and_read_0(Big_0 * b_0
// CHECK: Big_0 {{.*}} = *b_0;
// CHECK: __global__ void computeMain(Big_0 big_0, StructuredBuffer<uint>
// CHECK: mutate_and_read_0((&(big_0))
