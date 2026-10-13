//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=58d6fa34e989046b10c5c18ce14f1e0e4d1c1f389944592cfe513724b2c60cae
//META: purpose=An if/else statement in the AST becomes basic blocks joined by a conditional branch, and that structure reaches the emitted target text.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the statement-to-basic-block half of lowering. The condition is
// derived from the dispatch id so it cannot be folded, and each arm writes
// a different value to the output buffer so neither arm can be eliminated.
// The CHECKs assert a real two-way branch survives to emit: an `if`/`else`
// pair on the source back-ends, and on SPIR-V the structured-control-flow
// form -- `OpSelectionMerge` naming the join block plus the
// `OpBranchConditional` that selects between the two arms.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x > 0u)
    {
        gOut[0] = 11;
    }
    else
    {
        gOut[0] = -22;
    }
}

// HLSL-DAG: if(
// HLSL-DAG: else
// HLSL-DAG: int(11)

// GLSL-DAG: if(
// GLSL-DAG: else
// GLSL-DAG: 11

// SPIRV-DAG: OpSelectionMerge
// SPIRV-DAG: OpBranchConditional
// SPIRV-DAG: %int_11 = OpConstant %int 11

// METAL-DAG: if(
// METAL-DAG: else
// METAL-DAG: int(11)

// WGSL-DAG: if(
// WGSL-DAG: else
// WGSL-DAG: i32(11)

// CUDA-DAG: if(
// CUDA-DAG: else
// CUDA-DAG: int(11)

// CPP-DAG: if(
// CPP-DAG: else
// CPP-DAG: int(11)
