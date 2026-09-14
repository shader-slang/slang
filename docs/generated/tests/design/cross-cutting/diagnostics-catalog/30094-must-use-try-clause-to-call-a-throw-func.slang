//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d8bc0e348685b8bb4eac93e0abfe84ba60c2b0bdd81085fd0269bdf3e36bf377
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

//CHECK: 30094
