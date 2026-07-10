//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=341ea855b4cc01bfc612ef139eae7725ad7a366b921a54029383d73daf47f18f
//META: purpose=`ModuleName` accepts a `STRING_LIT` alternative in a `module` header.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the string-literal module-name spelling: `module "name";`
// uses a STRING_LIT (which can carry characters an identifier cannot,
// like `-`) and the file still parses and runs.

//TEST:INTERPRET(filecheck=CHECK):

module "grammar-bundle-string-name";

void main()
{
    // CHECK: ok=3
    printf("ok=%d\n", 3);
}
