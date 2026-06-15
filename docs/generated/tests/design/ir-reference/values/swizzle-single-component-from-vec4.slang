//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=43f95de1df286eb0d08e5ff850f113aedd3fa02de0fadc610d88f2bc9ba79f21
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
