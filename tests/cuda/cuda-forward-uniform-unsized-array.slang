//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A uniform unsized array passed to a helper that takes it by borrow is passed by address, like a
// sized array. The kernel declares it by value as `Array<float>`.

// [noinline] keeps the kernel->helper boundary so the array is forwarded, not inlined.
[noinline]
float gather(float tensors[], uint i)
{
    return tensors[i];
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform float tensors[],
    uniform RWStructuredBuffer<float> output)
{
    output[tid] = gather(tensors, tid);
}

// CHECK: gather{{[A-Za-z0-9_]*}}(Array<float>{{.*}}*
// CHECK: __global__ void computeMain(Array<float>{{ +}}[[TENSORS:tensors_[0-9]+]], RWStructuredBuffer<float>

// CHECK-NOT: Array<float>{{.*}}= [[TENSORS]]
// CHECK: gather{{[A-Za-z0-9_]*}}(&[[TENSORS]]
