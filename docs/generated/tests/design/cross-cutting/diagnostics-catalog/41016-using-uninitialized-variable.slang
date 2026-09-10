//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=22312e823abb4258939dc0af9b66fb500b02178361938b9f3b72f741795e8022
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

//CHECK: E41016
