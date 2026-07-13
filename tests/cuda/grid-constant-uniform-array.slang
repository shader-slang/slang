//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A by-value uniform array parameter must be emitted as a `__grid_constant__ const` kernel
// parameter so every thread reads it directly from the kernel's `.param`/constant space, rather
// than each thread first copying the whole array into its per-thread `.local` stack frame. When the
// array is forwarded into a helper, its address is passed directly (`const_cast<...>(&tensors_0)`)
// instead of spilling a per-thread temporary copy. This is the fix for the CUDA-only slowdown in
// shader-slang/slang#11774, for the array (not just struct) aggregate case.

// [noinline] keeps the kernel->helper boundary so the array's address is forwarded into a
// borrow-in pointer parameter rather than the helper being inlined and the array read in place.
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

// The helper takes the array by pointer.
// CHECK: gather{{[A-Za-z0-9_]*}}(FixedArray<float, 256>{{.*}}*

// The entry parameter carries the grid-constant qualifier (SLANG_CUDA_GRID_CONSTANT expands to
// `__grid_constant__` on CUDA 11.7+ via the prelude).
// CHECK: __global__ void computeMain(SLANG_CUDA_GRID_CONSTANT const FixedArray<float, 256>{{.*}}[[TENSORS:tensors_[0-9]+]]

// No spilled per-thread copy of the array is materialized in the kernel-body prologue before the
// forward (the pre-fix path emitted `FixedArray<float, 256> _Sx = tensors_1;` there). This
// `CHECK-NOT` sits between the signature and the call-site `CHECK` below so its scan window covers
// exactly that prologue region.
// CHECK-NOT: = [[TENSORS]];

// The call site forwards the entry array's address directly.
// CHECK: gather{{[A-Za-z0-9_]*}}(const_cast<FixedArray<float, 256>{{.*}}>(&[[TENSORS]])
