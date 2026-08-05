//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#deduplication
//META: doc_section_digest=802de1508256e26daf66c46a07bf5d1f9a2f9a83d5b8d75510a0d79a00768b98
//META: purpose=The facet list is deduplicated by origin, so a base interface reached through two inheritance paths contributes one facet and its requirement is not ambiguous.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the within-member-lookup deduplication the doc contrasts
// with the absence of deduplication across lookup paths. IBase is
// reachable through both ILeft and IRight, so without origin-based
// deduplication `t.b()` would collect the same requirement twice and
// the call would need narrowing to survive. CHECK asserts 3, so the
// call resolved cleanly through the single merged facet.

//TEST:INTERPRET(filecheck=CHECK):

interface IBase { int b(); }
interface ILeft : IBase {}
interface IRight : IBase {}
interface IBoth : ILeft, IRight {}

struct D : IBoth { int b() { return 3; } }

int go<T : IBoth>(T t) { return t.b(); }

void main()
{
    D d;
    // CHECK: r=3
    printf("r=%d\n", go(d));
}
