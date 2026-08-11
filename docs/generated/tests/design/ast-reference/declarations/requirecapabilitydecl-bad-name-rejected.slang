//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A `__require_capability` naming a capability that does not exist is diagnosed at the name.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the capability names in a requirement declaration are resolved
// rather than stored verbatim. An invented name is used; the CHECKs pin the
// unknown-capability diagnostic spanning the name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

__require_capability ThisIsANonExistingCapability;
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unknown capability
//CHECK:             ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unknown capability name 'ThisIsANonExistingCapability'.

void main() {}
