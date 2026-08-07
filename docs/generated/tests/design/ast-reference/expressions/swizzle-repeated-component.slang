//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Stresses SwizzleExpr by repeating the same source component across the result: `v.xxxx` on (5,_,_,_) builds a vector of four 5s.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress on SwizzleExpr.elementIndices uniqueness: repeated indices
// are legal and must replicate the chosen component.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 v = float4(5.0f, 6.0f, 7.0f, 8.0f);
    float4 s = v.xxxx;
    // CHECK: c=5 5 5 5
    printf("c=%d %d %d %d\n", int(s.x), int(s.y), int(s.z), int(s.w));
}
