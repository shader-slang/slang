//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=62c13a3a598d1dca8af06c234d076ba76f18da8cccfe9e066177eb9316e939cd
//META: purpose=Fires diagnostic E30117 (forward-reference-in-generic-constraint) - forward reference in generic constraint
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30117
//META: catalog_name=forward-reference-in-generic-constraint
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Constraint that mentions a generic parameter declared later triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IThing {}

void f<T : U, U : IThing>() {}

void main() {}

//CHECK: 30117
