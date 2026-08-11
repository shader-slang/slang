//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=A `discard` statement terminates its block with the `discard` exit terminator and emits as each target's fragment-kill form.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `return_val / unreachable / discard` exit-terminator row for
// the `discard` member. The IR CHECK asserts `discard` appears as a bare
// terminator (no operands, ending its block) rather than as a call, and the
// per-target CHECKs assert the exit reaches emission — SPIR-V renders it as
// `OpKill` and Metal as `discard_fragment()`, which are the two spellings
// that differ enough from the source keyword to be worth pinning. The
// discard is guarded by a `uniform` comparison so the branch survives to
// emission instead of being folded away.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage fragment -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage fragment -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage fragment -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage fragment -entry main

uniform float a;

float4 main(float4 pos : SV_Position) : SV_Target
{
    if (a < 0.0)
        discard;
    return float4(a, 0, 0, 1);
}

// IR-LABEL: func %main
// IR: discard

// HLSL: discard;
// GLSL: discard;
// SPV: OpKill
// METAL: discard_fragment();
// WGSL: discard;
