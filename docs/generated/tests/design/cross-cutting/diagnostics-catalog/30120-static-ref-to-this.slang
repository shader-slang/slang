//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c30676792ca6f1372f6890c3fd562187e4a7cabbe9ae1f4e4b3e18f0608adae7
//META: purpose=Fires diagnostic E30120 (static-ref-to-this) - static function cannot refer to non-static member via `this`
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30120
//META: catalog_name=static-ref-to-this
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `this` referenced from a static method triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int x;
    static int read() { return this.x; }
};

void main() {}

//CHECK: 30120
