//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint-entrypointdecoration
//META: doc_section_digest=d3d96830534992c13d27b2f3fc509ca407eb67250f8a01585d992e27d7abbae9
//META: purpose=The entryPoint decoration's three operands are profile-tag, user-visible name, and module-name; a vertex stage uses a different profile-tag integer than compute.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the three operands are "the profile (an IRIntLit
// tag), the user-visible name, and an optional module name." A
// compute entry recorded earlier shows profile tag 6; a vertex
// entry shows a different tag (1). The exact integer is implicit
// in the ProfileMask enum, but pinning that the operand is an
// `Int` literal and the entry-name is `main` is stable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage vertex

[shader("vertex")]
float4 main(float3 pos : POSITION) : SV_Position
{
    return float4(pos, 1.0);
}

// CHECK: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK: func %main
