//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=730d07f4cad01d2edbbbf03523fae836c3254cc356ffd1ac578d0a21bad19381
//META: purpose=Verifies -fvk-invert-y inserts an OpFNegate on the SV_Position.y output that is absent without the flag.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of OptionKind::VulkanInvertY (-fvk-invert-y). The flag drives
// an IR pass that negates the Y component of SV_Position before stage output.
// The Y value is fed from a `uniform float` so the constant folder cannot
// precompute the negation. The INVERT directive pins the resulting `OpFNegate`;
// the PLAIN directive (no flag) uses CHECK-NOT to pin that no OpFNegate appears,
// isolating the option as the sole cause of the negate.

//TEST:SIMPLE(filecheck=INVERT):-target spirv-asm -fvk-invert-y -entry main -stage vertex
//TEST:SIMPLE(filecheck=PLAIN):-target spirv-asm -entry main -stage vertex

uniform float yval;

[shader("vertex")]
float4 main() : SV_Position
{
    return float4(0, yval, 0, 1);
}

//INVERT: OpFNegate
//PLAIN-NOT: OpFNegate
