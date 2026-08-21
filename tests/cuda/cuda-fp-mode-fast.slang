// Emit-level check (no GPU): the wrapper redirect lives in the #include'd
// prelude, so slangc's output carries the define, not the expanded `__cosf`.

//TEST:SIMPLE(filecheck=FAST): -stage compute -entry computeMain -target cuda -fp-mode fast
//TEST:SIMPLE(filecheck=DEFAULT): -stage compute -entry computeMain -target cuda

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
