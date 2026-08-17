//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bad520bcaead7629ec850f194d53265302fd994ecb437005e7e05913ca666434
//META: purpose=Fires diagnostic E33076 (cannot-have-differentiable-method-in-dyn-interface) - dyn interfaces cannot have [Differentiable] methods
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=33076
//META: catalog_name=cannot-have-differentiable-method-in-dyn-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `dyn` interface containing a [Differentiable] method is
// rejected under Slang 2026 semantics.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

dyn interface IFoo
{
    [Differentiable]
    float f(float v);
}

//CHECK: E33076
