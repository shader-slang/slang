//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f64689ff5b6cb29926b1f0602498333f390b94257df1ba47701e18e6251e16ad
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

//CHECK: E38012
