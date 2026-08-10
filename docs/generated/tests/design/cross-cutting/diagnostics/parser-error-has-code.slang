//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
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
