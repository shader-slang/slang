//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2e4d79b8a0c010c7da58273cbe657308c371dcbd50a97619283664523500b04e
//META: purpose=Fires diagnostic E30700 (output-parameter-cannot-have-default-value) -- an 'out' or 'inout' parameter cannot have a default-value expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30700
//META: catalog_name=output-parameter-cannot-have-default-value
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f(out int x = 5) {}
[shader("compute")][numthreads(1,1,1)]
void main() { int y; f(y); }


//CHECK: E30700
