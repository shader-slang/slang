//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E29112 (spirv-instruction-with-not-enough-operands) - not enough operands
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29112
//META: catalog_name=spirv-instruction-with-not-enough-operands
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// OpIAdd requires two operands; supplying only one fires the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    int x = spirv_asm
    {
        result: $$int = OpIAdd $(1)
    };
}

// CHECK: E29112
