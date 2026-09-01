//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=4c78ab08cf659f53d04e87356fa441615410f33a23152c70e9fb93b377efd998
//META: purpose=The synthetic tag-type inheritance on an enum is filtered out of the facet walk, so the underlying integer type's members are not reachable as enum members (E30027).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the enum-to-tag-type inheritance is excluded from
// lookup. Color declares int as its tag type, so a facet for int would
// otherwise surface int's members through an enum value. Reaching for
// one of those members must fail exactly like any absent member; CHECK
// annotates the member-not-found diagnostic naming 'Color' as the host
// type, which shows the int facet never contributed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum Color : int { Red, Green, Blue }

void main()
{
    Color c = Color::Red;
    int v = c.asInt();
//CHECK:      ^^^^^ member not found
//CHECK:      ^^^^^ 'asInt' is not a member of 'Color'.
// CHECK: E30027
}
