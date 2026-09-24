// IR-level guard that Metal out-parameter lowering gives its replacement wrapper function its own
// DebugFunction record rather than sharing the original entry point's (shader-slang/slang#13251).
//
// legalizeShaderOutputParamsForMetal -> lowerOutParameters replaces a vertex entry point with a
// wrapper whose outputs are lifted into a return struct. The entry-point `uniform GridCfg*` is what
// keeps the original function alive alongside the wrapper: its hoisted global still records the
// original as its entry point until retargetEntryPointParamDecorations runs, so both
// function-to-DebugFunction associations exist and can be checked here. Each entry point then has
// two distinct records -- the original typed with its pre-lowering signature, the wrapper typed
// with the lifted result struct -- and each function references its own. Both triggers of the pass
// are exercised: a by-value struct return (vsMain) and an `out` parameter (vsOut). This pass is
// Metal-only and MSL emit ignores DebugFunction, so the post-legalization IR is the only place the
// property is observable.

//TEST:SIMPLE(filecheck=VSMAIN): -target metal -stage vertex -entry vsMain -g2 -dump-ir-after legalizeIRForMetal
//TEST:SIMPLE(filecheck=VSOUT): -target metal -stage vertex -entry vsOut -g2 -dump-ir-after legalizeIRForMetal

struct GridCfg { uint dim; };
struct VOut { float4 pos : SV_Position; float4 color : COLOR; };

// Bind the original record (typed with the pre-lowering VOut return) and the wrapper record (typed
// with the lifted result struct) to distinct ids, and require each id to be referenced by its own
// DebugFunction decoration -- so the two functions cannot be sharing one record.
//VSMAIN-DAG: [[ORIG:%[0-9]+]]{{.*}}= DebugFunction("vsMain",{{.*}}Func(%VOut,
//VSMAIN-DAG: [DebugFunction([[ORIG]])]
//VSMAIN-DAG: [DebugFunction([[WRAP:%[0-9]+]])]
//VSMAIN-DAG: [[WRAP]]{{.*}}= DebugFunction("vsMain",{{.*}}Result
[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim), 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}

// Same distinct-record binding for the `out`-parameter trigger: the original record is typed with
// an OutParam, the wrapper record with the lifted result struct.
//VSOUT-DAG: [[ORIG:%[0-9]+]]{{.*}}= DebugFunction("vsOut",{{.*}}OutParam
//VSOUT-DAG: [DebugFunction([[ORIG]])]
//VSOUT-DAG: [DebugFunction([[WRAP:%[0-9]+]])]
//VSOUT-DAG: [[WRAP]]{{.*}}= DebugFunction("vsOut",{{.*}}Result
[shader("vertex")]
void vsOut(uint vid : SV_VertexID, uniform GridCfg* cfg, out float4 pos : SV_Position)
{
    pos = float4(float(cfg.dim), 0, 0, 1);
}
