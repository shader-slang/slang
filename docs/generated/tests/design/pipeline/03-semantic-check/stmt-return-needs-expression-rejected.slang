//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=955b45bda31b26608aa302d1b2e04635daf73a8cc4ddd4784579a780f4e536d1
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
