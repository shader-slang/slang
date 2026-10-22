// Regression test for https://github.com/shader-slang/slang/issues/13841
// An entry point's `inout` parameter is a shader input and output, not a pointer to a caller's
// storage, so linking removes its `DebugVarStorage` decoration. On WGSL, entry-point
// legalization would otherwise move the decoration onto the struct key of the packed stage input,
// and that run's `-validate-ir` fails; it guards the removal. The SPIR-V run checks that the
// debug variable keeps a `_dbgvar_` copy, and the Metal run that the shader still compiles.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry fsMain -stage fragment -g2 -O0
//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsMain -stage fragment -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsMain -stage fragment -g2 -O0 -validate-ir

[shader("fragment")]
float4 fsMain(inout float4 c: COLOR, float2 uv: TEXCOORD) : SV_Target
{
    c.x += uv.x;
    return c;
}

// CHECK-DAG: [[C_STR:%[0-9]+]] = OpString "c"
// CHECK-DAG: OpName %_dbgvar_c "_dbgvar_c"
// CHECK-DAG: OpName %_dbgvar_uv "_dbgvar_uv"
// CHECK-DAG: [[C_DBG:%[A-Za-z_0-9]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[C_STR]]
// CHECK: DebugDeclare [[C_DBG]] %_dbgvar_c

// METAL: {{\[\[fragment\]\]}} {{.*}} fsMain(
// WGSL: fn fsMain
