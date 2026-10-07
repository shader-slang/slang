//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=1690bc07bc351f6d57ef0cbfbdf90dd7a3a711d8e434d872c178e166712d1f85
//META: purpose=The discarded-result check recurses through a comma expression, so a `[NoDiscard]` call in either operand is reported.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the comma arm of the recursion. Both operands of a discarded comma
// expression are themselves discarded, so both calls must be reported rather
// than just the trailing one. CHECK pins both E30059 reports, and also pins
// the comma operator's own advisory warning so the file records everything the
// compiler said about this statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[NoDiscard]
int f()
{
    return 1;
}

void main()
{
    (f(), f());
//CHECK: E30059
//CHECK: E30059
//CHECK: E41024
}
