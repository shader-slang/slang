//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=ea81eee9c248c045a204cf03769d29438ea6b24a6cbd441a4c42b3efc37f9f53
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
