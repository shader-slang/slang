//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=7d2dd721ab3c1d33497a937e79d182339817290b8e4a173ec5ce69d40d28ae8d
//META: purpose=The entry-point-decorations pass attaches `[numthreads(...)]` to the entry-point IR function and the marker surfaces in every text target's emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `slang-ir-entry-point-decorations.cpp` (the "Applies stage /
// numthreads / etc." row of the entry-point category) attaches the
// `numthreads` decoration to the entry-point function. The
// decoration surfaces as `[numthreads(8,1,1)]` on HLSL,
// `layout(local_size_x = 8 ...)` on GLSL, `LocalSize 8 1 1` on
// SPIR-V, `@workgroup_size(8` on WGSL, and `kernel` on Metal /
// `__global__` on CUDA.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// HLSL: numthreads(8, 1, 1)
// GLSL: local_size_x = 8
// SPIRV: OpExecutionMode {{.*}} LocalSize 8 1 1
// WGSL: @workgroup_size(8
// METAL: kernel
// CUDA: __global__
