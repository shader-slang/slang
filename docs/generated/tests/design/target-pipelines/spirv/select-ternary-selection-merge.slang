//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#eliminatephis-with-spir-v-specific-options
//META: doc_section_digest=b36f8fb17f73698f8db16cbc260b4d3068995c13f2e365b125cbcf5423d67537
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
