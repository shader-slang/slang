//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#two-pass-interaction-with-the-parser
//META: doc_section_digest=256a93a6f5809caaf2388b80d4fd482fa44b1f1872e9a0e6becde5c2caebda36
//META: purpose=When the name before `<` is an ordinary local rather than a generic, the same token shape inside a body parses as relational comparisons.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other side of the `<`-disambiguation decision. The token shape
// is identical to the generic-application test, but `v` is a local `int`, so
// the checker callback reports "not a generic" and the parser must fall back
// to `(v < one) `-style relational parsing. CHECK asserts the boolean results
// of the two comparisons, which only the comparison reading yields.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 5;
    int one = 1;
    int nine = 9;
    bool lo = v < one;
    bool hi = v < nine;
    // CHECK: lo=0 hi=1
    printf("lo=%d hi=%d\n", lo ? 1 : 0, hi ? 1 : 0);
}
