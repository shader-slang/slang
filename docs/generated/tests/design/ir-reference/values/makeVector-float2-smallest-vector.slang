//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=136fa0f20043e0a8db805b7f018d292446250e6117047722c77e7aa7efca0675
//META: purpose=Boundary: `float2(fa, fb)` (smallest documented vector dimension) lowers to `makeVector(%fa, %fb)` with result type `Vec(Float, 2 : Int)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The smoke test covers float3 and float4. This boundary probe
// takes the smallest documented vector size (N=2) on the
// makeVector axis.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<float2> v2buf;

[numthreads(1, 1, 1)]
void main()
{
    v2buf[0] = float2(fa, fb);
}

// IR-LABEL: func %main
// IR: makeVector(%fa, %fb)
