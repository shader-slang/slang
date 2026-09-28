// https://github.com/shader-slang/slang/issues/12278
// The -target ptx lane is load-bearing: it compiles the emitted CUDA through NVRTC,
// so it fails if the prelude lacks a backing type for SamplerComparisonState.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -entry computeMain -stage compute

// CHECK: struct GlobalParams
// CHECK: SamplerComparisonState g_scmp
// CHECK: Array<SamplerComparisonState> g_scmpArray
// CHECK-PTX: .visible .entry{{.*}}computeMain

SamplerComparisonState g_scmp;
SamplerComparisonState g_scmpArray[];
RWStructuredBuffer<float> g_out;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    g_out[0] = 1.0f;
}
