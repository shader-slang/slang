//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A renamed entry-point function keeps its symbol (%myKernel) but the OpEntryPoint string operand is the canonical "main" name the driver looks up.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Slang preserves the user's entry-point symbol on the SPIR-V
// function (myKernel) but reports "main" as the
// driver-visible entry-point name string in OpEntryPoint.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry myKernel -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void myKernel(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpEntryPoint GLCompute %myKernel "main"
// CHECK: %myKernel = OpFunction %void None
