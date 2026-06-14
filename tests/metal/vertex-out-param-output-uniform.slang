//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vsMain

// Companion to vertex-composite-output-uniform.slang covering the OTHER composite-output form that
// #11606 names: a vertex entry point with an `out` struct parameter (no struct return). Both forms
// funnel through the same `lowerOutParameters` wrapper swap and the same decoration re-point, so
// this pins that the `out`-parameter shape also binds the hoisted `uniform GridCfg* cfg` as a
// `[[buffer(0)]]` argument with the local `KernelContext` initialized from it.

struct GridCfg
{
    uint dim;
};

struct VOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

// The hoisted uniform is bound as a `[[buffer(0)]]` argument on the emitted vertex entry point.
//METAL: vsMain({{.*}}{{\[\[}}buffer(0){{\]\]}}{{.*}})
// ...and the local KernelContext field (LHS) is initialized from the bound kernel argument (RHS).
//METAL: kernelContext{{.*}}entryPointParams{{.*}} = entryPointParams

[shader("vertex")]
void vsMain(uint vid : SV_VertexID, out VOut o, uniform GridCfg* cfg)
{
    o.pos = float4(float(cfg.dim) * 0.001, 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
}
