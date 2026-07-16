//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A by-value uniform struct is forwarded by address into a borrow-in helper instead of copied into
// a per-thread temporary (the #11774 slowdown).

// CHECK: __device__ {{.*}} readsum_0(Big_0 *{{.*}})
// CHECK: __global__ void computeMain(Big_0 [[BIG:big_0]], StructuredBuffer<uint>

// No whole-struct copy spilled before the forward; anchored on the type so reformatted spills trip it.
// CHECK-NOT: Big_0 {{.*}}= [[BIG]]
// CHECK: readsum_0((&([[BIG]]))

struct Big
{
    float v[128];
}

// [noinline] keeps the kernel->helper boundary so the struct is forwarded, not inlined.
[noinline]
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
