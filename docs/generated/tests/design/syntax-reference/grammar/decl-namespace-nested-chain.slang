//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=341ea855b4cc01bfc612ef139eae7725ad7a366b921a54029383d73daf47f18f
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
