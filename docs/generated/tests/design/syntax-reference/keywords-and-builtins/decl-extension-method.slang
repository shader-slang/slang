//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=b49700d31492527c686bc2846076b0dd0110de840c0102148f8bbf876b2bcdb4
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
