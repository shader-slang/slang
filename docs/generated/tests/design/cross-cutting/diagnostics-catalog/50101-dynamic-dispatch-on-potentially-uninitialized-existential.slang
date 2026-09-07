//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d92a8f589bdeea318efa6d8bcf2c0249182c81b3ae9b309eb67bbe9acdbed6bd
//META: purpose=Fires diagnostic E50101 (dynamic-dispatch-on-potentially-uninitialized-existential) - cannot dispatch on uninitialized interface
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=50101
//META: catalog_name=dynamic-dispatch-on-potentially-uninitialized-existential
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Calling a method on an interface-typed variable that has not been
// initialized to a concrete type triggers the uninitialized-
// existential dispatch error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

interface IFoo { int get(); }
struct A : IFoo { int v; int get() { return v; } }

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() {
    IFoo p;
    o[0] = p.get();
}

//CHECK: E50101
