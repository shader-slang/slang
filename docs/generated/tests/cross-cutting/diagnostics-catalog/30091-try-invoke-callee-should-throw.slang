//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30091 (try-invoke-callee-should-throw) - callee in 'try' does not throw
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30091
//META: catalog_name=try-invoke-callee-should-throw
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Wrapping a non-throwing call in `try` triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int safe() { return 0; }

int wrapper() throws int
{
    int v = try safe();
    return v;
}

void main() {}

// CHECK: 30091
