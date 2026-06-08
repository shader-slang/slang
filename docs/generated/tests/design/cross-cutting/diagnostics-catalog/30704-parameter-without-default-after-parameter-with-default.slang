//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=90515e39946c6c0aaf3bb3944720235c740a17183dcf5e70ad5578c650153888
//META: purpose=Fires diagnostic E30704 (parameter-without-default-after-parameter-with-default) -- parameter '~param:Decl' does not have a default value, but follows a parameter that does
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30704
//META: catalog_name=parameter-without-default-after-parameter-with-default
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f(int a = 5, int b) {}
[shader("compute")][numthreads(1,1,1)]
void main() { f(1, 2); }


// CHECK: E30704
