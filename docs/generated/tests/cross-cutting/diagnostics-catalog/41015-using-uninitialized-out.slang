//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E41015 (using-uninitialized-out) - use of uninitialized out parameter
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41015
//META: catalog_name=using-uninitialized-out
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reading from an `out` parameter before assigning to it produces
// the uninitialized-out-parameter warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

void f(out int x) { o[0] = x; x = 0; }

[numthreads(1,1,1)]
void main() {
    int a;
    f(a);
}

// CHECK: E41015
