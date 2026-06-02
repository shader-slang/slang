//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:32+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-pointer.md#syntax
//META: doc_section_digest=7d0d8d99e81aaca2b6f41962be8f0bffe859a83dddeba6909e61b74e024ba2ff
//META: purpose=For a pointer to a struct, `p->member` and `(*p).member` both access the same struct member; mutation through `->` is visible in the original.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `p->member` and `(*p).member` both access the same struct field.
// Takes a pointer to a `Vec2`, mutates `p->x`, then reads both fields via both
// syntaxes; CHECK asserts the updated and untouched fields have the expected values,
// confirming arrow-access and dot-dereference are equivalent.
//TEST:INTERPRET(filecheck=CHECK):

struct Vec2
{
    int x;
    int y;
}

void main()
{
    Vec2 v;
    v.x = 11;
    v.y = 22;
    Vec2* p = &v;
    p->x = 99;
    //CHECK: 99 22
    printf("%d %d\n", p->x, (*p).y);
}
