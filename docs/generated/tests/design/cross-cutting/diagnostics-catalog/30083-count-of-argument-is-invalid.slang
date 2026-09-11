//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e20c6f68d4ea25475818312597f9b1f0bc997ac2e5da8b296b776a62c4180f95
//META: purpose=Fires diagnostic E30083 (count-of-argument-is-invalid) - invalid countof argument
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30083
//META: catalog_name=count-of-argument-is-invalid
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// countof applied to a non-tuple non-pack scalar is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = 5;
    int c = countof(x);
}

//CHECK: 30083
