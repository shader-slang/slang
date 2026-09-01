// Regression: an entry-point `uniform T*` parameter on a vertex shader must still be bound as a
// buffer argument on the Metal target when the vertex entry's output is lowered into a return
// struct by legalizeShaderOutputParamsForMetal -> lowerOutParameters. That pass replaces
// the entry point with a wrapper function; the moved-to-global uniform's
// IREntryPointParamDecoration used to keep pointing at the original function, so
// introduceExplicitGlobalContext failed to thread it into the wrapper and the [[buffer]] argument
// was silently dropped (it read uninitialized memory). The wrapper is produced for BOTH triggers
// of that pass -- a struct return AND `out` parameters -- so both are covered here.
//
// Each check pins not just that a buffer argument exists, but that the wrapper initializes its
// KernelContext from that argument (`kernelContext...= entryPointParams`). That assignment is
// exactly what the buggy compiler omitted, so the check fails against the unfixed compiler rather
// than merely matching an incidental `buffer(` elsewhere. The concrete buffer index is Slang's own
// default layout (entry-point params take the first buffer slot), so the signature check matches
// `buffer(` without pinning a slot number.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vsMain
//TEST:SIMPLE(filecheck=METALOUT): -target metal -stage vertex -entry vsOut
//TEST:SIMPLE(filecheck=METALCOMP): -target metal -stage compute -entry csMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage vertex -entry vsMain -fvk-use-entrypoint-name

struct GridCfg { uint dim; };
struct VOut { float4 pos : SV_Position; float4 color : COLOR; };

// Struct-return vertex: uniform must be a buffer argument and be threaded into the context.
//METAL: {{\[\[}}vertex{{\]\]}}
//METAL-SAME: buffer(
//METAL: kernelContext{{.*}}= entryPointParams
[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uint iid : SV_InstanceID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim) * 0.001 + float(iid) * 0.0 + float(vid) * 0.0, 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}

// `out`-parameter vertex: same lowerOutParameters wrapper swap, so the same regression applies.
//METALOUT: {{\[\[}}vertex{{\]\]}}
//METALOUT-SAME: buffer(
//METALOUT: kernelContext{{.*}}= entryPointParams
[shader("vertex")]
void vsOut(uint vid : SV_VertexID, uniform GridCfg* cfg, out float4 pos : SV_Position,
           out float4 col : COLOR)
{
    pos = float4(float(cfg.dim) * 0.001 + float(vid) * 0.0, 0, 0, 1);
    col = float4(1, 0, 0, 1);
}

// Compute never goes through the vertex output-struct wrapper; guards the shared lowering.
//METALCOMP: {{\[\[}}kernel{{\]\]}}
//METALCOMP-SAME: buffer(
[shader("compute")]
[numthreads(1, 1, 1)]
void csMain(uint3 tid : SV_DispatchThreadID, uniform GridCfg* cfg)
{
    if (tid.x < cfg.dim)
        return;
}

// On Vulkan the entry-point uniform lowers to a push constant, not a descriptor in set 0.
//SPIRV: OpEntryPoint Vertex
//SPIRV: PushConstant
