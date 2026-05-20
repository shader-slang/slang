//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/architecture/overview.md#purpose
//META: doc_section_digest=29ff1a17a16f6d189494cbbb74a8012455c22e933ba970444c1402a95877725f
//META: purpose=Slang accepts HLSL-compatible source ("Slang (and HLSL-compatible) source code"); a `cbuffer` block is parsed and lowered to the chosen target.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an HLSL-flavoured `cbuffer` declaration is accepted by
// the Slang frontend and survives to emit on both HLSL and GLSL
// backends. If Slang did not accept HLSL-compatible syntax (as the
// `## Purpose` paragraph claims), the file would fail to parse and no
// emit would occur.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

cbuffer Globals
{
    float4 color;
    int scale;
};

RWStructuredBuffer<int> outBuf;

void main()
{
    //HLSL: cbuffer
    //GLSL: uniform
    outBuf[0] = int(color.x) + scale;
}
