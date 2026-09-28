//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=80b132e0784d50c06f30bc2f3a7a43329a7bb0d041844954a8801b64e87bf476
//META: purpose=Fires diagnostic E15401 (macro-not-defined) - undefined macro
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15401
//META: catalog_name=macro-not-defined
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#undef` on a name that was never defined is harmless but warned about. The
// CHECK pins the undefined-macro warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#undef NOT_DEFINED_XYZ
void main() {}

//CHECK: E15401
