//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Verifies the lower-edge boundary of SizeOfExpr: `sizeof(int8_t)` is the documented value 1.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for SizeOfExpr: the smallest documented
// numeric type is 1 byte wide.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = int(sizeof(int8_t));
    // CHECK: s=1
    printf("s=%d\n", s);
}
