//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E40008 (invalid-l-value-for-ref-parameter) - the form of this l-value argument is not valid for a `ref` parameter
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=40008
//META: catalog_name=invalid-l-value-for-ref-parameter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Passing a temporary as the argument of a `ref` parameter
// triggers E40008.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_6_0

void g(ref int x) { x = 5; }

[numthreads(1,1,1)]
void main()
{
    g(5);
}

// CHECK: E40008
