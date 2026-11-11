//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#two-pass-interaction-with-the-parser
//META: doc_section_digest=a7dea93156126395af43f909ce2264a419291aa4fbab609c33243957962e4449
//META: purpose=A function body the parser deferred as an UnparsedStmt is parsed and checked on demand, so its statements execute normally.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the deferred body really does get parsed and checked rather
// than skipped. The body holds a loop, a compound assignment and a local, none
// of which exist until the checker calls back into the parser for this
// function. CHECK asserts the accumulated value, which requires the whole body
// to have been re-parsed, type-checked, and executed.

//TEST:INTERPRET(filecheck=CHECK):

int helper(int n)
{
    int acc = 0;
    for (int i = 0; i < n; i = i + 1)
    {
        acc += i;
    }
    return acc;
}

void main()
{
    // CHECK: acc=10
    printf("acc=%d\n", helper(5));
}
