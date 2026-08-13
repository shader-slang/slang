//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A RequireCapabilityDecl (`__require_capability`) is a module-scope declaration; a module requiring a capability the target provides still compiles.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a capability requirement is expressed as a declaration and is
// satisfied when the target provides the named capability. The module requires
// `cpp` and is run on the interpreter's CPU-backed path; the CHECK asserts the
// program's output, which only appears if the requirement was accepted.

//TEST:INTERPRET(filecheck=CHECK):

__require_capability cpp;

void main()
{
    //CHECK: ok
    printf("ok\n");
}
