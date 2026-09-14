//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#branch--flatten--loopcontrol
//META: doc_section_digest=f2ba884d83c46721a4e1598e9622f13a2cab5e3cbdcc2f2993805ccbe9c8f247
//META: purpose=The [flatten] hint is consumed by the backend emit step: HLSL re-emits the [flatten] attribute on the conditional and SPIR-V encodes the Flatten selection control on the conditional's merge.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `flatten` is a pass-through hint the backend reads to pick
// the predicated control-flow strategy. The HLSL CHECK asserts the
// attribute reappears on the `if`; the SPIR-V CHECK asserts the same hint
// becomes the `Flatten` mask on `OpSelectionMerge`. The two targets encode
// the identical hint differently, which is why both are observed.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -O0 -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = int(tid.x);

    [flatten]
    if (tid.x > 2)
    {
        v = v * 3;
    }

    outputBuffer[tid.x] = v;
}

// HLSL: [flatten]
// SPIRV: OpSelectionMerge {{.*}} Flatten
