//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=4012d317b8285bf4455042f74ea907029640a1b43e84b980a12945df5545dd84
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
