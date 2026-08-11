//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
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
