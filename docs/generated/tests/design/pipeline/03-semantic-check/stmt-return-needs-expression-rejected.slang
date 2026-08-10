//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=b5f523113b24470cb3eee4f9c8ae76906f41333e955cb7bb7bac14ed1e5d3a05
//META: purpose=Statement checking validates `return` against the function's return type, so a bare `return` in an `int` function is rejected with E30006.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "return-type validation" concern. The enclosing function is
// declared to return `int`, so a valueless `return` cannot satisfy it; CHECK
// pins E30006 on the return statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int f()
{
    return;
//CHECK: E30006
}

void main()
{
    f();
}
