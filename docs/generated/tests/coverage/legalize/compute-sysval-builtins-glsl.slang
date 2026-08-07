//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=b4c8d3637c56a76592380b5850772aac25face32ea591824cb9bbf293ce90b35
//META: purpose=Verifies the four compute-stage thread-id system values map to their GLSL gl_* builtins (GlobalInvocationID/LocalInvocationID/WorkGroupID/LocalInvocationIndex).
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the compute-stage system-value mapping table in the GLSL
// legalizer: SV_DispatchThreadID->gl_GlobalInvocationID,
// SV_GroupThreadID->gl_LocalInvocationID, SV_GroupID->gl_WorkGroupID,
// SV_GroupIndex->gl_LocalInvocationIndex. Each value is read in the body
// and summed into a buffer so none is DCE'd; CHECK asserts each builtin
// name appears in the emitted GLSL.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(
    uint3 dt : SV_DispatchThreadID,
    uint3 gt : SV_GroupThreadID,
    uint3 g : SV_GroupID,
    uint gi : SV_GroupIndex)
{
    outBuf[dt.x] = dt.x + gt.y + g.z + gi;
}

//GLSL-DAG: gl_GlobalInvocationID
//GLSL-DAG: gl_LocalInvocationID
//GLSL-DAG: gl_WorkGroupID
//GLSL-DAG: gl_LocalInvocationIndex
