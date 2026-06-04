//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Each text-emit backend in the dispatcher emits a target-distinctive entry-point marker for a `[shader("compute")]` function with `[numthreads(16, 1, 1)]`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Backends section enumerates the per-target emitters. Each
// translates a compute entry point into its own marker:
//   HLSL  : `[numthreads(16, 1, 1)]`
//   GLSL  : `layout(local_size_x = 16, ...)`
//   SPIRV : `OpEntryPoint GLCompute` + `OpExecutionMode ... LocalSize 16 1 1`
//   Metal : `kernel` + `thread_position_in_grid`
//   WGSL  : `@compute` + `@workgroup_size(16, 1, 1)`
//   CUDA  : `extern "C" __global__`
//   C++   : `SLANG_PRELUDE_EXPORT`-wrapped `main_0`
// CHECK patterns are bare substrings to avoid FileCheck's `[[ ]]`
// variable syntax clashing with HLSL/Metal attribute brackets.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(16, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: numthreads(16, 1, 1)
// GLSL: layout(local_size_x = 16, local_size_y = 1, local_size_z = 1)
// SPIRV: OpEntryPoint GLCompute %main "main"
// SPIRV: OpExecutionMode %main LocalSize 16 1 1
// METAL: kernel{{.*}}main_{{[0-9]+}}({{.*}}thread_position_in_grid
// WGSL: @compute
// WGSL: @workgroup_size(16, 1, 1)
// CUDA: extern "C" __global__ void main_{{[0-9]+}}()
// CPP: SLANG_PRELUDE_EXPORT
