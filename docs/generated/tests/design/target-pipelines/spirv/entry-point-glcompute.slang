//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=a11b0c6012622aa65eafd1deea44aaeb117d13faf8de8cf2fe2778e1dd206d36
//META: purpose=A compute entry point emits OpEntryPoint GLCompute %func "main" with the GLCompute execution model.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The OpEntryPoint instruction names the execution model
// ("GLCompute" for a compute shader), the SPIR-V function symbol,
// the user-visible entry-point name string ("main"), and the
// referenced interface variables.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpEntryPoint GLCompute %{{[a-zA-Z_0-9]+}} "main"
