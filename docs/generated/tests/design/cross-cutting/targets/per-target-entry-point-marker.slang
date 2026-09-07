//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=1103b6c6793a74ba9a2a797712002157cc506284940af6ab698172cadfb6ec8a
//META: purpose=Target choice surfaces through target-specific lowering passes; a `[shader("compute")]` entry point emits the target-shaped compute marker on each text-emit backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "How target choice affects IR" section lists target-
// specific legalization passes (slang-ir-hlsl-legalize,
// slang-ir-glsl-legalize, slang-ir-metal-legalize, ...). The
// observable consequence is that a compute entry point emerges with
// a target-shaped marker: HLSL `numthreads(...)`, GLSL
// `layout(local_size_x = ...)`, SPIR-V `OpExecutionMode LocalSize`,
// Metal `kernel` + `thread_position_in_grid`, WGSL
// `@workgroup_size(...)`, CUDA `extern "C" __global__`, CPP
// `SLANG_PRELUDE_EXPORT`. CHECK patterns use bare substrings to
// avoid FileCheck's `[[ ]]` variable syntax clashing with Metal's
// attribute brackets.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: numthreads(8, 1, 1)
// GLSL: local_size_x = 8
// SPIRV: OpExecutionMode %main LocalSize 8 1 1
// METAL: kernel{{.*}}main_{{[0-9]+}}({{.*}}thread_position_in_grid
// WGSL: @workgroup_size(8, 1, 1)
// CUDA: extern "C" __global__ void main_{{[0-9]+}}()
// CPP: SLANG_PRELUDE_EXPORT
