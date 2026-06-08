//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: a `continue` outside any loop construct renders E30004 with the documented "must appear inside loop" message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `continue-outside-loop` (30004) to its rendered code and
// the exact message from the lua table. Distinct id from
// break-outside-loop (30003) confirms the uniqueness claim.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    continue;
/*CHECK:
    ^^^^^^^^ E30004
    ^^^^^^^^ 'continue' must appear inside loop constructs.
*/
}
