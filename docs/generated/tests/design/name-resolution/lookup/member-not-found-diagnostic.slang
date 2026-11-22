//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=31abb67bf5286f69461146710f091d89f6d90d100b6c4de40ee80f2ebf88ef5a
//META: purpose=Member lookup that exhausts the facet walk without a candidate yields the member-not-found diagnostic (E30027) naming both the missing identifier and the host type.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the failure mode of member lookup: when no facet of the
// receiver type contributes a matching decl the result is empty and the
// caller diagnoses. CHECK annotates the short form, the interpolated
// form naming 'missing' and 'A', and the stable error code, so the test
// pins the identity of the diagnostic rather than its phrasing alone.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A
{
    int x;
}

void main()
{
    A a;
    int n = a.missing;
//CHECK:      ^^^^^^^ member not found
//CHECK:      ^^^^^^^ 'missing' is not a member of 'A'.
// CHECK: E30027
}
