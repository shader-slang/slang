//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=75f3ce830a8be3ceee10f296c745651bc662abc9ccc71a004fe24b61377b4051
//META: purpose=Fires diagnostic E30081 (unrecommended-implicit-conversion) - implicit conversion not recommended
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30081
//META: catalog_name=unrecommended-implicit-conversion
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Some implicit conversions are legal but lossy. Initializing an `int` from a
// `float` narrows, so the checker warns rather than errors; the CHECK pins the
// unrecommended-conversion warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { float f = 1.5; int i = f; }

//CHECK: E30081
