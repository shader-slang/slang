//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c9937e090f68861fd3cf6c2dd9e06e19656064efe27f6e29086f92a17cebd282
//META: purpose=Fires diagnostic E30076 (global-var-cannot-have-opaque-type) - global variable cannot have opaque type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30076
//META: catalog_name=global-var-cannot-have-opaque-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Opaque resource types are bound through parameter blocks, not through ordinary
// mutable module-scope storage. Declaring a `static Texture2D` global is
// rejected, and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
static Texture2D gTex;
void main() { }

//CHECK: E30076
