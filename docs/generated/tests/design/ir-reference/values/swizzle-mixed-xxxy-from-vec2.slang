//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=4d65a5f477addb92a25c5896b6cb496d8436da7a5ccfb3b367db94c8861a7ee9
//META: purpose=Boundary: M=4 swizzle from an N=2 source (`vec2.xxxy`) lowers to `swizzle(%v, 0, 0, 0, 1)` — M > N, mixed-repeat indices.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that lowering allows M > N when components are repeated. This
// boundary probe takes M=4 from N=2 with two distinct indices
// (one repeated three times), demonstrating the cross-axis
// upsample case.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float2 v;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.xxxy;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int, 0 : Int, 0 : Int, 1 : Int)
