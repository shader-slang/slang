//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#matrix-layout-modifiers
//META: doc_section_digest=9c412555177df54d8a02994bc4dd822ca52bf44cd21e2b3b4fa0c13ca797d935
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
