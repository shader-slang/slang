//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#top-level-structure
//META: doc_section_digest=badf6daf5560f5144e7061f1568abcf04bfa7ece00a5e1441996814b2d18964f
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
