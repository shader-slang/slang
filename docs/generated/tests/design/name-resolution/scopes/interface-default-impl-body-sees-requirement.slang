//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=ea81eee9c248c045a204cf03769d29438ea6b24a6cbd441a4c42b3efc37f9f53
//META: purpose=A default-implementation body inside an interface parses against a scope whose chain reaches the interface's requirement list, so a sibling requirement binds unqualified.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the derived scope a default-implementation body parses
// against still reaches the interface's single requirement scope. `total`
// carries a default body that calls the sibling requirement `base()` with
// no qualification; the conformer only implements `base`, so the value a
// generic sees through `total()` comes from the default body. CHECK asserts
// 105 (the conformer's 5 plus the default body's 100), which requires the
// unqualified `base` inside the default body to have bound to the
// requirement declared next to it.

//TEST:INTERPRET(filecheck=CHECK):

interface ICounter
{
    int base();
    int total() { return base() + 100; }
}

struct C : ICounter
{
    int base() { return 5; }
}

int totalOf<T : ICounter>(T c)
{
    return c.total();
}

void main()
{
    C c;
    // CHECK: t=105
    printf("t=%d\n", totalOf(c));
}
