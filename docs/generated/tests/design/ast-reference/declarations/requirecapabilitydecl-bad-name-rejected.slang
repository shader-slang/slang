//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A RequireCapabilityDecl with a non-existent capability name is diagnosed; the decl carries through to capability resolution.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of the RequireCapabilityDecl role: an
// unknown capability name in a `__require_capability` declaration is
// rejected. The parser resolves each named capability and diagnoses
// E36105 when the name is not a known capability atom.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

__require_capability ThisIsANonExistingCapability;
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unknown capability
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unknown capability name 'ThisIsANonExistingCapability'.

void main() {}
