//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:31:34+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/architecture/module-map.md#code-emission
//META: doc_section_digest=54c3ed6ad11e306a70eade49ca5c608eaaa20fc13929dce0907bef1d0aecaeaf
//META: purpose=The "Code emission" table lists an emit dispatcher and per-target backends for HLSL, GLSL, SPIR-V, Metal, WGSL, CPP, and CUDA; the same source compiles to each.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the consequence of the "Code emission" subsystem listed in
// the module map: a dispatcher selects a per-target backend and emits
// target-specific text. The same Slang source is compiled to seven
// text-emit targets; each emit must contain a token that is unique to
// that target's syntax.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    //HLSL: RWStructuredBuffer
    //GLSL: #version
    //SPV: OpCapability
    //METAL: metal_stdlib
    //WGSL: @compute
    //CUDA: slang-cuda-prelude.h
    //CPP: slang-cpp-prelude.h
    outBuf[tid.x] = int(tid.x);
}
