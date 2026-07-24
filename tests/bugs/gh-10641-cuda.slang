// Regression test for the CUDA and Metal handling of a bare `groupshared` array parameter
// (https://github.com/shader-slang/slang/issues/10641).
//
// A bare `groupshared` array parameter is passed by reference, which lowers to a pointer into the
// group-shared address space. This test pins the target-specific spelling of that pointer. On the
// CUDA target it is a plain pointer parameter (`T*`) with no parameter-level `__shared__`
// qualifier -- the `__shared__` storage stays on the caller's global -- and `[noinline]` must be
// honored. On the Metal target it is a `threadgroup`-qualified pointer (`T threadgroup*`). The
// `-target ptx` lane additionally compiles the emitted CUDA through NVRTC, which rejects member
// access on a pointer -- the failure mode when the by-reference parameter is mistaken for a
// by-value array.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -capability cuda_sm_7_0
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

static const uint N = 8;
groupshared uint groupSharedData[N];

// A `[noinline]` bare group-shared parameter must survive as an ordinary by-reference (pointer)
// parameter, with no `__shared__` qualifier on the parameter itself and no forced inlining. Its
// `__shared__`-ness stays on the caller's global, never on the parameter.
// CUDA: void writeSlot{{.*}}({{.*}}* scratch
// CUDA-NOT: __shared__ {{[A-Za-z0-9_<>, ]*}}scratch
//
// The same by-reference group-shared pointer is spelled `threadgroup*` on Metal.
// METAL: void writeSlot{{.*}}(uint {{[A-Za-z0-9_]+}}, {{.*}}threadgroup* scratch
//
// NVRTC accepts the emitted CUDA and produces PTX for the entry point.
// PTX: computeMain
[noinline]
void writeSlot(uint tid, groupshared uint scratch[N])
{
    scratch[tid] = tid + 1;
}

[shader("compute")]
[numthreads(N, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    writeSlot(tid.x, groupSharedData);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid.x] = groupSharedData[(tid.x + 1) % N];
}
