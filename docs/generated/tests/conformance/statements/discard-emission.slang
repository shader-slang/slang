//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#discard-statement
//META: doc_section_digest=c2e15d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3b4
//META: purpose=discard in a fragment shader emits a discard instruction in the target output.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the discard statement compiles and emits a discard/kill in HLSL and SPIR-V.
// discard is valid only in fragment shaders; this test targets pixel-shader compilation.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=SPVCHECK):-target spirv-asm -entry fragMain -stage fragment

float4 fragMain(float4 color : COLOR) : SV_Target
{
    if (color.a < 0.1)
        discard;
    return color;
}

// HLSL should emit discard
// CHECK: discard
// SPIR-V should emit OpKill or OpTerminateInvocation
// SPVCHECK: OpKill
