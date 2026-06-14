//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vsMain

// On `-target metal`, a vertex entry point with composite output (a struct return here) is
// wrapped by a new function in `legalizeVertexShaderOutputParamsForMetal`. This test pins the
// contract that the hoisted entry-point `uniform GridCfg* cfg` is still bound as a `[[buffer(0)]]`
// argument on the emitted vertex entry point and that the local `KernelContext` is initialized
// from it.
//
// Regression for #11606: before the fix the wrapper swap left the uniform's
// `IREntryPointParamDecoration` naming the old function, so `introduceExplicitGlobalContext`
// dropped it — the emitted vertex signature had no buffer argument and the body read an
// uninitialized `KernelContext`.

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
// ...and the local KernelContext field (LHS) is initialized from the bound kernel argument (RHS);
// both are spelled `entryPointParams` and are distinguished only by the assignment (`= `). The
// `{{.*}}` wildcards absorb the per-inst name suffixes and the `(&...)->` access.
//METAL: kernelContext{{.*}}entryPointParams{{.*}} = entryPointParams

[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uint iid : SV_InstanceID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim) * 0.001, 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}
