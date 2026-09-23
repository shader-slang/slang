//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=1690bc07bc351f6d57ef0cbfbdf90dd7a3a711d8e434d872c178e166712d1f85
//META: purpose=The discarded-result check recurses into a short-circuit expression, so the `[NoDiscard]` call whose value is actually thrown away is the one reported.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the short-circuit arm of the recursion, and that the recursion is
// precise rather than blanket: the left operand's result feeds the `&&` and is
// not discarded, while the right operand's result is the statement's own
// discarded value. CHECK pins the single E30059 and its message, and
// exhaustive mode makes "only the right operand was reported" part of the
// assertion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[NoDiscard]
bool g()
{
    return true;
}

void main()
{
    g() && g();
//CHECK: E30059
//CHECK: the result of calling 'g' is discarded
}
