//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=c2bc4d2909493a2c6dd101713168eaf7faf6486837f1b0abbdb93abe6ce44b5b
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
