//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=341ea855b4cc01bfc612ef139eae7725ad7a366b921a54029383d73daf47f18f
//META: purpose=`ModuleName` accepts the dotted-identifier form `IDENT ('.' IDENT)*` in a `module` header.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the dotted module-name spelling: a `module a.b.c;` header
// must be consumed as one ModuleName chain, leaving the following
// declarations to parse and run normally.

//TEST:INTERPRET(filecheck=CHECK):

module grammar.bundle.dotted;

void main()
{
    // CHECK: ok=2
    printf("ok=%d\n", 2);
}
