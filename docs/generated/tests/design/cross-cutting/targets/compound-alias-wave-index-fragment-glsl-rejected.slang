//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#auto-generated-reference
//META: doc_section_digest=193f4efb497d3a5e48943b5ae86614705efc25f80ae9ad3a2ea85d4446323ddb
//META: purpose=The `subgroup_workgroup_index` compound alias restricts `WaveGetWaveIndex` to compute-class stages on GLSL; a fragment entry point calling it is rejected by the capability system with E36107.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the compound-alias gating path: `WaveGetWaveIndex` is gated by
// the `subgroup_workgroup_index` alias, which on GLSL admits only
// compute-class stages. A fragment entry point calling the query trips the
// capability join at check time; the CHECK pins the E36107 stage/target
// mismatch, proving misuse is rejected up front rather than reaching the
// GLSL backend and producing invalid output.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -entry main -stage fragment

[shader("fragment")]
float4 main() : SV_Target
/*CHECK:
       ^^^^ entrypoint 'main' uses features that are not available in 'fragment' stage for 'glsl' compilation target.
*/
//CHECK: E36107
{
    return float4(float(WaveGetWaveIndex()), 0, 0, 1);
}
