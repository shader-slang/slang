//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=c2bc4d2909493a2c6dd101713168eaf7faf6486837f1b0abbdb93abe6ce44b5b
//META: purpose=Boundary: M=4 swizzle from an N=2 source (`vec2.xxxy`) lowers to `swizzle(%v, 0, 0, 0, 1)` — M > N, mixed-repeat indices.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc allows M > N when components are repeated. This
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
