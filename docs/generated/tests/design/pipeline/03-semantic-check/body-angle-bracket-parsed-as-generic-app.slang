//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#two-pass-interaction-with-the-parser
//META: doc_section_digest=a7dea93156126395af43f909ce2264a419291aa4fbab609c33243957962e4449
//META: purpose=Inside a deferred function body the parser calls back into the checker so `f<1, 2>(3)` is disambiguated as a generic application rather than a chain of comparisons.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `<`-disambiguation callback that the two-pass handoff exists
// for. `tag<1, 2>(3)` is textually ambiguous with `(tag < 1), (2 > 3)`; only a
// parser that can ask the checker whether `tag` names a generic will build a
// generic application here. CHECK asserts the digits of both value arguments
// plus the ordinary argument, which no comparison reading could produce.

//TEST:INTERPRET(filecheck=CHECK):

int tag<let A : int, let B : int>(int d)
{
    return A * 100 + B * 10 + d;
}

void main()
{
    // CHECK: t=123
    printf("t=%d\n", tag<1, 2>(3));
}
