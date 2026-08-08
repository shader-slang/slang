//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=326cb71980ffed90f76a958150baa07d0f93731cea05be675fa04981cf40db24
//META: purpose=Fires diagnostic E00062 (unknown-command-line-value) - unknown value for option '~option'. Valid values are '~validValues'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=62
//META: catalog_name=unknown-command-line-value
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Enumerated options reject values outside their documented value list. A
// made-up `-line-directive-mode` value fires the generic unknown-value error,
// which the CHECK pins by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-line-directive-mode bogusmode
[numthreads(1,1,1)]
void main() {}

//CHECK: E00062
