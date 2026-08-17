//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes how a domain (tessellation-evaluation) entry point's OutputPatch parameter, SV_DomainLocation and patch-constant struct are legalized into per-vertex Location inputs and the TessCoord / TessLevelInner builtins.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the domain-stage arm of `getGLSLSystemValueInfo` /
// `legalizePatchParam` in slang-ir-glsl-legalize.cpp. A domain shader takes
// three conceptually different inputs, and the legalizer routes each to a
// different place: the `OutputPatch<HsOut,3>` becomes a per-vertex `Location`
// input array, `SV_DomainLocation` becomes the `TessCoord` builtin, and the
// patch-constant struct's SV_InsideTessFactor field is read back from the
// `Patch`-decorated `TessLevelInner` builtin rather than from a Location slot.
// CHECK asserts each of those three destinations on the entry-point
// interface, which is what distinguishes the domain arm from the vertex arm
// (where all three would be plain Location inputs).
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage domain

struct HsOut { float3 pos : POSITION; }
struct HscOut
{
    float edges[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
}
struct DsOut { float4 pos : SV_Position; }

[domain("tri")]
[shader("domain")]
DsOut main(OutputPatch<HsOut, 3> patch, float3 uvw : SV_DomainLocation, HscOut pc)
{
    DsOut o;
    o.pos = float4(
        patch[0].pos * uvw.x + patch[1].pos * uvw.y + patch[2].pos * uvw.z,
        pc.inner);
    return o;
}

//SPIRV: OpEntryPoint TessellationEvaluation %main "main"
//SPIRV-DAG: OpExecutionMode %main Triangles
// The control-point patch is an ordinary per-vertex Location input array.
//SPIRV-DAG: OpDecorate %patch_pos Location 0
// SV_DomainLocation is the TessCoord builtin (not Patch-decorated).
//SPIRV-DAG: OpDecorate %gl_TessCoord BuiltIn TessCoord
// The patch-constant struct's SV_InsideTessFactor is read from the
// Patch-decorated TessLevelInner builtin.
//SPIRV-DAG: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
//SPIRV-DAG: OpDecorate %gl_TessLevelInner Patch
//SPIRV-DAG: OpDecorate %gl_Position BuiltIn Position
