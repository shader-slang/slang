// Regression for shader-slang/slang#13251. When a Metal vertex entry point's outputs are lowered
// into a return struct, legalizeShaderOutputParamsForMetal -> lowerOutParameters replaces the entry
// point with a wrapper IRFunc. transferFunctionDecorations bare-clones the original's
// DebugFuncDecoration onto the wrapper with an empty clone environment, so the wrapper and the
// original ended up sharing a single IRDebugFunction whose debug type still described the
// pre-lowering signature -- the same producer-side shared-record defect as autodiff #13238, plus a
// stale debug type. An entry-point `uniform T*` keeps the original function alive past the swap: its
// hoisted global still names the original as its entry point until retargetEntryPointParamDecorations
// runs, so handleOriginalFunction sees useCount != 1 and leaves the original in place. That is what
// makes the sharing observable here.
//
// The defect is debug-info quality only: Metal MSL emit does not consume DebugFunction, and the
// SPIR-V/GLSL out-parameter path does not go through this Metal-only pass, so nothing observes it at
// runtime -- an IR-dump FileCheck guard is the only coverage. After the fix each function owns its
// own record, so two distinct DebugFunction records named after the entry point survive to this
// point; the unfixed compiler produces exactly one shared record. Both triggers of the pass are
// covered: a by-value struct return (vsMain) and an `out` parameter (vsOut).

//TEST:SIMPLE(filecheck=VSMAIN): -target metal -stage vertex -entry vsMain -g2 -dump-ir-after legalizeIRForMetal
//TEST:SIMPLE(filecheck=VSOUT): -target metal -stage vertex -entry vsOut -g2 -dump-ir-after legalizeIRForMetal

struct GridCfg { uint dim; };
struct VOut { float4 pos : SV_Position; float4 color : COLOR; };

// The surviving original keeps a record typed with the pre-lowering signature (returns VOut); the
// wrapper gets its own distinct record typed with the lifted result struct (name ends in "Result").
// Two order-independent checks, each matching a different record line -- against the unfixed compiler
// only the VOut-typed record exists, so the "Result" check fails.
//VSMAIN-DAG: DebugFunction("vsMain",{{.*}}Func(%VOut,
//VSMAIN-DAG: DebugFunction("vsMain",{{.*}}Result
[shader("vertex")]
VOut vsMain(uint vid : SV_VertexID, uniform GridCfg* cfg)
{
    VOut o;
    o.pos = float4(float(cfg.dim), 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}

// Same two-record assertion for the `out`-parameter trigger: the original record is typed with an
// OutParam; the wrapper's own record is typed with the lifted result struct.
//VSOUT-DAG: DebugFunction("vsOut",{{.*}}OutParam
//VSOUT-DAG: DebugFunction("vsOut",{{.*}}Result
[shader("vertex")]
void vsOut(uint vid : SV_VertexID, uniform GridCfg* cfg, out float4 pos : SV_Position)
{
    pos = float4(float(cfg.dim), 0, 0, 1);
}
