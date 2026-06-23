//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface may require `static const int` data members; the conforming struct provides one with the same name and type and the doc constraint that the type must be `int` or `bool`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that interface `static const` requirements with `int` and `bool`
// types are satisfiable. ConcreteWithConstants provides both members; CHECK
// asserts the concrete values are readable as intConst=10 and boolConst=1,
// confirming both types are accepted and the values propagate correctly.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasConstants
{
    static const int intConst;
    static const bool boolConst;
}

struct ConcreteWithConstants : IHasConstants
{
    static const int intConst = 10;
    static const bool boolConst = true;
}

void main()
{
    //CHECK: intConst=10 boolConst=1
    printf("intConst=%d boolConst=%d\n",
        ConcreteWithConstants.intConst,
        ConcreteWithConstants.boolConst ? 1 : 0);
}
