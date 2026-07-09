//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=An ImportDecl that names a module that cannot be found is diagnosed at parse/check time.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of the ImportDecl role. An import of
// a module that does not exist on the search path is rejected with
// a diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import this_module_does_not_exist_zzz;
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cannot open file 'this_module_does_not_exist_zzz.slang'
*/

void main() {}
