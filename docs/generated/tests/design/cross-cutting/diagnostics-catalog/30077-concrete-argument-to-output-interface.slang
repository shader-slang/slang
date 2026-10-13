//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=32a2d63e386f576d5b2a72b51415ca0a0ad30dcf44a5d15a0fd8b791c151f14f
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

//CHECK: 30077
