//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=c9513e1455ac8d0e90436e3db8230754dc3be3e0a40bcd7e4c788a745e88d3e7
//META: purpose=An offset attribute gains a third operand holding the register space when that space is non-zero.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper edge of the same optional operand. Binding the texture
// and sampler into `space2` forces a non-zero space, so the corresponding
// `offset` attributes must carry the third operand with the value 2 -- the
// case the two-operand encoding cannot express.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o - -entry main -stage compute

Texture2D gTex : register(t0, space2);
SamplerState gSamp : register(s0, space2);
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gTex.SampleLevel(gSamp, float2(0, 0), 0).x;
}

// CHECK: = offset({{[0-9]+}} : Int, {{[0-9]+}} : Int, 2 : Int)
