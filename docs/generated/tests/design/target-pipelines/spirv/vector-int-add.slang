//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=An int4 + int4 add emits a single OpIAdd %v4int — SPIR-V's per-element arithmetic operates on the vector type without unpacking.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the bundle pins scalar uint OpIAdd at the overflow
// boundary. SPIR-V allows the same OpIAdd opcode to act on vector
// types directly. This pins OpIAdd %v4int for int4 + int4.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int4> outBuf;
RWStructuredBuffer<int> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int4 a = int4(inBuf[0], inBuf[1], inBuf[2], inBuf[3]);
    int4 b = int4(inBuf[4], inBuf[5], inBuf[6], inBuf[7]);
    outBuf[tid.x] = a + b;
}

// CHECK: OpIAdd %v4int
