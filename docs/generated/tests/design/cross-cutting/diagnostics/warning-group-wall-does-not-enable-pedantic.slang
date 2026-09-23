//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#warning-groups
//META: doc_section_digest=fe2a3c2e311ba8162e26f4c17f4a54e2d2b52665ed3d562b5445ade975ac7e04
//META: purpose=Boundary: the groups are independent rather than nested, so `-Wall` does not enable a `pedantic` warning.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the independence of the groups at the boundary a nested model would
// get wrong: under clang/gcc-style nesting `-Wall` would subsume the stricter
// groups. The vertex entry point that 38052 reports is compiled with `-Wall`
// only; CHECK-NOT asserts the pedantic warning is still silent, and CHECK
// asserts the compile itself succeeded.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry vertMain -stage vertex -Wall

struct VOut { float4 color : COLOR; };

VOut vertMain(uint id : SV_VertexID)
{
    VOut o;
    o.color = float4(float(id), 0, 0, 1);
    return o;
}

// CHECK-NOT: E38052
// CHECK-NOT: SV_Position' system value semantic
// CHECK: vertMain
