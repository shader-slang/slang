//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes how the GLSL/SPIR-V entry-point legalizer splices a hull shader's patch-constant function into the entry point, guarded by SV_OutputControlPointID == 0 and preceded by a control barrier.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `invokePathConstantFuncInHullShader` (slang-ir-glsl-legalize.cpp:1229).
// Vulkan has no separate "patch constant" function: the tessellation-control
// shader must compute the tess factors itself, once per patch. The legalizer
// therefore rewrites the entry point's `return` into a branch to a new tail
// block that emits `OpControlBarrier`, compares the invocation id against 0,
// and calls the `[patchconstantfunc]` inside the `true` arm, passing the
// entry point's `InputPatch` parameter and a materialized `OutputPatch` built
// from the per-control-point output array. CHECK asserts exactly that shape:
// the barrier, the `OpIEqual ... %int_0` guard around an `OpSelectionMerge`,
// and the TessLevelOuter/TessLevelInner `Patch`-decorated builtins the
// SV_TessFactor / SV_InsideTessFactor fields are routed to.
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage hull

struct VOut { float3 pos : POSITION; }
struct HsOut { float3 pos : POSITION; }
struct HscOut
{
    float edges[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
}

// Reads both patch kinds, so the legalizer has to supply an `InputPatch`
// argument (the entry point's own parameter) and an `OutputPatch` argument
// (materialized from the control-point output array).
HscOut constants(InputPatch<VOut, 3> ip, OutputPatch<HsOut, 3> op)
{
    HscOut o;
    o.edges[0] = op[0].pos.x;
    o.edges[1] = 2;
    o.edges[2] = 3;
    o.inner = ip[0].pos.y;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
[shader("hull")]
HsOut main(InputPatch<VOut, 3> ip, uint cpid : SV_OutputControlPointID)
{
    HsOut o;
    o.pos = ip[cpid].pos;
    return o;
}

// The tess-factor outputs become `Patch`-decorated builtins on the entry point.
//SPIRV: OpEntryPoint TessellationControl %main "main" %gl_TessLevelOuter %gl_TessLevelInner
//SPIRV-DAG: OpExecutionMode %main OutputVertices 3
//SPIRV-DAG: OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
//SPIRV-DAG: OpDecorate %gl_TessLevelOuter Patch
//SPIRV-DAG: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
//SPIRV-DAG: OpDecorate %gl_TessLevelInner Patch
// SV_OutputControlPointID is the InvocationId builtin, and is reused as the
// "am I control point 0?" predicate for the spliced patch-constant call.
//SPIRV-DAG: OpDecorate %gl_InvocationID BuiltIn InvocationId
// The spliced tail: barrier, then a single-invocation guarded region.
//SPIRV: OpControlBarrier
//SPIRV: OpIEqual %bool {{%[0-9a-zA-Z_]+}} %int_0
//SPIRV: OpSelectionMerge
