//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=ForStmt owns its own scope; the init-statement's variable is not visible after the loop.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ForStmt is listed as scope-bearing (in contrast to UnscopedForStmt
// in HLSL mode). The loop variable lives in the ForStmt's scope only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    for (int i = 0; i < 3; ++i)
    {
    }
    int z = i;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'i'.
// CHECK: E30015
}
