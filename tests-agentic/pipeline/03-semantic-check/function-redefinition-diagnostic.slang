//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Defining the same function twice is rejected (function-redefinition, 30201) with a note pointing at the previous definition.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The semantic checker enforces the "function may have one body"
// rule via the function-redefinition diagnostic. Verify both the
// primary span at the redeclaration and the note at the previous
// definition.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int foo() { return 1; }
/*CHECK:
    ^^^ see previous definition of 'foo'
*/

int foo() { return 2; }
/*CHECK:
    ^^^ function 'foo' already has a body
    ^^^ redeclared here
*/
// CHECK: E30201
