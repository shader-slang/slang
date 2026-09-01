//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
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
