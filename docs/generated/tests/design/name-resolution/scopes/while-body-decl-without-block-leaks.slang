//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=c25b4949da1f02764ce1c67f1094b0f0f36902472c4e7072dce4b3577317245e
//META: purpose=A declaration used directly as a `while` body — with no braces — is accepted rather than rejected, and the name leaks into the enclosing scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "no braces, no scope" rule for a loop body: because a
// `WhileStmt` never gets a fresh ScopeDecl of its own, a declaration written
// directly as its body is inserted into the *enclosing* container, so the
// name survives the loop. The loop condition is false, so the declaration
// never executes; the test then assigns to `x` and prints it after the loop.
// Both the assignment and the read would be undefined-identifier errors if
// the statement had isolated the name, so `x=42` proves the leak.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int c = 0;
    while (c > 0) int x = 1;
    x = 42;
    // CHECK: x=42
    printf("x=%d\n", x);
}
