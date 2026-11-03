// Companion regression test for shader-slang/slang#13107.
//
// A `static const` struct table can stay a global constant on most targets. CUDA is
// different: NVRTC rejects dynamic initialization of a `__device__` variable, and a
// `half` literal is emitted as a `__half(...)` constructor call, which is a dynamic
// initializer. A struct, array, or vector that contains `half` must therefore still be
// inlined into the functions that use it when the target is CUDA.
//
// SlangPy's `test_print` hit this: `print` packs the `half` arguments into a tuple
// constant, and NVRTC failed with "dynamic initialization is not supported for a
// __device__ variable".

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none

struct HalfPair
{
    half a;
    half b;
}

static const HalfPair kPairs[2] = { { half(1.5), half(2.5) }, { half(3.5), half(4.5) } };
static const half4 kVec = half4(1.0h, 2.0h, 3.0h, 4.0h);

RWStructuredBuffer<float> outputBuffer;

// The tables must not become `__device__ static const` globals. They are built in the
// kernel instead.
// CUDA-NOT: __device__ static const
// CUDA: __global__ void computeMain
// CUDA: kPairs{{.*}} = {

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int i = int(tid.x);
    outputBuffer[0] = float(kPairs[i].a) + float(kPairs[i].b) + float(kVec[i]);
}
