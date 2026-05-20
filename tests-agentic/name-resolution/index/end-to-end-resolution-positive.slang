//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/name-resolution/index.md#name-resolution
//META: doc_section_digest=923fa8c2984b966fc030eec580a22d4d2a441d740d5356eaeac7f165da7de96b
//META: purpose=Name resolution turns an identifier in source text into a resolved DeclRef whose value is observable at runtime.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc opens by stating that the subtree's job is to turn
// "an identifier in source text into a resolved DeclRef". The
// cross-cutting observable for that claim is that some identifier,
// referenced from a non-trivial position, ultimately produces the
// value declared at the resolved decl. This test exercises that
// minimal end-to-end shape: scope walk + unqualified lookup +
// (implicit public) visibility + (single-candidate) overload all
// compose into a working call.

//TEST:INTERPRET(filecheck=CHECK):

int produce()
{
    return 42;
}

void main()
{
    // CHECK: v=42
    printf("v=%d\n", produce());
}
