//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: 30120
