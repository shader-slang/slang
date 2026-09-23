//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2c43df85b9a2be6b302619c3130cb5c5cc6a0b5ed69e0f36fc8024cd8ba6e99c
//META: purpose=Fires diagnostic E29100 (unrecognized-spirv-opcode) - unrecognized SPIR-V opcode
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29100
//META: catalog_name=unrecognized-spirv-opcode
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A bogus opcode in spirv_asm fires the unrecognized-opcode diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    int x = spirv_asm
    {
        result: $$int = OpNotARealOpcodeFromSpec $(0)
    };
}

//CHECK: E29100
