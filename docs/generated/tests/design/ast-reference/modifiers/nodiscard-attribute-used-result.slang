//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:51+00:00
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability-autodiff-annotations
//META: doc_section_digest=3959c707a386e75c3eea38b819f320935508e4d38543a0a9aeb0a01aca4d3471
//META: purpose=`[NoDiscard]` is accepted on a value-returning function, and a call whose result is consumed checks and runs normally.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `NoDiscardAttribute` flags a function whose result must not be discarded.
// This exercises the accepted path: the attribute parses on a value-returning
// function, and a call site that consumes the result passes checking and
// computes the expected value (the attribute must not perturb the call).
// The rejected path (discarding the result) is doc-gapped: the doc names no
// diagnostic code, so no DIAGNOSTIC_TEST expectation can be pinned here.

//TEST:INTERPRET(filecheck=CHECK):

[NoDiscard]
int triple(int x)
{
    return x * 3;
}

void main()
{
    int r = triple(14);
    //CHECK: r=42
    printf("r=%d\n", r);
}
