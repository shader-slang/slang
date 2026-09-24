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

// Walking the dump in order: the original record (pre-lowering VOut return) and the wrapper
// record (lifted result struct) get distinct ids. Tie the original's id to a DebugFunction
// decoration, then the wrapper's id to the decoration on the [entryPoint] function and to its
// lifted signature -- a shared record, or a record on the wrong function, fails.
//VSMAIN: [[ORIG:%[0-9]+]]{{.*}}= DebugFunction("vsMain",{{.*}}Func(%VOut,
//VSMAIN: [DebugFunction([[ORIG]])]
//VSMAIN: [DebugFunction([[WRAP:%[0-9]+]])]
//VSMAIN: [entryPoint(
//VSMAIN: [[WRAP]]{{.*}}= DebugFunction("vsMain",{{.*}}Func(%{{.*}}Result{{.*}}BorrowInParam(UInt
[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim), 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}

// Same distinct-record binding for the `out`-parameter trigger: the original record is typed with
// an OutParam; the wrapper record (on the [entryPoint] function) with the lifted result struct.
//VSOUT: [[ORIG:%[0-9]+]]{{.*}}= DebugFunction("vsOut",{{.*}}OutParam
//VSOUT: [DebugFunction([[ORIG]])]
//VSOUT: [DebugFunction([[WRAP:%[0-9]+]])]
//VSOUT: [entryPoint(
//VSOUT: [[WRAP]]{{.*}}= DebugFunction("vsOut",{{.*}}Func(%{{.*}}Result{{.*}}BorrowInParam(UInt
[shader("vertex")]
void vsOut(uint vid : SV_VertexID, uniform GridCfg* cfg, out float4 pos : SV_Position)
{
    pos = float4(float(cfg.dim), 0, 0, 1);
}
