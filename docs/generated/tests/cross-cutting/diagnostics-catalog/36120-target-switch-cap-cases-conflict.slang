//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E36120
