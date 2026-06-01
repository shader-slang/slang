//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30084 (float-bit-cast-type-mismatch) - bit cast type mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30084
//META: catalog_name=float-bit-cast-type-mismatch
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// asuint() applied to a non-float bit-cast input triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int4 a = int4(1, 2, 3, 4);
    uint x = asuint(a);
}

// CHECK: 30084
