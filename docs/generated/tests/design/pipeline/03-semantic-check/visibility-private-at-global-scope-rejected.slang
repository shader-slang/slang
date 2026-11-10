//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#visibility-scopes
//META: doc_section_digest=fc40943abb8d4c479b692d39171c3acf02766b831d8fe5456b3c43f7df6d3a63
//META: purpose=Writing `private` where there is no enclosing type for it to scope to — at global scope — is rejected up front (E30603).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the companion rule to type-scoped `private`: because the scope the
// modifier names is a type, a declaration with no enclosing type gives it
// nothing to mean, and that is rejected at the declaration rather than deferred
// to a later failed access. CHECK pins E30603 and the message explaining that
// the declaration is not a member of a type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

private int g = 1;
//CHECK: E30603
//CHECK: cannot have private visibility because it is not a member of a type.

void main()
{
}
