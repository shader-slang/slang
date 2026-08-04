//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
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
