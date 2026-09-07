//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#root-hierarchy
//META: doc_section_digest=f096489235afb0ace7568d52e5958adc760834f2011fdaf7b734854a7c3fd937
//META: purpose=A scope is created during parsing and attached to the container declaration that owns it, so a member body can name a sibling member without qualification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a container declaration owns a lookup scope holding
// its members. Inside `clamped()`, the names `total` and `limit` are
// written bare; resolving them requires the enclosing struct's own
// scope to be on the lookup chain for the method body. CHECK asserts
// the clamped result, which can only be computed if both bare names
// found the sibling fields.

//TEST:INTERPRET(filecheck=CHECK):

struct Accum
{
    int total;
    int limit;

    int clamped() { return total < limit ? total : limit; }
};

void main()
{
    Accum a;
    a.total = 10;
    a.limit = 4;
    //CHECK: clamped=4
    printf("clamped=%d\n", a.clamped());
}
