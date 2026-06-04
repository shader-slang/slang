//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E29113
