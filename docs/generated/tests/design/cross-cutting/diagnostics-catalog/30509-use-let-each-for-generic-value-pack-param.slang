//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=18441e9cda8d212144b1ef3c325cbb8bb6c659d3a13009af1b49259d93d827ca
//META: purpose=Fires diagnostic E30509 (use-let-each-for-generic-value-pack-param) -- use 'let each' to declare a variadic generic value parameter: 'let each ~paramName:Name : ~type:Type'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30509
//META: catalog_name=use-let-each-for-generic-value-pack-param
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f<each T : int>() {}


// CHECK: E30509
