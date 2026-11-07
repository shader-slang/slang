// An offline nvcc compile of `-target cuda` output sees only the defines in the source: the
// prelude includes cuda_fp16.h / cuda_bf16.h / cuda_fp8.h and enables its __half,
// __nv_bfloat16 and __nv_fp8_* helpers only under SLANG_CUDA_ENABLE_{HALF,BF16,FP8}, so the
// emitted source defines each one the module needs, and only those, ahead of the prelude
// include (#14037).

//TEST:SIMPLE(filecheck=HALF): -stage compute -entry computeMain -target cuda -DUSE_HALF
//TEST:SIMPLE(filecheck=BF16): -stage compute -entry computeMain -target cuda -DUSE_BF16
//TEST:SIMPLE(filecheck=FP8): -stage compute -entry computeMain -target cuda -DUSE_FP8
//TEST:SIMPLE(filecheck=NONE): -stage compute -entry computeMain -target cuda
// The single-type lanes cannot catch an emitter that writes at most one define per module.
//TEST:SIMPLE(filecheck=MIXED): -stage compute -entry computeMain -target cuda -DUSE_HALF -DUSE_BF16 -DUSE_FP8
// The defines are CUDA-only: they must never leak into the shared C++ base emitter's output.
//TEST:SIMPLE(filecheck=CPP): -stage compute -entry computeMain -target cpp -DUSE_HALF
// On -target ptx the defines reach NVRTC as -D options instead; these lanes are a smoke check
// that NVRTC still compiles each type, and auto-skip on tiers without NVRTC.
//TEST:SIMPLE(filecheck=PTX): -stage compute -entry computeMain -target ptx -DUSE_HALF
//TEST:SIMPLE(filecheck=PTX): -stage compute -entry computeMain -target ptx -DUSE_BF16
//TEST:SIMPLE(filecheck=PTX): -stage compute -entry computeMain -target ptx -DUSE_FP8

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float x = outBuf[tid.x];
    float r = x * 2.0f;
#if defined(USE_HALF)
    r += float(half(x) * half(2));
#endif
#if defined(USE_BF16)
    r += float(BFloat16(x));
#endif
#if defined(USE_FP8)
    r += float(FloatE4M3(x)) + float(FloatE5M2(x));
#endif
    outBuf[tid.x] = r;
}

// Each define must precede the prelude include so the prelude's gates see it. The front
// matter ends at that include, so the -NOT lines on both sides of each define cover all of it.
//HALF-NOT: SLANG_CUDA_ENABLE_{{BF16|FP8}}
//HALF: #define SLANG_CUDA_ENABLE_HALF 1
//HALF-NOT: SLANG_CUDA_ENABLE_{{BF16|FP8}}
//HALF: slang-cuda-prelude.h

//BF16-NOT: SLANG_CUDA_ENABLE_{{HALF|FP8}}
//BF16: #define SLANG_CUDA_ENABLE_BF16 1
//BF16-NOT: SLANG_CUDA_ENABLE_{{HALF|FP8}}
//BF16: slang-cuda-prelude.h

//FP8-NOT: SLANG_CUDA_ENABLE_{{HALF|BF16}}
//FP8: #define SLANG_CUDA_ENABLE_FP8 1
//FP8-NOT: SLANG_CUDA_ENABLE_{{HALF|BF16}}
//FP8: slang-cuda-prelude.h

//MIXED-DAG: #define SLANG_CUDA_ENABLE_HALF 1
//MIXED-DAG: #define SLANG_CUDA_ENABLE_BF16 1
//MIXED-DAG: #define SLANG_CUDA_ENABLE_FP8 1
//MIXED: slang-cuda-prelude.h

// The positive prelude matches keep these lanes from passing on a failed compile.
//NONE-NOT: SLANG_CUDA_ENABLE_{{HALF|BF16|FP8}}
//NONE: slang-cuda-prelude.h

//CPP-NOT: SLANG_CUDA_ENABLE_{{HALF|BF16|FP8}}
//CPP: slang-cpp-prelude.h

//PTX: .visible .entry computeMain
