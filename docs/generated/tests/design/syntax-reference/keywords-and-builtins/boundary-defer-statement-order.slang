//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=4e5c787f52095c6dbdd20c8c0f4df226f6e2e9bbef0e89d6a888ac6bb5940443
//META: purpose=Boundary: `defer` is documented in the statement-keywords table but uncovered by the smoke tests; the body of the deferred statement runs after the enclosing function body completes.
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
