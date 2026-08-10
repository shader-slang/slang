//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#error-recovery
//META: doc_section_digest=490a9dc1fa74d15b5c32d617920911d5d9b67dbf1c0dd0c9637afde42b648b85
//META: purpose=While the parser is recovering, no further unexpected-token diagnostic is reported, so a run of three bad tokens yields exactly one syntax error.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the cascade-suppression half of error recovery. Three consecutive
// unrecognised punctuators appear in statement position; each on its own would
// be an unexpected token, so a parser without the recovering flag would report
// three errors. The directive runs in exhaustive mode, meaning any diagnostic
// the CHECK annotations do not consume fails the test — so a single annotated
// error at the first `@` is what proves the following two were suppressed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int a = 1;
    @ @ @ ;
/*CHECK:
    ^ error E20002
    ^ syntax error.
*/
    int b = a;
}
