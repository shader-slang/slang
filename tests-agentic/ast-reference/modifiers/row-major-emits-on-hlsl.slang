//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#matrix-layout-modifiers
//META: doc_section_digest=f5bf6180424a661517e1d7eed081d1290763e2ddcb8df006dbe35e44a85076e4
//META: purpose=`row_major` matrix-layout modifier survives into emitted GLSL layout.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLRowMajorLayoutModifier` is the modifier behind the HLSL
// `row_major` matrix-layout keyword. In emitted GLSL the layout
// appears as `layout(row_major) ...` qualifiers on the uniform/buffer
// block carrying the matrix.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

cbuffer Params
{
    row_major float4x4 m;
};

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = m[0][0];
}

//CHECK: row_major
