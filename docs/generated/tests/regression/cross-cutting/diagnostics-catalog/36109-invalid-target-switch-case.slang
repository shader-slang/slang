//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E36109 (invalid-target-switch-case) - invalid target_switch case
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36109
//META: catalog_name=invalid-target-switch-case
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `__target_switch` case using a non-target capability atom
// (e.g. sm_6_0 is a profile/feature, not a target) triggers E36109.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target glsl -entry main -stage compute -profile sm_5_0

void worker()
{
    __target_switch
    {
    case sm_6_0:
        return;
    }
}

[numthreads(1,1,1)]
void main()
{
    worker();
}

// CHECK: E36109
