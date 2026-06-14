//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vsMain

// Regression test for #11606: a vertex entry point with a composite output (here a
// struct return) goes through `legalizeVertexShaderOutputParamsForMetal`, which wraps
// the entry point in a new function. The hoisted entry-point `uniform GridCfg* cfg`
// must still be bound as a kernel argument on the emitted vertex entry point. Before
// the fix the wrapper swap left the uniform's IREntryPointParamDecoration naming the
// old function, so `introduceExplicitGlobalContext` dropped it: the emitted vertex
// signature had no buffer argument and the body read an uninitialized KernelContext.

struct GridCfg
{
    uint dim;
};

struct VOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

// The hoisted uniform must be bound as a buffer argument on the emitted vertex entry point.
//METAL: vsMain({{.*}}buffer(0){{.*}})
// ...and the KernelContext must be initialized from that buffer argument, not left uninitialized.
//METAL: kernelContext{{.*}}entryPointParams{{.*}} = entryPointParams

[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uint iid : SV_InstanceID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim) * 0.001, 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}
