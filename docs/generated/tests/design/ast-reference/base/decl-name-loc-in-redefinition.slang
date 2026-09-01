//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#decl-declbase
//META: doc_section_digest=e37d0cabbf8f1a75a6de578cd8488f2df3e6f4c6f1f7b7cc4a11429862830842
//META: purpose=A Decl's nameAndLoc pairs the declared name with the source location of the name token, so a conflicting-declaration diagnostic points at the name column rather than at the start of the declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the location a Decl stores is the location of the
// declared *name*, not of the whole declaration. Two module-scope
// variables share the spelling `dup`; the checker rejects the second
// one. The caret annotation sits under `dup` (column 7), several
// columns past the start of the `float` declaration, and the note
// points at the first declaration's own name column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int dup;
/*CHECK:
    ^^^ see previous declaration of 'dup'
*/
float dup;
/*CHECK:
      ^^^ conflicting declaration
      ^^^ declaration of 'dup' conflicts with existing declaration
*/
// CHECK: E30200

void main() {}
