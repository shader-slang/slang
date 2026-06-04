//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E32000 (invalid-enum-tag-type) - invalid enum tag type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=32000
//META: catalog_name=invalid-enum-tag-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a non-integer type (float) as the enum tag type triggers
// E32000.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

enum E : float
{
    A,
}

// CHECK: E32000
