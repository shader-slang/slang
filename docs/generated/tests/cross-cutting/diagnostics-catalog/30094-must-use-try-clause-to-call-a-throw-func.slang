//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30094 (must-use-try-clause-to-call-a-throw-func) - callee may throw, use 'try'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30094
//META: catalog_name=must-use-try-clause-to-call-a-throw-func
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A throws-function must be called inside a try clause.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int may_fail() throws int { throw 1; }

int wrapper() throws int
{
    int v = may_fail();
    return v;
}

void main() {}

// CHECK: 30094
