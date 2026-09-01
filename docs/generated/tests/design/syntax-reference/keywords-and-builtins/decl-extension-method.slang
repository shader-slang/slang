//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`extension <Type> { ... }` is a decl keyword that adds new members to an existing type without modifying its primary declaration.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Add a `sum()` method to `Vec2` via an `extension` block and verify
// the new method is callable through normal member-access syntax.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec2 { int x; int y; };

extension Vec2
{
    int sum() { return x + y; }
}

void main()
{
    Vec2 v;
    v.x = 3;
    v.y = 4;
    // CHECK: s=7
    printf("s=%d\n", v.sum());
}
