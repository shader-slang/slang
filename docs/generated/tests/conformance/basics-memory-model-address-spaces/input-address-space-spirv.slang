//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=Entry-point input parameters (non-uniform) belong to the Input address space; SPIRV emits built-in inputs with `Input` storage class.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that entry-point input parameters in the non-uniform (per-thread) Input address
// space map to SPIRV `Input` storage class. `SV_DispatchThreadID` is a system-supplied
// per-thread value; CHECK asserts the SPIRV emits an `OpVariable ... Input` with a
// `BuiltIn GlobalInvocationId` decoration, confirming the Input address-space mapping.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<uint> output)
{
    output[tid.x] = tid.x;
}

// SV_DispatchThreadID (a non-uniform per-thread input) emits as an Input storage class variable.
// CHECK: OpVariable {{.*}} Input
// CHECK-DAG: BuiltIn GlobalInvocationId
// Metal maps SV_DispatchThreadID to thread_position_in_grid builtin attribute.
// METAL: thread_position_in_grid
// WGSL maps SV_DispatchThreadID to @builtin(global_invocation_id).
// WGSL: @builtin(global_invocation_id)
