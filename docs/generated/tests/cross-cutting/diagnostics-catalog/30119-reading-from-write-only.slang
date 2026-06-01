//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30119 (reading-from-write-only) - cannot read from writeonly
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30119
//META: catalog_name=reading-from-write-only
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reading from a `writeonly` local variable is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[shader("compute")]
[numthreads(1,1,1)]
void main(uniform RWStructuredBuffer<int> buf : register(u0))
{
    writeonly int x = 5;
    int y = x;
}

// CHECK: 30119
