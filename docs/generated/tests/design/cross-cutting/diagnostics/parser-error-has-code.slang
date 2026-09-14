//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
//META: purpose=Parser diagnostics take ids from the conventional 2xxxx subsystem range.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that parser diagnostics draw their ids from the conventional
// 2xxxx subsystem range. A parameter list is opened and never closed; the
// caret-anchored CHECK pins the rendered code E20001 at the offending column,
// so a renumbering out of the parser range would fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int main(
{
/*CHECK:
^ E20001
^ expected identifier
*/
    return 0;
}
