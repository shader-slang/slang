//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a2dda35f8eaa980e6313728c73ed55128c1eaf293b9bc4c13f1281db1f22547c
//META: purpose=Fires diagnostic E30704 (parameter-without-default-after-parameter-with-default) -- parameter '~param:Decl' does not have a default value, but follows a parameter that does
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30704
//META: catalog_name=parameter-without-default-after-parameter-with-default
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f(int a = 5, int b) {}
[shader("compute")][numthreads(1,1,1)]
void main() { f(1, 2); }


//CHECK: E30704
