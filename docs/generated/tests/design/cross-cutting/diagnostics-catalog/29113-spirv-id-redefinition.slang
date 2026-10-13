//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=68655502ae8505afd8b38dd85fed209f988a2a0dbda3fedf901fb910a080a979
//META: purpose=Fires diagnostic E29113 (spirv-id-redefinition) - SPIR-V id redefinition
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29113
//META: catalog_name=spirv-id-redefinition
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Defining the same %id twice within one spirv_asm block triggers redefinition.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    int x = spirv_asm
    {
        %dup : $$int = OpIAdd $(1) $(2);
        %dup : $$int = OpIAdd $(3) $(4);
        result: $$int = OpCopyObject %dup
    };
}

//CHECK: E29113
