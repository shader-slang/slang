//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:36:14Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=9aab17755318b3eba1171ef991b0e4f492c51e9b08946482351cebd403903cdb
//META: purpose=The discarded-result check also covers a `for` loop side-effect expression, so a `[NoDiscard]` call written there is reported (E30059).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `for` side-effect position the refreshed doc names alongside
// the bare expression statement: the third clause of the loop is evaluated
// only for effect, so its value is discarded. CHECK pins E30059.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[NoDiscard]
int f()
{
    return 1;
}

void main()
{
    for (int i = 0; i < 2; f())
//CHECK: E30059
    {
        i++;
    }
}
