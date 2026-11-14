//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=d1da3e17144b1181125647a41f45bc77bf033878ec635adafe5eb5de985e8006
//META: purpose=Boundary: single-component swizzle from a `float4` (`v.x`) lowers to `swizzle(%v, 0 : Int)` with exactly one literal index — the M=1 edge of the doc's "M elements from an N-vector" claim.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's swizzle note describes "reads M elements from an
// N-vector to make an M-vector". The smoke test covers the
// M=3 case; this boundary probe takes M=1, the lower-edge length.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int)
