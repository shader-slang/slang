//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=badf6daf5560f5144e7061f1568abcf04bfa7ece00a5e1441996814b2d18964f
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
