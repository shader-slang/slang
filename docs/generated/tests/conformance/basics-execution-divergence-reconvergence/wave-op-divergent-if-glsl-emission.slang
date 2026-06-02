//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:03:42+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-execution-divergence-reconvergence.md#divergence
//META: doc_section_digest=7748f330eccf2fb718b5ba2c0f9aff0035f24f4197647463775fd43f1b5ebbb9
//META: purpose=Wave-tangled ops (WaveActiveMin/WaveActiveMax) inside a divergent `if` emit as GLSL subgroup ops (subgroupMin/subgroupMax) inside their respective if-arms, requiring the GL_KHR_shader_subgroup_arithmetic extension.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: when wave-tangled functions are supported on divergent paths,
// they apply only between the mutually convergent thread set. In GLSL emission,
// WaveActiveMin -> subgroupMin and WaveActiveMax -> subgroupMax; they appear
// inside the respective if arms and require the subgroup_arithmetic extension.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint result = 0;
    if ((tid.x & 1) == 0)
    {
        result = WaveActiveMin(tid.x);
    }
    else
    {
        result = WaveActiveMax(tid.x);
    }
    output[tid.x] = result;
}

// GLSL requires the subgroup_arithmetic extension for wave ops.
// CHECK: GL_KHR_shader_subgroup_arithmetic
// subgroupMin is the GLSL form of WaveActiveMin.
// CHECK: subgroupMin
// subgroupMax is the GLSL form of WaveActiveMax.
// CHECK: subgroupMax
