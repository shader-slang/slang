//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=677c751462cdb5ae5189c32a0f57ea5105199a379d69be08450135057a10b2db
//META: purpose=The ScopeDecl that `if (let x = ...)` creates for the unwrapped variable covers only the positive branch; the name is undefined after the if statement.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the if-let desugaring's synthetic scope: the
// ScopeDecl holding the unwrapped `v` wraps the positive branch only, so
// a read of `v` after the if statement must fail as an undefined
// identifier rather than seeing the (possibly absent) unwrapped value.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void probe(Optional<int> opt)
{
    if (let v = opt)
    {
    }
    int y = v;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'v'.
// CHECK: E30015
}
