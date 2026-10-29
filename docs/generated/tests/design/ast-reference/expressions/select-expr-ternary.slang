//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A SelectExpr `c ? a : b` yields a when c is true and b when c is false.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the ternary conditional yields its first arm when the
// condition is true and its second when false. Both directions are exercised
// with the same arms so the CHECKs distinguish "picked the right arm" from
// "always returns one side".

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = (1 < 2) ? 100 : 200;
    int b = (1 > 2) ? 100 : 200;
    // CHECK: a=100
    printf("a=%d\n", a);
    // CHECK: b=200
    printf("b=%d\n", b);
}
