//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30077 (concrete-argument-to-output-interface) - concrete type passed to interface-typed output parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30077
//META: catalog_name=concrete-argument-to-output-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Passing a concrete struct as an `out` parameter typed to an interface is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IThing { int get(); }
struct Impl : IThing { int get() { return 42; } };

void produce(out IThing t) {}

void main()
{
    Impl x;
    produce(x);
}

// CHECK: 30077
