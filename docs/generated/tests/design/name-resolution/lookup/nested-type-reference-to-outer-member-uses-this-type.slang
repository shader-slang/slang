//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=d1c02da07b357c8b41134fbb13a153c9451e991db3087032b3d340a5ddfdfa75
//META: purpose=From a nested type, an unqualified name that is a static member of the enclosing type resolves through a This-type breadcrumb (no this value), while an instance member of the enclosing type is rejected as non-static.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the nested-type arm of the this-breadcrumb kind. Stepping
// outward past `Inner` (an AggTypeDeclBase) switches the breadcrumb
// from the this-value form to the this-type form, so `k` resolves as
// `Outer::k` while `x`, an instance member, cannot be reached with
// only the type: the checker reports E30100.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Outer
{
    int x;
    static const int k = 3;
    struct Inner
    {
        int f() { return k; }
        int g() { return x; }
//CHECK:                 ^ type cannot refer to non-static member
//CHECK:                 ^ type 'Outer' cannot be used to refer to non-static member 'x'
// CHECK: E30100
    }
}

void main()
{
    Outer::Inner i;
    int r = i.f() + i.g();
}
