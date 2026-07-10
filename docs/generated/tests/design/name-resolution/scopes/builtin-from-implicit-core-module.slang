//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=1730544025501f67efee33835c81f561c62ac7733b5e89832c526106e61b012f
//META: purpose=When a module is imported, the checker adds its scope as a sibling so names from it are reachable without explicit qualification; the core module's builtins (e.g. `sin`) are reachable through this mechanism.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sibling scopes use case 2 (imported modules): when module B
// imports module A, A's scope is linked as a sibling of B's so
// unqualified names resolve. The core module is implicitly available
// to every translation unit; `sin` resolves without explicit
// `core::sin` qualification.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float v = sin(0.0);
    // CHECK: ok
    printf("ok\n");
}
