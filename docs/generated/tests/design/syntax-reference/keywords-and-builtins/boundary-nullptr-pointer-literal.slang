//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=96871287e0a5c71c8f6df1013470b534356880ae94ba849477f98d1e5a7bf53f
//META: purpose=Boundary: `nullptr` is documented in the expression-keywords table as the null pointer literal; it compares equal to itself when stored in a pointer-typed binding.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Covers the `nullptr` expression keyword that was not exercised by
// the smoke tests. A pointer-typed local set to `nullptr` compares
// equal to `nullptr`, returning 1.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int* p = nullptr;
    // CHECK: eq=1
    printf("eq=%d\n", (p == nullptr) ? 1 : 0);
}
