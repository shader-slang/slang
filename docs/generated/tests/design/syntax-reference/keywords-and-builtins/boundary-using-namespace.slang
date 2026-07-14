//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=e3d06ef8bf7a4508b0ecb0515b3867979078821ac9b8143e7b039cd76e1b73b1
//META: purpose=Boundary: `using` is listed in the decl-keywords table (`parseUsingDecl`); `using namespace N;` brings `N`'s members into the current scope so they may be looked up unqualified.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `decl-namespace-scope.slang`, which exercises only the
// `::`-qualified form. This test exercises the `using` declaration
// itself.

//TEST:INTERPRET(filecheck=CHECK):

namespace lib { int get() { return 42; } }
using namespace lib;

void main()
{
    int v = get();
    // CHECK: v=42
    printf("v=%d\n", v);
}
