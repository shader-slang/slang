//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=An ImportDecl that names a module that cannot be found is diagnosed at parse/check time.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of the ImportDecl role. An import of
// a module that does not exist on the search path is rejected with
// a diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import this_module_does_not_exist_zzz;

void main() {}
//CHECK: error
//CHECK: E15301
