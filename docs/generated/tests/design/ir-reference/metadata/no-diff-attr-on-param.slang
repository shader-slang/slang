//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=0e7a8ec2a490ce368dde09c10d45863e67fc1e491cc433074fe4f7bbf890e750
//META: purpose=The no_diff modifier on a parameter type produces a no-operand no_diff attribute that marks the type as not contributing to derivatives.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the derivative-exclusion marker attribute. `no_diff float` on a
// parameter is the surface; the CHECK captures the zero-operand attribute
// definition and requires the same id to appear as the attribute operand of an
// `Attributed` type, showing the marker travels with the parameter's type
// rather than as a decoration on the function.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

float helper(no_diff float x)
{
    return x * 2.0;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helper(float(tid.x));
}

// CHECK: %[[N:[0-9]+]]{{.*}}= no_diff
// CHECK: Attributed({{.*}}, %[[N]])
