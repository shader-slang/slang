//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=341ea855b4cc01bfc612ef139eae7725ad7a366b921a54029383d73daf47f18f
//META: purpose=`UsingDecl` parses as `'using' 'namespace'? Expr ';'`, with the optional `namespace` keyword accepted.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `using namespace N;` spelling of UsingDecl. After the
// directive, a function declared inside the namespace is callable
// without qualification, which requires the whole decl (including the
// optional `namespace` keyword) to have parsed.

//TEST:INTERPRET(filecheck=CHECK):

namespace util
{
    int helper() { return 42; }
}

using namespace util;

void main()
{
    // CHECK: r=42
    printf("r=%d\n", helper());
}
