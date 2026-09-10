//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=85b4881c409db1dc7ecad2297af95cfb63d252e61195d75a89a5f3c6222105a6
//META: purpose=Fires diagnostic E29114 (spirv-undefined-id) - undefined SPIR-V id
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29114
//META: catalog_name=spirv-undefined-id
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Referencing an undefined SPIR-V id triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    int x = spirv_asm
    {
        result: $$int = OpIMul $(15) %undefinedId
    };
}

//CHECK: E29114
