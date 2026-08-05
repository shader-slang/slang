//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#moving-and-deleting-control-flow-instructions
//META: doc_section_digest=1a35f3e6e880b02476bb7fbe3db3c3bfa0fe959a20d8b9005e5bd64d9b003102
//META: purpose=discard survives dead-code elimination at -O3 because it reports as possibly-side-effecting even though it produces no value.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same conservative side-effect classification for `discard`.
// The instruction yields nothing and has no users, so a value-based DCE
// would be free to drop it and silently change which pixels are written.
// The shader is compiled at -O3 and each CHECK asserts the target's own
// spelling of `discard` is still in the emitted text, guarded by a condition
// read from a uniform so the branch itself cannot be folded away first.
//
// The claim is target-dependent -- each back-end spells the kill differently
// (`discard;` in HLSL/GLSL/WGSL, `discard_fragment();` in Metal, `OpKill` in
// SPIR-V) -- so every text-emit shader target gets its own directive.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -O3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=GLSL):-target glsl -O3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -O3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=METAL):-target metal -O3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -O3 -entry main -stage fragment

uniform float threshold;

float4 main(float4 pos : SV_Position) : SV_Target
{
    if (pos.x < threshold)
        discard;
    return float4(1, 0, 0, 1);
}

// HLSL-LABEL: float4 main(
// HLSL: discard;

// GLSL-LABEL: void main()
// GLSL: discard;

// SPIRV-LABEL: %main = OpFunction
// SPIRV: OpKill

// METAL-LABEL: pixelOutput_0 main_0(
// METAL: discard_fragment();

// WGSL-LABEL: fn main(
// WGSL: discard;
