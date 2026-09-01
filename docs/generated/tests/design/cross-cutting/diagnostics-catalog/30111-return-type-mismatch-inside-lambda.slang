//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ae5b43a46618dcb2670f3ecd3e6381d4862ffddeeb922e0a20f845187a7a2a58
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

//CHECK: 30111
