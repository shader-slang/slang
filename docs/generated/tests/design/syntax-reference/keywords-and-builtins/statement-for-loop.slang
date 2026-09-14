//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=58adeae3c01b07a208e3d126aee114b032a435add75925b7e18289c39283719b
//META: purpose=`for` is a statement keyword that opens a counted-loop header with init/condition/update clauses.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the canonical `for (init; cond; update) { body }`
// syntax is parsed and executed. Summing 1..3 yields 6.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 1; i <= 3; i = i + 1)
    {
        sum = sum + i;
    }
    // CHECK: sum=6
    printf("sum=%d\n", sum);
}
