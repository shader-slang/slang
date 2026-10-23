// IR-level guard that Metal out-parameter lowering gives its replacement wrapper function its own
// DebugFunction record rather than sharing the original entry point's (shader-slang/slang#13251).
//
// legalizeShaderOutputParamsForMetal -> lowerOutParameters replaces a vertex entry point with a
// wrapper whose out/struct outputs are lifted into a return struct. transferFunctionDecorations
// bare-clones the original's DebugFuncDecoration onto the wrapper under an empty clone environment,
// so without the fix the wrapper and the original alias one IRDebugFunction still typed with the
// pre-lowering signature. fixUpDebugFuncType on the wrapper gives it its own record typed with the
// lifted result struct.
//
// Two survival modes are covered. With an entry-point `uniform GridCfg*` the original stays live
// alongside the wrapper (its hoisted global records the original as its entry point until
// retargetEntryPointParamDecorations runs), so both records are present -- vsMain (struct-return
// trigger) and vsOut (out-parameter trigger) each assert the original and wrapper reference
// distinct records. Without a uniform the original's only use is the wrapper's call, so
// handleOriginalFunction inlines and removes it, leaving only the wrapper -- vsNoUniform asserts
// the sole surviving record carries the lifted type. This pass is Metal-only and MSL emit ignores
// DebugFunction, so post-legalization IR is the only place it shows. Checks are order-insensitive
// (CHECK-DAG): records are told apart by their distinct type shapes, not by dump print order.

//TEST:SIMPLE(filecheck=VSMAIN): -target metal -stage vertex -entry vsMain -g2 -dump-ir-after legalizeIRForMetal
//TEST:SIMPLE(filecheck=VSOUT): -target metal -stage vertex -entry vsOut -g2 -dump-ir-after legalizeIRForMetal
//TEST:SIMPLE(filecheck=VSNOUNIFORM): -target metal -stage vertex -entry vsNoUniform -g2 -dump-ir-after legalizeIRForMetal

struct GridCfg { uint dim; };
struct VOut { float4 pos : SV_Position; float4 color : COLOR; };

// Bind the original record (pre-lowering VOut return) and the wrapper record (lifted result struct)
// to distinct ids by their type shapes, and require each id to be referenced by its own
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

// Same distinct-record binding for the out-parameter trigger: the original record is typed with an
// OutParam, the wrapper record with the lifted result struct.
//VSOUT-DAG: [[ORIG:%[0-9]+]]{{.*}}= DebugFunction("vsOut",{{.*}}OutParam
//VSOUT-DAG: [DebugFunction([[ORIG]])]
//VSOUT-DAG: [DebugFunction([[WRAP:%[0-9]+]])]
//VSOUT-DAG: [[WRAP]]{{.*}}= DebugFunction("vsOut",{{.*}}Result
[shader("vertex")]
void vsOut(uint vid : SV_VertexID, uniform GridCfg* cfg, out float4 pos : SV_Position)
{
    pos = float4(float(cfg.dim), 0, 0, 1);
}

// No uniform: the original is inlined and removed, so only the wrapper survives. Its sole record
// must carry the lifted result struct, not the pre-lowering signature a leaked shared record shows.
//VSNOUNIFORM-DAG: [DebugFunction([[W:%[0-9]+]])]
//VSNOUNIFORM-DAG: [[W]]{{.*}}= DebugFunction("vsNoUniform",{{.*}}Result
[shader("vertex")]
VOut vsNoUniform(uint vid : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.color = float4(1, 0, 0, 1);
    return o;
}
