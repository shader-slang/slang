//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=d6e74209f0cfcbd939127caa9bf2791387db9155c2d0b2f0a561349e2e185456
//META: purpose=Boundary: `defer` is documented in the statement-keywords table (line 6406) but uncovered by the smoke tests; the body of the deferred statement runs after the enclosing function body completes.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the `defer` statement keyword listed alongside `if`/`for`/
// `return`. The expected order ("before after") confirms the
// deferred body runs at scope exit, not at the point of declaration.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    defer printf("after");
    printf("before ");
    // CHECK: before after
}
