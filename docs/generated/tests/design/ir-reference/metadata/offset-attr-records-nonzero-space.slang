//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=43c0f4ec58392bc558dea4e760d5be974187198677d818ffcac504b279a89de3
//META: purpose=An offset attribute gains a third operand holding the register space when that space is non-zero.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper edge of the same optional operand. Binding the texture
// and sampler into `space2` forces a non-zero space, so the corresponding
// `offset` attributes must carry the third operand with the value 2 -- the
// case the two-operand encoding cannot express.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

Texture2D gTex : register(t0, space2);
SamplerState gSamp : register(s0, space2);
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gTex.SampleLevel(gSamp, float2(0, 0), 0).x;
}

// CHECK: = offset({{[0-9]+}} : Int, {{[0-9]+}} : Int, 2 : Int)
