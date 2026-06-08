//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A read-only StructuredBuffer carries the NonWritable SPIR-V decoration in addition to Block / Binding.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V's NonWritable decoration marks a variable as read-only
// to the shader. Slang applies it to the inputs side of
// StructuredBuffer / ByteAddressBuffer (immutable from the
// shader's POV; writes happen on the host).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x] + 1;
}

// CHECK-DAG: OpDecorate %inBuf NonWritable
