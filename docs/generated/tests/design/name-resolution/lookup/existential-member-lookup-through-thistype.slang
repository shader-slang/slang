//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=9574aa361d7b18b1da022473e0e53acd215b9af06dae0cb39490ca13610d03bf
//META: purpose=Member lookup on an interface-typed (existential) value targets the interface's implicit ThisType decl-ref, so the requirement is callable on the erased value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the existential arm of the member-lookup type dispatch. The
// local `c` is declared with the interface type, so its static type is
// an existential rather than the concrete struct; lookup must retarget
// at the interface's implicit self-type decl-ref for `c.val()` to
// resolve. CHECK asserts 5, produced by the concrete conformer stored
// in the existential.

//TEST:INTERPRET(filecheck=CHECK):

interface ICounter { int val(); }

struct S : ICounter { int val() { return 5; } }

void main()
{
    ICounter c = S();
    // CHECK: r=5
    printf("r=%d\n", c.val());
}
