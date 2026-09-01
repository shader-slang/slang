//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
//META: purpose=A `let`-bound local declared inside a block lives in the BlockStmt's scope only; it cannot be referenced from a later statement at the enclosing scope.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `let`-bound locals are ordinary VarDecls and live in the block's
// scope; they do not escape the BlockStmt that contains them.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    {
        let inner = 7;
    }
    int x = inner;
//CHECK:    ^^^^^ undefined identifier
//CHECK:    ^^^^^ undefined identifier 'inner'.
// CHECK: E30015
}
