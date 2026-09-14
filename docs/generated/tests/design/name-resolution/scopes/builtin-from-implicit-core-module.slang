//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=745e90bcc2440ac2accf9529f203ae4435a595ab30563e368559c64f8e6f978e
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
