//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: purpose=A multi-component swizzle (`v.xyz`) lowers to a `swizzle` opcode taking the source vector and one literal index per output lane.
//META: doc_section_digest=c2bc4d2909493a2c6dd101713168eaf7faf6486837f1b0abbdb93abe6ce44b5b
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's swizzle note describes `swizzle src idx0 idx1 ...` —
// "reads M elements from an N-vector to make an M-vector". The
// `v.xyz` form on a `float4` value produces
// `swizzle(%v, 0 : Int, 1 : Int, 2 : Int)`. The cross-cutting
// bundle covers single-component swizzle from a `uint3`; this test
// covers multi-component and the inline literal-index encoding.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float3> vbuf;

[numthreads(1, 1, 1)]
void main()
{
    vbuf[0] = v.xyz;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int, 1 : Int, 2 : Int)
