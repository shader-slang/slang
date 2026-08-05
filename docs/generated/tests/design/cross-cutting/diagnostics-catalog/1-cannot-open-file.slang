//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e708d290fcc21b870817185d7a347a04fa94c4ddecd02b9f6d2a071d84532a71
//META: purpose=Fires diagnostic E1 (cannot-open-file) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=1
//META: catalog_name=cannot-open-file
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E1.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
import nonexistent_module_xyz_pqrs;
void main() {}

//CHECK: E00001
