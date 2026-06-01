//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E38012 (entry-point-cannot-return-array-type) - entry point cannot return array type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38012
//META: catalog_name=entry-point-cannot-return-array-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An entry point that returns a fixed-size array triggers E38012.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage vertex -entry main -profile sm_5_0

typedef float ArrType[4];

ArrType main() : COLOR
{
    ArrType a;
    return a;
}

// CHECK: E38012
