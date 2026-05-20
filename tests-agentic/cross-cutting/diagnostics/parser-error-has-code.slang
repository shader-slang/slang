//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Parser-stage errors carry integer ids in their own range (2xxxx) per the diagnostic-id convention.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims diagnostic ids are unique per diagnostic and are
// assigned in conventional ranges per subsystem. Verify a parser
// error (range 2xxxx) renders with an E2#### code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int main(
{
/*CHECK:
^ E20001
^ expected identifier
*/
    return 0;
}
