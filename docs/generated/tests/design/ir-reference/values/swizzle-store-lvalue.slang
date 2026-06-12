//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=43f95de1df286eb0d08e5ff850f113aedd3fa02de0fadc610d88f2bc9ba79f21
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
