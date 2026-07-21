//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A by-value uniform array is forwarded by address into a borrow-in helper instead of copied into a
// per-thread temporary (the #11774 slowdown). Array counterpart of the struct-forward test.

// [noinline] keeps the kernel->helper boundary so the array is forwarded, not inlined.
[noinline]
float gather(float tensors[256], uint i)
{
    return tensors[i] + tensors[i + 1];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform float tensors[256],
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = gather(tensors, indices[tid.x]);
}

// CHECK: gather{{[A-Za-z0-9_]*}}(FixedArray<float, 256>{{.*}}*
// CHECK: __global__ void computeMain(FixedArray<float, 256>{{.*}}[[TENSORS:tensors_[0-9]+]], StructuredBuffer<uint>

// No whole-array copy spilled before the forward; anchored on the type so reformatted spills trip it.
// CHECK-NOT: FixedArray<float, 256>{{.*}}= [[TENSORS]]
// CHECK: gather{{[A-Za-z0-9_]*}}(&[[TENSORS]]
