//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=9bf971e79309b04b98ac1b07e618148a41a5c833af2e6a7f18f5e4b57eddceb9
//META: purpose=A RWStructuredBuffer element access outBuf[idx] lowers to OpAccessChain that indexes through the runtime array inside the wrapping struct.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V buffer-element indexing uses logical addressing.
// outBuf[idx] becomes OpAccessChain into the StorageBuffer
// pointer, with the first index selecting the runtime-array
// member (struct field 0) and the second index selecting the
// element at idx.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x + 3] = 5;
}

// CHECK: OpAccessChain %_ptr_StorageBuffer_int %outBuf
