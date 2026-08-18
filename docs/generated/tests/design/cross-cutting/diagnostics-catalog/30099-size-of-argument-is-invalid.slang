//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5153d635b895ef903c8e26e20d4ab833bde609da3b0a3cb0c595744c4cfb3d90
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

//CHECK: 30099
