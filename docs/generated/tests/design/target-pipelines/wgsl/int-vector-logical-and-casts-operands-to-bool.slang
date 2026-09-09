//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizelogicalandor
//META: doc_section_digest=6ccd530f856cc533b6af482353d594b904fc925de647213df1fea8069c0317f2
//META: purpose=legalizeLogicalAndOr casts non-boolean vector operands of a logical And to vector<bool,N> and rebuilds the And
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-boolean-operand half of legalizeLogicalAndOr. A logical
// `&&` written over two `int3` values has operands whose type WGSL cannot
// feed to a boolean And, so the pass casts each operand (and the result) to
// vector<bool,3> before rebuilding the And. Both operands are derived from
// the dispatch id so nothing folds; CHECK asserts the emitted operands are
// vec3<bool> conversions of vec3<i32> values feeding a select.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int3 a = int3(int(tid.x), int(tid.y), int(tid.z));
    int3 b = int3(1, 0, 1);
    bool3 r = a && b;
    outBuf[tid.x] = r.x ? 1 : 0;
}

// CHECK: select(vec3<bool>(false), vec3<bool>(vec3<i32>
// CHECK-SAME: vec3<bool>(vec3<i32>
