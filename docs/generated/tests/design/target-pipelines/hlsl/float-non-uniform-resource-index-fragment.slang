//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-c-hlsl-legalization-lowering-phi-elimination
//META: doc_section_digest=a8c4fe8f82db2627965e05b91b8cd95b5dca976c67648791b7338b2ffdef93e1
//META: purpose=floatNonUniformResourceIndex runs for HLSL (!isSPIRV); the NonUniformResourceIndex(...) intrinsic survives in the HLSL emission of a fragment-stage bindless texture-array access.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `floatNonUniformResourceIndex` runs on HLSL (it is gated on
// `!isSPIRV`) in its textual mode, which keeps the
// `NonUniformResourceIndex(...)` marker intact in the emitted source instead
// of lowering it away. The test samples a bindless texture array from a
// fragment entry point with a divergent index; CHECK asserts the call
// spelling survives into the HLSL text, which only holds if the pass chose
// the textual float mode.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage fragment

Texture2D textures[16];
SamplerState samp;

[shader("fragment")]
float4 main(float3 uv : TEXCOORD0) : SV_Target
{
    int idx = int(uv.z);
    return textures[NonUniformResourceIndex(idx)].Sample(samp, uv.xy);
}

// CHECK: NonUniformResourceIndex(
