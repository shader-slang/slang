//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E29100
