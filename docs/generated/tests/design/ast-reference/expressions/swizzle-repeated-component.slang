//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
