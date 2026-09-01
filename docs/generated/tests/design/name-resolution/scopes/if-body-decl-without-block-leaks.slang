//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=A declaration used directly as an `if` body — with no braces — is accepted rather than rejected, and the name leaks into the enclosing scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same "no braces, no scope" rule under `if`, the shape the
// scope-bearing-nodes discussion names alongside `while` and `do`. `IfStmt`
// owns no scope of its own, so the brace-less declaration goes into the
// enclosing function's container. The branch is taken, so the initializer
// runs and the value is observable after the statement: printing `x=7`
// requires both that the name still resolves and that the declaration it
// resolves to is the one written inside the branch.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int c = 1;
    if (c > 0) int x = 7;
    // CHECK: x=7
    printf("x=%d\n", x);
}
