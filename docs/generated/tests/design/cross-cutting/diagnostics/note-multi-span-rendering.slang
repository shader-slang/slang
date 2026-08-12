//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=79bb0fc1ac8583fef268d4e89fe89d0e6aa8ce66124dff7c481098ca077adbae
//META: purpose=An entry whose primary `span` is followed by a `note` renders both: the span on the offending site and the note on the earlier site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an entry declaring a primary span plus a note renders two
// co-located records rather than one. A function is given two bodies; the
// annotations pin the primary message at the redefinition and the
// `see previous definition` note at the first body, so the exhaustive check
// fails if either record is lost.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int bar(){ return 1; }
/*CHECK:
    ^^^ see previous definition of 'bar'
*/

int bar(){ return 2; }
/*CHECK:
    ^^^ function 'bar' already has a body
    ^^^ redeclared here
*/
