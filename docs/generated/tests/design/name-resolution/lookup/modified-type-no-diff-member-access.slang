//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=fc857e43020ac66546c51043cd77e6a53aea17212ca602e37a98726239622ecc
//META: purpose=A type modifier is transparent to member lookup, so a field and a method of a value declared no_diff S resolve exactly as they do on the unmodified struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the modified-type arm of the member-lookup type dispatch.
// `no_diff` is one of the three spellings the parser lifts off the
// declaration and onto its type expression, so the static type of `s`
// is the struct wrapped in a modifier rather than the plain struct.
// The test reads a field and calls a method through that value; CHECK
// asserts both resolve to the struct's own members (8 = v+1 from the
// method, 7 = the field itself), which is only possible if the walk
// runs on the modified type's underlying declaration.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int v;
    int get() { return v + 1; }
}

void main()
{
    no_diff S s;
    s.v = 7;
    // CHECK: method=8 field=7
    printf("method=%d field=%d\n", s.get(), s.v);
}
