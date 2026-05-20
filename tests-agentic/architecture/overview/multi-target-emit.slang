//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/architecture/overview.md#purpose
//META: doc_section_digest=29ff1a17a16f6d189494cbbb74a8012455c22e933ba970444c1402a95877725f
//META: purpose=One Slang source compiles to every text-emit target the doc lists (HLSL, GLSL, SPIR-V, Metal, WGSL, CUDA, C++).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the central architectural claim that Slang is a multi-target
// compiler: the same source compiles to each of the text-emit targets
// named in the doc's `## Purpose` section. Each directive runs the same
// source through one target; per-target syntax that only that backend
// emits is FileChecked.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outBuf;

void computeMain()
{
    //HLSL: RWStructuredBuffer
    //GLSL: layout(std430
    //SPV: OpEntryPoint
    //MTL: kernel
    //WGSL: @compute
    //CUDA: __global__
    //CPP: SLANG_PRELUDE_EXPORT
    outBuf[0] = int('A');
}
