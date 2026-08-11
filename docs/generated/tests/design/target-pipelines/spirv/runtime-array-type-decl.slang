//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A RWStructuredBuffer<T> emits an OpTypeRuntimeArray %T declaration wrapped inside the Block-decorated struct — the explicit SPIR-V opcode for "unbounded array".
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the bundle pins ArrayStride on the runtime array but
// not the OpTypeRuntimeArray declaration itself. SPIR-V has a
// distinct opcode for unbounded arrays — separate from OpTypeArray
// (statically sized).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) & 7;
}

// CHECK: OpTypeRuntimeArray %int
