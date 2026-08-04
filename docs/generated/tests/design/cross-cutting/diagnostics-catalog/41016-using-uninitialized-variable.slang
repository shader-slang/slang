//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E41016 (using-uninitialized-variable) - use of uninitialized variable
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41016
//META: catalog_name=using-uninitialized-variable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function that returns a local variable without initializing it
// triggers the uninitialized-variable warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute

int f() { int x; return x; }

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    o[0] = f();
}

// CHECK: E41016
