//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A read-only by-value uniform struct passed into a helper is marked
// `__grid_constant__ const` on the kernel parameter, and its address is forwarded
// directly into the (borrow-in) helper instead of copying the whole struct into a
// per-thread temporary. Because the helper's lowered parameter is a non-const `T*`,
// the forwarded address is emitted as `const_cast<T*>(&param)`. Fix for #11774.

// The helper is lowered to take the aggregate by pointer.
// CHECK: __device__ {{.*}} readsum_0(Big_0 *{{.*}})

// The kernel parameter is grid-constant, and the call forwards its address with no
// intervening per-thread copy of the struct.
// CHECK: __global__ void computeMain(SLANG_CUDA_GRID_CONSTANT const Big_0{{.*}}big_0
// CHECK-NOT: = big_0;
// CHECK: readsum_0(const_cast<Big_0 *>(&big_0)

struct Big
{
    float v[128];
}

float readsum(Big b, uint i)
{
    return b.v[i] + b.v[i + 1];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = readsum(big, indices[tid.x]);
}
