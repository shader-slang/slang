//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-policy-overrides-and-bool-conditioned-values
//META: doc_section_digest=d47eda324078a32dbc73f59419bad83ce4ed4cd8713fc5e48372759b3592f93c
//META: purpose=A component-wise vector<bool,3> to vector<int,3> cast emits a vector select because the same emitter arm tests the vector element type
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the vector edge of the same bool-to-int emitter arm: the operand
// test looks at getVectorElementType, so a `bool3` operand takes the select
// path too, and WGSL's select is component-wise when its condition is a
// vector. The condition vector is derived from the dispatch id to defeat
// folding; CHECK pins the vec3<i32> zero/one operands of the select.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int3> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 c = tid > uint3(0u, 0u, 0u);
    outBuf[tid.x] = int3(c);
}

// CHECK: select(vec3<i32>(0), vec3<i32>(1),
