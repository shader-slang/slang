//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E29109 (spirv-operand-range) - literal integer out of range
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29109
//META: catalog_name=spirv-operand-range
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A literal that does not fit in 0..0xFFFFFFFF fires the operand range diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    int x = spirv_asm
    {
        result: $$int = OpCopyObject 999999999999999999
    };
}

// CHECK: E29109
