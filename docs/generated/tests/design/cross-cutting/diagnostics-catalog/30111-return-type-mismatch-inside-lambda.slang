//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30111 (return-type-mismatch-inside-lambda) - lambda return type mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30111
//META: catalog_name=return-type-mismatch-inside-lambda
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A lambda that returns inconsistent types fires the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    let f = (bool b) =>
    {
        if (b) return 1;
        return 2.0;
    };
}

// CHECK: 30111
