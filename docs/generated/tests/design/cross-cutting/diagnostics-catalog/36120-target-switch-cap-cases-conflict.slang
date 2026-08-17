//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=43b7abf6c62c2959aa8ba7ad0f7de55f16c5e4997b50173693bd30fa6a9acd39
//META: purpose=Fires diagnostic E36120 (target-switch-cap-cases-conflict) - capability cases conflict in target_switch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36120
//META: catalog_name=target-switch-cap-cases-conflict
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two `__target_switch` cases that target the same target but
// neither capability set is a superset of the other trigger E36120.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage compute

void someFunc()
{
    __target_switch
    {
    case spvDeviceGroup:
        return;
    case spvBindlessTextureNV:
        return;
    }
}

[numthreads(1,1,1)]
void main()
{
    someFunc();
}

//CHECK: E36120
