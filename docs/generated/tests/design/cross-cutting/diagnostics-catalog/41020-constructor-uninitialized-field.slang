//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=088d9d2f2741dd5c4d695f6838328fd89b78da5993bc40fb010c1a155ab7d514
//META: purpose=Fires diagnostic E41020 (constructor-uninitialized-field) - exiting constructor without initializing field
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41020
//META: catalog_name=constructor-uninitialized-field
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A user-defined constructor that exits without assigning all
// fields triggers the constructor-uninitialized-field warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

struct S {
    int a;
    int b = 5;
    __init() {}
}

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    S s = S();
    o[0] = s.b;
}

//CHECK: E41020
