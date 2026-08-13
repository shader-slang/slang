//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
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
