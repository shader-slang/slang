//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#capability--target-attributes
//META: doc_section_digest=ce8106cbb762f8a84c2c1448718332b6001d57168b92318fbbf9d952624061de
//META: purpose=`[require(spirv)]` lets a function be called from an entry point compiled for a target that supplies the capability.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive half of the capability tie: when the compilation
// target does provide the required atom, the call checks and the function is
// emitted normally. The shader is compiled for SPIR-V and CHECK asserts the
// entry point reaches the emitted module.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[require(spirv)]
int spvOnly(int x)
{
    return x + 1;
}

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = spvOnly(int(tid.x));
}

//CHECK: OpEntryPoint GLCompute
