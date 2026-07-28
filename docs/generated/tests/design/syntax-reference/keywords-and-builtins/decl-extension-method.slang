//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=e3d06ef8bf7a4508b0ecb0515b3867979078821ac9b8143e7b039cd76e1b73b1
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
