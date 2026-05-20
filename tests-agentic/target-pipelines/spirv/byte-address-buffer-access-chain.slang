//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=ByteAddressBuffer.Load<T>(offset) lowers via legalizeByteAddressBufferOps (SPIR-V uses translateToStructuredBufferOps=true) into an OpAccessChain through the underlying runtime array.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// For SPIR-V, legalizeByteAddressBufferOps runs with
// scalarizeVectorLoadStore=false and
// translateToStructuredBufferOps=true. The HLSL template
// .Load<T>(offset) is rewritten into structured-buffer-style
// indexing - an OpAccessChain through a runtime array. No
// SPIR-V "Load<T>" template call exists.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

ByteAddressBuffer inBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = inBuf.Load<int>(tid.x * 4);
    outBuf[tid.x] = v;
}

// CHECK: OpAccessChain
