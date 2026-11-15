//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
//META: purpose=A per-id override takes precedence over warning-group gating, so `-W<id>` force-enables a single warning from a group that is off.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented ordering in which a per-id override is consulted before
// the group gate. 38052 lives in the pedantic group, which is off by default, so
// naming the id with `-W38052` must make it fire even though `-Wpedantic` was not
// passed. CHECK asserts the code and message appear; the sibling
// warning-group-pedantic-silent-by-default.slang shows the same shader is silent
// without any override.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry vertMain -stage vertex -W38052

struct VOut { float4 color : COLOR; };

VOut vertMain(uint id : SV_VertexID)
{
    VOut o;
    o.color = float4(float(id), 0, 0, 1);
    return o;
}

// CHECK: E38052
// CHECK: no output with the 'SV_Position' system value semantic
