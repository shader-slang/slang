// https://github.com/shader-slang/slang/issues/12278
// The -target ptx lane is load-bearing: it compiles the emitted CUDA through NVRTC,
// so it fails if the prelude lacks a backing type for SamplerComparisonState.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -emit-cuda-via-nvrtc -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-NVVM): -target ptx -emit-cuda-via-nvvm -capability cuda_sm_7_0 -entry computeMain -stage compute -o -

// CHECK: struct GlobalParams
// CHECK: SamplerComparisonState g_scmp
// CHECK: Array<SamplerComparisonState> g_scmpArray
// CHECK-PTX: .visible .entry{{.*}}computeMain
// CHECK-NVVM: .const .align 8 .b8 SLANG_globalParams[40]
// CHECK-NVVM: .visible .entry computeMain()
// CHECK-NVVM: ld.const.u64 {{.*}}[SLANG_globalParams+8]
// CHECK-NVVM: st.global.u32

SamplerComparisonState g_scmp;
SamplerComparisonState g_scmpArray[];
RWStructuredBuffer<float> g_out;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    g_out[0] = 1.0f;
}
