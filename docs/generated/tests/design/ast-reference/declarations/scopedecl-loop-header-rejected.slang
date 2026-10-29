//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A ScopeDecl also covers a loop header, so a variable declared there is not visible after the loop.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the loop header gets its own scope rather than leaking into
// the enclosing function. The loop variable is referenced after the loop; the
// CHECK pins the undefined-identifier diagnostic, which is the observable
// consequence of the header scope having ended.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    for (int i = 0; i < 3; ++i) { }
    int y = i;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'i'.
}
