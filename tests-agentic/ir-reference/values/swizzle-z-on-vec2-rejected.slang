//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=43f95de1df286eb0d08e5ff850f113aedd3fa02de0fadc610d88f2bc9ba79f21
//META: purpose=Negative boundary: accessing the `.z` component of a `float2` (out of the documented N-vector range) is rejected by semantic check with "'z' is not a member of 'vector<float,2>'." — no swizzle opcode is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's swizzle note says "reads M elements from an N-vector"
// — the indices must be within [0, N). This negative test goes
// one past the N=2 edge with `.z` and verifies that the frontend
// rejects the surface (no `swizzle` IR is produced).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform float2 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.z;
//CHECK:       ^ member not found
//CHECK:       ^ 'z' is not a member of 'vector<float,2>'.
// CHECK: E30027
}
