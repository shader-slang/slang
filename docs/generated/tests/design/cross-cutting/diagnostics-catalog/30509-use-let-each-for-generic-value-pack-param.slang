//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1f9f8cf229b33faeb639cdc8943e888040a9eda1888ba51c71972a8166965c83
//META: purpose=Fires diagnostic E30509 (use-let-each-for-generic-value-pack-param) -- use 'let each' to declare a variadic generic value parameter: 'let each ~paramName:Name : ~type:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30509
//META: catalog_name=use-let-each-for-generic-value-pack-param
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<each T : int>() {}


//CHECK: E30509
