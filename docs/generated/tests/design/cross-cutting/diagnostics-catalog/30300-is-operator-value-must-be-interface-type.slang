//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30300 (is-operator-value-must-be-interface-type) - 'is'/'as' operator requires interface-typed expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30300
//META: catalog_name=is-operator-value-must-be-interface-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying `is` to a primitive (non-interface) value triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IThing {}

void main()
{
    int x = 0;
    bool b = x is IThing;
}

// CHECK: E30300
