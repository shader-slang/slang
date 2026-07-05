//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A RequireCapabilityDecl is a module-level declaration that states a capability requirement; the module compiles when the target capability is met.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of RequireCapabilityDecl: it is
// expressed as a declaration (so it can be exported), and a module
// that requires a capability the target supports still compiles and
// runs.

//TEST:INTERPRET(filecheck=CHECK):

__require_capability cpp;

void main()
{
    //CHECK: ok
    printf("ok\n");
}
