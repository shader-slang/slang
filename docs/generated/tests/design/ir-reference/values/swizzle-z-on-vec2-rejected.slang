//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=4d65a5f477addb92a25c5896b6cb496d8436da7a5ccfb3b367db94c8861a7ee9
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
