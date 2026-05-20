//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=d6e74209f0cfcbd939127caa9bf2791387db9155c2d0b2f0a561349e2e185456
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
