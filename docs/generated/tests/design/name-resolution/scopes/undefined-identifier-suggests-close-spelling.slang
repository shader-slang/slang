//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=b02f65c83e89f12616da6503b3f6980da4abbbb0a70dcf5a9c00e860440cb7fb
//META: purpose=When a VarExpr fails to resolve, the checker reuses the scope-chain traversal to find a close spelling and attaches a "did you mean" suggestion to the undefined-identifier diagnostic (E30015).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the findClosestInScopeName reuse of the scope walk: the typo
// `totalCont` is one edit from the parameter `totalCount` that sits on
// the scope chain, so the E30015 diagnostic must carry a "did you mean
// 'totalCount'?" suggestion naming the in-scope spelling.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int compute(int totalCount)
{
    return totalCont;
//CHECK:   ^^^^^^^^^ undefined identifier
//CHECK:   ^^^^^^^^^ undefined identifier 'totalCont'.
//CHECK:   ^^^^^^^^^ did you mean 'totalCount'?
// CHECK: E30015
}
