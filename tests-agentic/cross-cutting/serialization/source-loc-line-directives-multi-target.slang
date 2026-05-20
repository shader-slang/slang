//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/cross-cutting/serialization.md#source-location-serialization
//META: doc_section_digest=a7785d3d0d63d41c707dce8b6259552b48fc0294b3f0028ba5ef1269c5af67e6
//META: purpose=Source locations survive the compile and reappear in emitted text as #line directives on C-family targets and as OpSource/OpLine markers on SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Source-location serialization" tells us the integer locations are
// rehydrated against a fresh SourceManager on load. The end-to-end
// observable is that emit carries source-location markers pointing
// back at the input file. HLSL and GLSL emit `#line` directives;
// SPIR-V emits `OpSource Slang` (with optional `OpLine` when debug
// info is requested).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -line-directive-mode standard
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute -skip-spirv-validation

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 42;
}

// HLSL: #line {{[0-9]+}} "{{.*source-loc-line-directives-multi-target\.slang}}"
// GLSL: #line {{[0-9]+}}
// SPIRV: OpSource Slang
