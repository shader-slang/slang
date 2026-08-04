//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=An IntegerLiteralExpr's suffixType drives overload selection: 1u selects the uint overload, 1 selects the int overload.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says IntegerLiteralExpr carries a suffixType: BaseType that
// selects the base type. Overloading on int vs uint is the cleanest
// observable: the suffix `u` must steer overload resolution to the
// uint candidate.

//TEST:INTERPRET(filecheck=CHECK):

int which(int v)  { return 1; }
int which(uint v) { return 2; }

void main()
{
    // CHECK: int=1
    printf("int=%d\n", which(1));
    // CHECK: uint=2
    printf("uint=%d\n", which(1u));
}
