//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:00Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=3c2365d6e03d7d49bdb8070671c9161746cf63482dd5364eff59042e14f93322
//META: purpose=Ids in the shared namespace are normally unique per diagnostic; function-redefinition renders its own code E30201.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a diagnostic renders the id it was declared with (ids are
// normally unique even though the namespace admits intentional sharing).
// Pin the rendered code for function-redefinition to E30201 by FileChecking
// it on the redefinition line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int foo(){ return 1; }
/*CHECK:
    ^^^ see previous definition of 'foo'
*/

int foo(){ return 2; }
/*CHECK:
    ^^^ E30201
    ^^^ redeclared here
*/
