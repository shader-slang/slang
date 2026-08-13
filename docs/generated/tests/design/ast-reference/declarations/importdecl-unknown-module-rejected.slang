//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=An ImportDecl naming a module that cannot be found on the search path is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an import is resolved against real files rather than recorded
// as an unchecked name. The imported module does not exist; the CHECK pins the
// file-not-found diagnostic under the module name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

import this_module_does_not_exist_zzz;
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cannot open file 'this_module_does_not_exist_zzz.slang'
*/

void main() {}
