//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=c2bc4d2909493a2c6dd101713168eaf7faf6486837f1b0abbdb93abe6ce44b5b
//META: purpose=Boundary: single-component swizzle from a `float4` (`v.x`) lowers to `swizzle(%v, 0 : Int)` with exactly one literal index — the M=1 edge of the doc's "M elements from an N-vector" claim.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's swizzle note describes "reads M elements from an
// N-vector to make an M-vector". The smoke test covers the
// M=3 case; this boundary probe takes M=1, the lower-edge length.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int)
