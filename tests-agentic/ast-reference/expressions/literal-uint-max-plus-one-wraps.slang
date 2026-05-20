//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=Verifies that `uint(0xFFFFFFFFu) + 1u` wraps to 0 — the documented unsigned overflow semantics for the uint base type.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// MAX+1 overflow for the uint base type. The literal at MAX is an
// IntegerLiteralExpr with suffixType=uint; the InfixExpr `+` invokes
// the unsigned addition, which wraps to 0.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint a = 0xFFFFFFFFu;
    uint b = a + 1u;
    // CHECK: b=0
    printf("b=%u\n", b);
}
