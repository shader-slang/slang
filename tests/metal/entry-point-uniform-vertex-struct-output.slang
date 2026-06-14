// Regression: an entry-point `uniform T*` parameter on a vertex shader that returns a struct
// must still be bound as a buffer argument on the Metal target. The vertex output-struct
// lowering (legalizeVertexShaderOutputParamsForMetal -> lowerOutParameters) replaces the entry
// point with a wrapper function; the moved-to-global uniform's IREntryPointParamDecoration used
// to keep pointing at the original function, so introduceExplicitGlobalContext failed to thread
// it into the wrapper and the [[buffer]] argument was silently dropped (it read uninitialized
// memory). It must lower the same way regardless of the entry's output shape, and on Vulkan the
// same parameter is a push constant.

//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vsMain
//TEST:SIMPLE(filecheck=METALCOMP): -target metal -stage compute -entry csMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage vertex -entry vsMain -fvk-use-entrypoint-name

struct GridCfg { uint dim; };
struct VOut { float4 pos : SV_Position; float4 color : COLOR; };

// The struct-returning vertex entry's uniform pointer must appear as a buffer argument at all
// (the bug dropped it entirely). The concrete index is Slang's own default layout (the
// entry-point params buffer takes the first buffer slot), so match buffer( without pinning 0.
//METAL: {{\[\[}}vertex{{\]\]}}
//METAL-SAME: buffer(
[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uint iid : SV_InstanceID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim) * 0.001 + float(iid) * 0.0 + float(vid) * 0.0, 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}

// The compute path (which never goes through the vertex output-struct wrapper) must keep binding
// the uniform too — guards against a regression in the shared lowering.
//METALCOMP: {{\[\[}}kernel{{\]\]}}
//METALCOMP-SAME: buffer(
[shader("compute")]
[numthreads(1, 1, 1)]
void csMain(uint3 tid : SV_DispatchThreadID, uniform GridCfg* cfg)
{
    // Touch the uniform so it is not dead-stripped.
    if (tid.x < cfg.dim)
        return;
}

// On Vulkan the entry-point uniform lowers to a push constant, not a descriptor in set 0.
//SPIRV: OpEntryPoint Vertex
//SPIRV: PushConstant
