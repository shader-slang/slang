//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizeirforwgsl
//META: doc_section_digest=33ccb88a3da115be9cff2c913440e47d7a1de3a78d0cd2269b27a0b1b59788c0
//META: purpose=legalizeSwitch synthesizes the default case WGSL requires when the source switch has none
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `kIROp_Switch` arm of legalizeIRForWGSL's processInst walk.
// WGSL requires every switch to carry a default selector; in the IR a switch
// with no default is spelled by making the default label equal the break
// label, so the pass has to synthesize an empty default block. The source
// switch below deliberately omits `default:`, and CHECK asserts a `default`
// selector nevertheless appears in the emitted WGSL switch.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = 0;
    switch (int(tid.x))
    {
    case 0: v = 10; break;
    case 1: v = 20; break;
    }
    outBuf[tid.x] = v;
}

// CHECK: switch(
// CHECK: case i32(0)
// CHECK: case i32(1)
// CHECK: default
