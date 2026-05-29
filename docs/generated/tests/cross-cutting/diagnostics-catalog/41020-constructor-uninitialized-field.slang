//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E41020
