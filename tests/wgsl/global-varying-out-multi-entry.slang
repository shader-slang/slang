// Regression test for shader-slang/slang#13687 (module-scope `out` varying).
//
// A module-scope `out` varying written by a callee shared by two fragment entry points must
// become part of the result of *every* entry point that reaches it when both entry points are
// emitted into one module, not only the first one in module order.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -whole-program
//TEST:SIMPLE(filecheck=METAL): -target metal -line-directive-mode none
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -whole-program

out float4 gColor : SV_Target;

void writeColor(float v)
{
    gColor = float4(v, 0, 0, 1);
}

// WGSL: @location(0) gColor{{[A-Za-z0-9_]*}} : vec4<f32>
// WGSL: fn fsA() -> [[RESULT_A:[A-Za-z0-9_]+]]
// WGSL: return
// WGSL: @location(0) gColor{{[A-Za-z0-9_]*}} : vec4<f32>
// WGSL: fn fsB() -> [[RESULT_B:[A-Za-z0-9_]+]]
// WGSL: return

// METAL: {{\[\[}}color(0){{\]\]}}
// METAL: {{\[\[}}fragment{{\]\]}} {{[A-Za-z0-9_]+}} fsA()
// METAL: {{\[\[}}color(0){{\]\]}}
// METAL: {{\[\[}}fragment{{\]\]}} {{[A-Za-z0-9_]+}} fsB()

// SPIRV-DAG: OpEntryPoint Fragment %fsA "fsA" {{.*}}%entryPointParam_fsA_gColor
// SPIRV-DAG: OpEntryPoint Fragment %fsB "fsB" {{.*}}%entryPointParam_fsB_gColor

[shader("fragment")]
void fsA()
{
    writeColor(1.0);
}

[shader("fragment")]
void fsB()
{
    writeColor(2.0);
}
