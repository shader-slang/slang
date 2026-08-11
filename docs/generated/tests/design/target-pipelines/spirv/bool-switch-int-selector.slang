//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=A switch on a bool selector is rewritten by legalizeBoolSwitchForTargetsRequiringIntSwitch into an integer OpSwitch whose selector is an OpSelect-produced int.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies Phase C's legalizeBoolSwitchForTargetsRequiringIntSwitch: SPIR-V
// requires an integer OpSwitch selector, so a `switch` written on a bool must
// be rewritten before emit. The selector is a runtime comparison so it cannot
// be constant-folded away; CHECK asserts an OpSelect that widens the bool to
// %int and an OpSwitch consuming that int with the literal 1 for `case true`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool sel = tid.x > 0;
    int r = 0;
    switch (sel)
    {
    case true:
        r = 10;
        break;
    default:
        r = 20;
        break;
    }
    outBuf[tid.x] = r;
}

// CHECK-DAG: OpSelect %int
// CHECK-DAG: OpSwitch %{{[a-zA-Z_0-9]+}} %{{[a-zA-Z_0-9]+}} 1
