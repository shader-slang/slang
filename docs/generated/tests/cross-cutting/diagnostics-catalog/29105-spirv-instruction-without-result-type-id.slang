//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E29105 (spirv-instruction-without-result-type-id) - instruction has no result-type-id operand
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29105
//META: catalog_name=spirv-instruction-without-result-type-id
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `result : <type> = OpReturn` triggers the no-result-type-id diagnostic
// because OpReturn does not have a <result-type-id> operand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    spirv_asm
    {
        result: $$int = OpReturn
    };
}

// CHECK: E29105
