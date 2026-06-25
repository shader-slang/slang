//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=43f95de1df286eb0d08e5ff850f113aedd3fa02de0fadc610d88f2bc9ba79f21
//META: purpose=Negative boundary: accessing the `.w` component of a `float3` (one past the N=3 edge) is rejected with "'w' is not a member of 'vector<float,3>'." — confirms the index-range check at every documented vector width.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Out-of-range swizzle, taken at the N=3 size. The diagnostic
// message names the source vector type and the missing component
// name, demonstrating the per-width range check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform float3 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.w;
//CHECK:       ^ member not found
//CHECK:       ^ 'w' is not a member of 'vector<float,3>'.
// CHECK: E30027
}
