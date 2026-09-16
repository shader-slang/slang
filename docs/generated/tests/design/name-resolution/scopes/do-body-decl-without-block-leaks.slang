//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=c25b4949da1f02764ce1c67f1094b0f0f36902472c4e7072dce4b3577317245e
//META: purpose=A declaration used directly as a `do` body — with no braces — is accepted rather than rejected, and the name leaks into the enclosing scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third shape of the brace-less-body rule. A `do` body runs at
// least once, so the declaration's initializer executes, and because
// `DoWhileStmt` gets no fresh ScopeDecl the declaration lands in the
// enclosing function's container. Printing `y=5` after the loop shows both
// halves: the name is still bound after the statement, and it is bound to
// the declaration written as the loop body.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int c = 0;
    do int y = 5; while (c > 0);
    // CHECK: y=5
    printf("y=%d\n", y);
}
