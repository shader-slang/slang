//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=b22ae0db87266fb01a1be1ff4487019df27480bf09ff6476d416556c6388f8d4
//META: purpose=The synthetic tag-type inheritance on enums carries IgnoreForLookupModifier so the underlying integer type's members are not surfaced as enum members; access to such a member produces 'member not found'.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's failure-modes section: "`IgnoreForLookupModifier` on a
// base. The synthetic tag-type inheritance on enums carries this
// modifier ... so the underlying integer type is not surfaced as a
// base interface when looking up enum members." Verify by trying
// to access an integer-side method through an enum value — the
// inheritance walk skips the int-tag facet and the lookup fails
// with `member not found`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum Color : int { Red, Green, Blue }

void main()
{
    Color c = Color::Red;
    int v = c.asInt();
//CHECK:      ^^^^^ member not found
//CHECK:      ^^^^^ 'asInt' is not a member of 'Color'.
}
