//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
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
