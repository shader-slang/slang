//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=badf6daf5560f5144e7061f1568abcf04bfa7ece00a5e1441996814b2d18964f
//META: purpose=`NamespaceDecl` accepts a nested chain `IDENT (('.' | '::') IDENT)*` before the brace body.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the nested-namespace chain in a single declaration header,
// once with the '.' separator and once with '::'. Members are then
// reached through the full qualified path, proving each header built
// a two-level namespace chain rather than a single flat name.

//TEST:INTERPRET(filecheck=CHECK):

namespace outer.byDot
{
    int f() { return 40; }
}

namespace outer::byColons
{
    int g() { return 2; }
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", outer::byDot::f() + outer::byColons::g());
}
