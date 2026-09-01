// A function marked with BOTH `[ForceInline]` and `[noinline]` must emit a single,
// non-contradictory CUDA specifier sequence: `__forceinline__ __device__ __noinline__` is a
// contradiction NVRTC rejects. `[ForceInline]` takes precedence — the emitted function carries
// `__forceinline__` and not `__noinline__` (shader-slang/slang#12623).

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

RWStructuredBuffer<int> outputBuffer;

// The combined function is emitted with `__forceinline__` and no `__noinline__`.
// CHECK-DAG: {{^}}__forceinline__ __device__ int combinedHelper
// CHECK-NOT: __noinline__ int combinedHelper
[ForceInline]
[noinline]
int combinedHelper(int x)
{
    return x + 1;
}

// A plain `[noinline]` function is unaffected: it still gets `__noinline__` and no forceinline.
// CHECK-DAG: {{^}}__device__ __noinline__ int noinlineHelper
[noinline]
int noinlineHelper(int x)
{
    return x * 2;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Call each more than once so neither is trivially inlined away before emit.
    outputBuffer[0] = combinedHelper(int(tid.x)) + combinedHelper(7);
    outputBuffer[1] = noinlineHelper(int(tid.x)) + noinlineHelper(9);
}
