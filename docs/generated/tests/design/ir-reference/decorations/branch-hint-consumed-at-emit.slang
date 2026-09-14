//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#branch--flatten--loopcontrol
//META: doc_section_digest=f2ba884d83c46721a4e1598e9622f13a2cab5e3cbdcc2f2993805ccbe9c8f247
//META: purpose=The [branch] hint is consumed by the backend emit step: HLSL re-emits the [branch] attribute on the conditional and SPIR-V encodes the DontFlatten selection control on the conditional's merge.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `branch` carries no IR semantics of its own and is instead
// read by the backend to pick the divergent control-flow strategy. The HLSL
// CHECK asserts the attribute is printed back onto the `if`, and the SPIR-V
// CHECK asserts the same hint becomes the `DontFlatten` mask on
// `OpSelectionMerge`. Both observations are at final emit, which is where
// the doc says the hint is consumed.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -O0 -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = int(tid.x);

    [branch]
    if (tid.x > 1)
    {
        v = v + 7;
    }

    outputBuffer[tid.x] = v;
}

// HLSL: [branch]
// SPIRV: OpSelectionMerge {{.*}} DontFlatten
