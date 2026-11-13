//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ba0fbb59f19c90964f30f63e36bd6d872a6caad870e1c39a6b2b4f2f5f9714ca
//META: purpose=A switch body parses as a scoped BlockStmt (the parser does not assign SwitchStmt::scopeDecl); a local declared inside one of its cases is not visible after the switch.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a switch's case-local does not escape the statement: the
// scope-bearing-nodes table says the parser gives the switch body a scoped
// BlockStmt rather than populating SwitchStmt::scopeDecl. Reading the
// case-local after the switch must fail with an undefined identifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 1;
    switch (x)
    {
    case 0:
        int caseLocal = 99;
        break;
    case 1:
        break;
    }
    int y = caseLocal;
//CHECK:    ^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^ undefined identifier 'caseLocal'.
// CHECK: E30015
}
