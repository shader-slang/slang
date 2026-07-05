//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=Converting int to float in user code emits OpConvertSToF — companion to the documented OpBitcast (which only handles same-width type-pun reinterprets).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the bundle pins OpBitcast for uint->int (same-width
// type-pun). A value-preserving int->float conversion emits
// OpConvertSToF — a different SPIR-V opcode family from OpBitcast.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float> outBuf;
RWStructuredBuffer<int> inBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = inBuf[tid.x];
    outBuf[tid.x] = float(x);
}

// CHECK: OpConvertSToF
