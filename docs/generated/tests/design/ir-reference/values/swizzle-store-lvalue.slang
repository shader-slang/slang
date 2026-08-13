//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=b3a4828968ba320e73bd253425b6f464bf34ceea7d61ab6e1933e1f91bcb19a9
//META: purpose=A swizzle in lvalue position (`dst.xy = ...`) lowers to `swizzledStore` mutating memory in place.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's swizzle note describes three forms — `swizzle` (read),
// `swizzleSet` (functional update), and `swizzledStore` (in-place
// memory store). Writing to a partial vector lvalue (`dst.xy =
// float2(fa, fa)`) lowers to `swizzledStore(%dst, ..., 0 : Int,
// 1 : Int)` in the IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform float fa;
uniform RWStructuredBuffer<float4> v4buf;

[numthreads(1, 1, 1)]
void main()
{
    float4 dst = v;
    dst.xy = float2(fa, fa);
    v4buf[0] = dst;
}

// IR-LABEL: func %main
// IR: swizzledStore(%dst, %{{.*}}, 0 : Int, 1 : Int)
