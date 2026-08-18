//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
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
