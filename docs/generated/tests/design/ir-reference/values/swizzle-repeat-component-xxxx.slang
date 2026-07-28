//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=c2bc4d2909493a2c6dd101713168eaf7faf6486837f1b0abbdb93abe6ce44b5b
//META: purpose=Boundary: repeat-component swizzle (`v.xxxx`) lowers to `swizzle(%v, 0, 0, 0, 0)` — the same literal index repeated four times, exercising the duplicate-lane case of the doc's "M elements" claim.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "M elements from an N-vector" does not exclude
// repeated indices. This boundary probe takes M=4 from N=4 with
// all four lanes referencing index 0 — the broadcast case.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.xxxx;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int, 0 : Int, 0 : Int, 0 : Int)
