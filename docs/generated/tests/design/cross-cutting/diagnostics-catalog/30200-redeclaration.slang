//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=942c3d0588d0c1fbb49ba049b98bce7f0ed1e4964f1316f3c6e639ac6c8452cf
//META: purpose=Fires diagnostic E30200 (redeclaration) - conflicting declaration
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30200
//META: catalog_name=redeclaration
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two declarations of the same name in the same scope conflict unless they are
// overloads. Two variables differing only in type are not overloads, so the
// CHECK pins the redeclaration error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
int gDup;
float gDup;
void main() {}

//CHECK: E30200
