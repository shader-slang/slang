//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E29115 (target-switch-case-cannot-be-a-stage) - cannot use stage name in __target_switch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=29115
//META: catalog_name=target-switch-case-cannot-be-a-stage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A stage name used as a target_switch case fires this diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int main()
{
    __target_switch
    {
    case vertex:
        return 1;
    default:
        return 0;
    }
}

// CHECK: E29115
