//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=9574aa361d7b18b1da022473e0e53acd215b9af06dae0cb39490ca13610d03bf
//META: purpose=The linearized facet list includes transitive super-interfaces, so a method declared on the topmost interface of a three-level chain is reachable through a constraint naming only the bottom one.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the facet walk at inheritance depth three: IA <- IB <- IC,
// with the generic constrained only to IC. `aval()` is declared on IA,
// so the call resolves only if the linearized facet list carries the
// transitive interface facets rather than just the direct base. CHECK
// asserts 17, the conformer's implementation of that requirement.

//TEST:INTERPRET(filecheck=CHECK):

interface IA { int aval(); }
interface IB : IA {}
interface IC : IB {}

struct Impl : IC
{
    int aval() { return 17; }
}

int via<T : IC>(T t) { return t.aval(); }

void main()
{
    Impl i;
    // CHECK: r=17
    printf("r=%d\n", via(i));
}
