//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=5cf1a0be2720fdb7640ba4534ce97ec9428cc65b37576f2b89297596bf873eb6
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
