//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#eliminatephis-with-spir-v-specific-options
//META: doc_section_digest=fe05b23b1c0443d967eca4c904387718a54e3d95f22f2d7525a5b76c28360580
//META: purpose=A ternary "?:" expression emits an OpSelectionMerge structured-control-flow header with two OpStore branches into a Function-storage variable — eliminatePhis(useRegisterAllocation=true) rewrites the merge phi as a per-branch store.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the bundle pins eliminatePhis(register-allocation) for
// if/else. A ternary "?:" produces the same shape via
// OpSelectionMerge — exercising the phi-rewrite path on a different
// HLL construct.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = (tid.x > 0) ? 100 : 200;
}

// CHECK-DAG: OpSelectionMerge
// CHECK-DAG: OpStore %{{[A-Za-z_0-9]+}} %int_100
// CHECK-DAG: OpStore %{{[A-Za-z_0-9]+}} %int_200
