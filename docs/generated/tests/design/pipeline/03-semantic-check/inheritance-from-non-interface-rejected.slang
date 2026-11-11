//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=945e0e7ae3cbf784bb9a33476a0f0098d325add73b8ef9ecaa04ffc5497af7ed
//META: purpose=Inheritance checking rejects a struct that names a sealed builtin type as its base (E30831).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "inheritance and extension lookup" concern. `int` is a sealed
// builtin, so listing it in an inheritance clause is not a legal base;
// CHECK pins E30831 plus the accompanying unstable-inheritance warning the
// same clause triggers.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Derived : int
{
    int d;
}
//CHECK: E30831
//CHECK: E30816

void main()
{
}
