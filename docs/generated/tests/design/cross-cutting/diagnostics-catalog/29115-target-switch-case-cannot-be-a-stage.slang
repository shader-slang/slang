//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e03fdf05a12e4d6c633a2eb4e6b1cc7d608761ff628bfd68595c1eb4146c2c10
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

//CHECK: E29115
