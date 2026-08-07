//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=d2bb343d99876dfd1d05e29a3d730266a1a5c82ccace506601773d10ac4885df
//META: purpose=A for-loop statement in the AST becomes a back-edged block structure whose loop header, exit test, and body all reach the emitted target text.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the loop case of statement lowering, which needs a back edge
// rather than the straight-line join an if/else produces. The trip count
// comes from the dispatch id so the loop cannot be unrolled or folded away.
// On the source back-ends the CHECKs pin the canonical lowered shape -- an
// unconditional `for(;;)` header with the exit test re-expressed as an
// early `break` -- and on SPIR-V they pin `OpLoopMerge`, the structured
// marker that names the loop's merge and continue targets.

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
    int sum = 0;
    for (uint i = 0u; i < tid.x; ++i)
        sum = sum + int(i);
    gOut[0] = sum;
}

// HLSL-DAG: for(;;)
// HLSL-DAG: break;

// GLSL-DAG: for(;;)
// GLSL-DAG: break;

// SPIRV-DAG: OpLoopMerge
// SPIRV-DAG: OpBranchConditional

// METAL-DAG: for(;;)
// METAL-DAG: break;

// WGSL-DAG: for(;;)
// WGSL-DAG: break;

// CUDA-DAG: for(;;)
// CUDA-DAG: break;

// CPP-DAG: for(;;)
// CPP-DAG: break;
