//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=edef30f1b126016841d1f9779d5eda58e5cbe3faf3a7fcb75850d3c8c379039c
//META: purpose=Fires diagnostic E30065 (new-can-only-be-used-to-initialize-a-class) - `new` can only be used to initialize a class
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30065
//META: catalog_name=new-can-only-be-used-to-initialize-a-class
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `new` is reserved for reference-typed `class` declarations. Applying it to a
// value-typed `struct` is rejected, and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
struct S { int a; }
void main() { S s = new S(); }

//CHECK: E30065
