// Under `-fp-mode fast`, the CUDA emitter must emit a
// `#define SLANG_CUDA_ENABLE_FAST_MATH` in front matter (before the prelude
// #include) so the prelude redirects redirectable transcendentals to their
// hardware-approximate `__*f` intrinsics. Without it, the emit is unchanged.
// This is an emit-level check (no GPU): the redirect itself lives in the
// #include'd prelude, so slangc's output carries the define, not `__cosf`.

//TEST:SIMPLE(filecheck=FAST): -stage compute -entry computeMain -target cuda -fp-mode fast
//TEST:SIMPLE(filecheck=DEFAULT): -stage compute -entry computeMain -target cuda

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float x = outBuf[0];
    outBuf[0] = cos(x) + sin(x) + exp(x) + pow(x, 2.0f) + log2(x);
}

// The define must appear, and must precede the prelude include so it is visible
// to the prelude's #ifdef.
//FAST: #define SLANG_CUDA_ENABLE_FAST_MATH
//FAST: slang-cuda-prelude.h

// By default the define must not be emitted.
//DEFAULT-NOT: SLANG_CUDA_ENABLE_FAST_MATH
