//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=A plain `switch` gives its whole body one scoped BlockStmt, so a local declared under one `case` label is still in scope in textually later cases of the same switch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive half of the plain-`switch` scoping rule: because the
// parser wraps the entire switch body in a single scoped BlockStmt rather
// than scoping each case group, `caseLocal` declared under `case 0` is the
// same declaration a textually later `case 1` sees. The selector picks
// `case 1`, which assigns through that name and copies it out. Printing
// `out=22` can only happen if the name resolved in the later case; if each
// case had its own scope the assignment would be an undefined identifier.
// The complementary negative — the name is gone after the switch — is
// switch-scope-local-not-visible-outside.slang.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sel = 1;
    int result = -1;
    switch (sel)
    {
    case 0:
        int caseLocal = 11;
        result = caseLocal;
        break;
    case 1:
        caseLocal = 22;
        result = caseLocal;
        break;
    }
    // CHECK: out=22
    printf("out=%d\n", result);
}
