//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=5ed89b211beca6ec6df445f7d63a9889da0768b35a383df6c7c268b0424663dc
//META: purpose=An `internal` decl is visible from any file in the same module; a free function calls another module-local `internal` function successfully.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per-keyword semantics: `internal` is visible inside the declaring
// module. Two free functions in the same module-compilation see
// each other through `getModuleDecl(scope)` equality.

//TEST:INTERPRET(filecheck=CHECK):

internal int helper(int x) { return x + 100; }

void main()
{
    // CHECK: helper=142
    printf("helper=%d\n", helper(42));
}
