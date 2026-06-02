//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#wave-execution-model
//META: doc_section_digest=8d83b79ec2f7645747c3d299d4c9c52ee4ddc1279b8d96b52ef616d23d94159e
//META: purpose=The discard statement disables a fragment thread (making it inactive); it emits as discard in HLSL and OpKill in SPIR-V for a fragment entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "A thread can be inactive for one of the following reasons:
// ... The thread has executed a `discard` statement, which disables the thread
// (fragment shaders only)."
//
// We verify that `discard` in a fragment entry point compiles and emits
// the target-specific discard/kill instruction.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry fragMain -stage fragment

float4 fragMain(float4 pos : SV_Position) : SV_Target
{
    if (pos.x < 0.0)
        discard;
    return float4(1.0, 0.0, 0.0, 1.0);
}

// HLSL emits the discard statement.
// CHECK: discard

// SPIR-V emits OpKill (the SPIR-V discard operation for fragment shaders).
// CHECK-SPV: OpKill
