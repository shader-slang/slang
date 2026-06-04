//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E41202 (not-equal-bit-cast-size) - bit_cast size mismatch
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41202
//META: catalog_name=not-equal-bit-cast-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `bit_cast` between types of different sizes (int64 -> int) is
// rejected with a size-mismatch error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    int64_t big = int64_t(tid.x);
    int small = bit_cast<int>(big);
    o[0] = small;
}

// CHECK: E41202
