//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:10:36Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#branch-flatten-loopcontrol
//META: doc_section_digest=262cdb3047c0e840655e74b96e0546c7c47439596c001991c8eb9e996922be06
//META: purpose=The [branch] hint is consumed by the backend emit step: HLSL re-emits the [branch] attribute on the conditional and SPIR-V encodes the DontFlatten selection control on the conditional's merge.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the branch hint carries no IR semantics of its own and is
// consumed at emit to pick the divergent control-flow strategy on the
// target: the HLSL backend prints the [branch] attribute back onto the
// if statement, and the SPIR-V backend encodes the same hint as the
// DontFlatten selection-control mask on OpSelectionMerge. The condition
// depends on the dispatch thread id and the result escapes through a
// buffer write, so the conditional survives to the emit step.

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
