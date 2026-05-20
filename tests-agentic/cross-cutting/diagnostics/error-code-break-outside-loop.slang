//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: a `break` outside any loop / switch construct renders E30003 with the documented "must appear inside loop or switch" message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// One negative test per documented diagnostic code. Pin
// `break-outside-loop` (30003) to its rendered code and the
// exact "must appear inside loop or switch constructs" message
// from the lua table.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    break;
/*CHECK:
    ^^^^^ E30003
    ^^^^^ 'break' must appear inside loop or switch constructs.
*/
}
