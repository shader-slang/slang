//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A write through a nested struct member chains additional OpAccessChain steps and emits an Offset 8 on the outer struct member (the documented Offset decoration per member).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: chained struct member access through a nested struct.
// SPIR-V represents this as additional OpAccessChain steps; each
// inner member carries its own OpMemberDecorate Offset.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct Inner
{
    float a;
    float b;
}

struct Outer
{
    Inner inner;
    float c;
}

RWStructuredBuffer<Outer> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x].inner.a = float(tid.x);
}

// CHECK-DAG: OpMemberDecorate %Outer{{[A-Za-z_0-9]*}} 1 Offset 8
// CHECK-DAG: OpAccessChain %{{[A-Za-z_0-9]+}} %buf %int_0
