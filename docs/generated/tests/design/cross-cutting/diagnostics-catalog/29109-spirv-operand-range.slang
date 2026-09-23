//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0b3961f2abb0dcdb02d77969692c3e1c814d3605b750d6a0a244076569d212ef
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

//CHECK: E29109
