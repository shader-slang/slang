//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A RequireCapabilityDecl is a module-level declaration that states a capability requirement; the module compiles when the target capability is met.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of RequireCapabilityDecl: it is
// expressed as a declaration (so it can be exported), and a module
// that requires a capability the target supports still compiles and
// runs.

//TEST:INTERPRET(filecheck=CHECK):

__require_capability(cpp);

void main()
{
    //CHECK: ok
    printf("ok\n");
}
