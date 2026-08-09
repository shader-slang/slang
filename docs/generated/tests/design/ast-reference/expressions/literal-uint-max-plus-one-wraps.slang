//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=1e8f144399f6073f93949619c72c2230cfb4349def0ab49ac5088e68f85aa4ab
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
