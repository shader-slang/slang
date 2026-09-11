//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=9574aa361d7b18b1da022473e0e53acd215b9af06dae0cb39490ca13610d03bf
//META: purpose=An inherited This candidate is suppressed for an interface, so a derived interface that also inherits This from its base can still name This without an ambiguity.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the special handling of an inherited This during the facet
// walk. IB inherits IA, so both interfaces contribute a self-type
// declaration; without the suppression the `This dup()` requirement in
// IB would see two candidates for This and fail to resolve. CHECK
// asserts 6 * 2, which requires IB to compile and the requirement to
// dispatch to the conformer.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int a(); }
interface IB : IA { This dup(); }

struct S : IB
{
    int k;
    int a() { return k; }
    This dup() { S s; s.k = k * 2; return s; }
}

int use<T : IB>(T t) { return t.dup().a(); }

void main()
{
    S s;
    s.k = 6;
    // CHECK: r=12
    printf("r=%d\n", use(s));
}
