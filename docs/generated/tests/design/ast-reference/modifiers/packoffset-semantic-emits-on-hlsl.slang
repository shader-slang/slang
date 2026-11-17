//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#hlsl-semantics
//META: doc_section_digest=924ecf08334941e0ce19332c62256f9af73bd09c738c20e634f3eefd0c64d11f
//META: purpose=A `: packoffset(...)` semantic places a constant-buffer field at the requested offset in emitted HLSL.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other layout-bearing HLSL semantic, `: packoffset(...)`, which
// carries a uniform offset rather than a register. Two fields request `c0` and
// `c1`; both are read so neither is eliminated, and the CHECK-DAG lines assert
// each field kept its requested offset in the emitted `cbuffer`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params
{
    float4 a : packoffset(c0);
    float4 b : packoffset(c1);
};

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = a.x + b.y;
}

//CHECK-DAG: packoffset(c0)
//CHECK-DAG: packoffset(c1)
