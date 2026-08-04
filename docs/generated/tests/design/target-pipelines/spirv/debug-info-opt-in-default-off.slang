//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=ea1dc5e43dbe24afedb64d7530802c7f0537a68697032d3fc655a1128601e739
//META: purpose=Without -g, no DebugLine OpExtInst calls appear; the only source marker is OpSource Slang 1.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// targetCompilerOptions.getDebugInfoLevel() defaults to a non-
// emitting level. Without -g, the SPIR-V emit carries an
// OpSource Slang 1 marker but no per-instruction DebugLine
// ExtInst calls (which only appear when debug info is requested).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: OpSource Slang 1
// CHECK-NOT: DebugLine
