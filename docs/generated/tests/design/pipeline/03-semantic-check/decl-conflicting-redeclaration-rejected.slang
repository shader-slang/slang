//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=b5f523113b24470cb3eee4f9c8ae76906f41333e955cb7bb7bac14ed1e5d3a05
//META: purpose=Decl checking rejects two struct types declared with the same name as a conflicting declaration (E30200) and points at the previous declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "Decl checking" concern by exercising its name-collision rule
// at the type-declaration shape. Two `struct Dup` definitions occupy the same
// container, so the second must be reported as conflicting; CHECK pins E30200
// on the second name and the companion note that names the first.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Dup { int a; }

struct Dup { int b; }
//CHECK: E30200
//CHECK: see previous declaration of 'Dup'

void main()
{
}
