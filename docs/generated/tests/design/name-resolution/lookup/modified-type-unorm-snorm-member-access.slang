//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=fc857e43020ac66546c51043cd77e6a53aea17212ca602e37a98726239622ecc
//META: purpose=The other two modifier spellings that produce a modified type, unorm and snorm, are equally transparent to member lookup.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the remaining two of the three spellings that produce a
// modified type. `no_diff` is exercised by the companion functional
// test; this one instantiates the `unorm` and `snorm` cells so the
// claim is covered across the whole enumeration rather than at one
// representative. Each local is declared with a modifier in front of
// the same struct and then read through a field access; CHECK asserts
// the two distinct stored values come back, which requires lookup to
// have descended past the modifier to the struct's own member.

//TEST:INTERPRET(filecheck=CHECK):

struct Cell
{
    int n;
    int twice() { return n * 2; }
}

void main()
{
    unorm Cell a;
    snorm Cell b;
    a.n = 3;
    b.n = 5;
    // CHECK: unorm=6 snorm=10
    printf("unorm=%d snorm=%d\n", a.twice(), b.twice());
}
