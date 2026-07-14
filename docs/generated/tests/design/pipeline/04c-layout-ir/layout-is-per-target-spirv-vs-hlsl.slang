//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#why-this-module-exists
//META: doc_section_digest=f97d8ab9f1f96b1993f89917a566e350a842fd94e06662226e6a914e19152232
//META: purpose=Layout is target-specific; the same global parameter produces a [layout(...)] / varLayout / offset structure on both SPIR-V and HLSL targets (this test asserts presence on HLSL to complement the default SPIR-V test).
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Why this module exists" section says: "Layout is
// target-specific. The same source program can have different
// binding numbers, register spaces, byte offsets, ..." Each
// TargetProgram builds its own layout-IR module via
// createIRModuleForLayout. Verifying the layout instructions
// also appear on HLSL confirms the same per-target construction
// pipeline runs for non-SPIR-V targets.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: varLayout(
// CHECK: offset(
// CHECK: EntryPointLayout(
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: global_param
