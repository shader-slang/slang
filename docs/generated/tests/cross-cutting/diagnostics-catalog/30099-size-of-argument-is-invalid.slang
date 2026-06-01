//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30099 (size-of-argument-is-invalid) - invalid sizeof argument
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30099
//META: catalog_name=size-of-argument-is-invalid
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying sizeof to a function reference is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void foo() {}

void main()
{
    int s = sizeof(foo);
}

// CHECK: 30099
