//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the Metal system-value mapping for the vertex and compute stages, including that SV_GroupIndex has no Metal attribute and is instead synthesized from the thread position within the threadgroup.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the vertex/compute rows of
// `LegalizeMetalEntryPointContext::getSystemValueInfo` plus its
// `handleSpecialSystemValue` escape hatch
// (slang-ir-legalize-varying-params.cpp:4110 and :4453). Most rows are a
// straight name mapping, but `SV_GroupIndex` is marked `isSpecial` because
// Metal has no flattened-thread-index attribute at all. The legalizer instead
// materializes it arithmetically from `[[thread_position_in_threadgroup]]`
// and the `[numthreads]` extents, so the value is computed in the body rather
// than declared in the signature. CHECK asserts both halves: the plain
// attribute mappings appear in the parameter lists, while the group index
// appears only as the `(z*Y + y)*X + x` expression built from the
// threadgroup-local id — with X and Y baked in from `[numthreads(4,2,1)]`.
//TEST:SIMPLE(filecheck=VERT):-target metal -entry vertMain -stage vertex
//TEST:SIMPLE(filecheck=COMP):-target metal -entry computeMain -stage compute

struct VOut
{
    float4 pos : SV_Position;
    float psize : SV_PointSize;
    uint layer : SV_RenderTargetArrayIndex;
}

[shader("vertex")]
VOut vertMain(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    VOut o;
    o.pos = float4(float(vid), float(iid), 0, 1);
    o.psize = 2.0;
    o.layer = 1;
    return o;
}

RWStructuredBuffer<uint> gOut;

[shader("compute")]
[numthreads(4, 2, 1)]
void computeMain(
    uint3 dtid : SV_DispatchThreadID,
    uint3 gid : SV_GroupID,
    uint3 gtid : SV_GroupThreadID,
    uint gi : SV_GroupIndex)
{
    gOut[dtid.x] = gid.x + gtid.y + gi;
}

// The vertex outputs are returned as an attributed result struct.
//VERT: struct vertMain_Result_0
//VERT-DAG: float4 pos_0 {{\[\[}}position]];
//VERT-DAG: float psize_0 {{\[\[}}point_size]];
//VERT-DAG: uint layer_0 {{\[\[}}render_target_array_index]];
//VERT: {{\[\[}}vertex]] vertMain_Result_0 vertMain(uint vid_0 {{\[\[}}vertex_id]], uint iid_0 {{\[\[}}instance_id]])

// The three id vectors map directly; SV_GroupIndex does not appear as a
// parameter at all.
//COMP: {{\[\[}}kernel]] void computeMain(uint3 dtid_0 {{\[\[}}thread_position_in_grid]], uint3 gid_0 {{\[\[}}threadgroup_position_in_grid]], uint3 gtid_0 {{\[\[}}thread_position_in_threadgroup]]
//COMP-NOT: local_invocation_index
// It is synthesized instead from the threadgroup-local id and [numthreads].
//COMP: (gtid_0[int(2)] * 2U + gtid_0[int(1)]) * 4U + gtid_0[int(0)]
