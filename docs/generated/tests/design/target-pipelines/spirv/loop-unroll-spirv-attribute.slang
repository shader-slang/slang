//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A [unroll] attribute on a for-loop changes the OpLoopMerge attribute to "Unroll" — companion to the unattributed loop test that emits "None".
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: structured control flow preserves loop hints. The
// bundle pins OpLoopMerge for an unattributed loop; this pins the
// [unroll]-attributed shape — OpLoopMerge ... Unroll.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        sum += i;
    }
    outBuf[tid.x] = sum;
}

// CHECK: OpLoopMerge %{{[A-Za-z_0-9]+}} %{{[A-Za-z_0-9]+}} Unroll
