//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=ee40ae671436ac98c9b021b1bf97506d1463f211581d7a1d67255a10c9b0ac0a
//META: purpose=Fires diagnostic E36101 (conflicting-capability-due-to-statement) - conflicting capability requirement
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36101
//META: catalog_name=conflicting-capability-due-to-statement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `__target_switch` case that conflicts with the enclosing
// function's [require(...)] declaration triggers E36101.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage compute -skip-spirv-validation

[require(glsl, _sm_4_0)]
public void use1()
{
    __target_switch
    {
    case glsl:
        return;
    }
}

void main()
{
    use1();
}

//CHECK: E36101
