//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T07:55:45Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: 30083
