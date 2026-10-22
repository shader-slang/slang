// Regression test for https://github.com/shader-slang/slang/issues/13841
// An entry point's `inout` parameter is a shader varying, not a pointer to the caller's storage,
// so its debug variable keeps a `_dbgvar_` copy on every target, as does a by-value parameter.
// Entry-point legalization moves parameter decorations onto the global inputs and outputs that
// replace the parameters, and Metal/WGSL must still compile.

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

// METAL: fsMain
// WGSL: fn fsMain
