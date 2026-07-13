//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Mutation safety for the `__grid_constant__ const` qualifier: even when a by-value uniform
// aggregate is "mutated", Slang's value semantics require the write to go through a local copy
// first (a by-value parameter is not an l-value), so the kernel parameter itself is never written
// in place. The emitted parameter therefore stays `__grid_constant__ const` soundly, and the
// mutation lands on a distinct local. This guards against a regression where the qualifier would
// be emitted on a parameter that is actually written through, which would be a CUDA compile error.

struct Big
{
    float v[128];
}

// A helper that mutates its by-value aggregate parameter. The lowered helper copies the
// pointed-to aggregate into a local before writing, so the forwarded address is only read.
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

// The helper takes the aggregate by pointer and, before any write, copies the pointed-to value
// into a local; the write below therefore lands on the local, never on the forwarded
// (grid-constant) pointer.
// CHECK: mutate_and_read_0(Big_0 * b_0
// CHECK: Big_0 {{.*}} = *b_0;

// The kernel parameter is emitted grid-constant and const.
// CHECK: __global__ void computeMain(SLANG_CUDA_GRID_CONSTANT const Big_0{{.*}}big_0

// The call site forwards the parameter's address directly (`const_cast<Big_0 *>(&big_0)`), proving
// the address-forwarding path fired rather than spilling `big_0` into a per-thread temporary copy.
// CHECK: mutate_and_read_0(const_cast<Big_0 *>(&big_0)
