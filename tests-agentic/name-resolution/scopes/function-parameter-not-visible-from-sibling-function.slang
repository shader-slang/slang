//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=A function parameter lives in its owning CallableDecl's scope; a sibling function cannot see it.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `square`'s parameter `n` lives in square's scope only. `main`'s
// scope chain does not pass through square's scope, so `n` is not
// resolvable from main.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int square(int n) { return n * n; }

void main()
{
    int r = square(7);
    int s = n;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'n'.
}
