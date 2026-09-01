//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
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
