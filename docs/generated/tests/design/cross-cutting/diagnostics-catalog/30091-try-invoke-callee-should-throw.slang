//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cf387c159c8d64ac7d53e868da6453ef9704bf6842edfb061f27bae72d35824d
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

//CHECK: 30091
