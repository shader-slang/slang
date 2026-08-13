//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#transparent-members
//META: doc_section_digest=32382a9ebef19c50b9e06793af48bd9784fb821c105edada1ca8660859a10b64
//META: purpose=No parser keyword introduces a transparent modifier, so writing __transparent in front of a struct member is not recognised as a modifier and the declaration fails to parse.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that transparent-member injection has no user-writable
// spelling: the only producers are the buffer-block declaration
// keywords. Written in front of a member, `__transparent` is consumed
// as the member's type and `Inner` as its name, which leaves the real
// name with nowhere to go. CHECK annotates the resulting parse error
// at the trailing identifier, pinning both the unexpected-token report
// and the follow-on empty-declaration report.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Inner { int v; }

struct Outer
{
    __transparent Inner i;
//CHECK:                ^ unexpected token
//CHECK:                ^ unexpected identifier, expected ';'
//CHECK:                ^ declaration does not declare anything
}

void main()
{
    Outer o;
    printf("r=%d\n", o.i.v);
}
