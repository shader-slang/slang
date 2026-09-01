// Emit-level check (no GPU): the wrapper redirect lives in the #include'd
// prelude, so slangc's output carries the define, not the expanded `__cosf`.

//TEST:SIMPLE(filecheck=FAST): -stage compute -entry computeMain -target cuda -fp-mode fast
//TEST:SIMPLE(filecheck=DEFAULT): -stage compute -entry computeMain -target cuda
// The define is CUDA-only: it must never leak into the shared C++ base emitter's output.
//TEST:SIMPLE(filecheck=CPP): -stage compute -entry computeMain -target cpp -fp-mode fast
// Behavioral lanes: compile the emitted CUDA through NVRTC to PTX and confirm the redirect
// is actually *selected* (fast -> approximate SFU ops; default -> none of them). These
// auto-skip on tiers without NVRTC, like the -target ptx arm in tests/cuda/noinline.slang.
//TEST:SIMPLE(filecheck=PTX_FAST): -stage compute -entry computeMain -target ptx -fp-mode fast
//TEST:SIMPLE(filecheck=PTX_PRECISE): -stage compute -entry computeMain -target ptx

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float x = outBuf[0];
    outBuf[0] = cos(x) + sin(x) + exp(x) + pow(x, 2.0f) + log2(x);
}

// The define must precede the prelude include so the prelude's #ifdef sees it.
//FAST: #define SLANG_CUDA_ENABLE_FAST_MATH
//FAST: slang-cuda-prelude.h

//DEFAULT-NOT: SLANG_CUDA_ENABLE_FAST_MATH

//CPP-NOT: SLANG_CUDA_ENABLE_FAST_MATH

// Under fast math, sin/cos/log(2) lower to the approximate SFU ops (from __sinf/__cosf/__log2f).
//PTX_FAST-DAG: sin.approx
//PTX_FAST-DAG: cos.approx
//PTX_FAST-DAG: lg2.approx

// By default they must not — the precise wrappers do not emit these ops.
//PTX_PRECISE-NOT: sin.approx
//PTX_PRECISE-NOT: cos.approx
//PTX_PRECISE-NOT: lg2.approx
