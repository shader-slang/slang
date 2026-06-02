//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30052 (invalid-swizzle-expr) - invalid swizzle expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30052
//META: catalog_name=invalid-swizzle-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Swizzling with a letter outside the xyzw/rgba alphabet is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    float3 v = float3(1, 2, 3);
    float f = v.q;
}

// CHECK: E30052
