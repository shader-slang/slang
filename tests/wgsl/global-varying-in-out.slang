// Regression test for shader-slang/slang#13687 (module-scope `in out` varying).
//
// An `in out` global has one declared semantic, which applies to its input. The output field of
// the entry point's result takes the stage's default fragment output semantic. With two fragment
// entry points emitted into one module, each one gets its own input and its own result field.

//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsA -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsA -stage fragment
//TEST:SIMPLE(filecheck=MULTI_METAL): -target metal -whole-program -line-directive-mode none
//TEST:SIMPLE(filecheck=MULTI_WGSL): -target wgsl -whole-program

in out float4 gIO : COLOR0;

void touch()
{
    gIO = gIO * 2.0;
}

// METAL: {{\[\[}}color(0){{\]\]}}
// METAL: fsA(float4 gIO{{[A-Za-z0-9_]*}} {{\[\[}}user(COLOR0){{\]\]}})

// HLSL: : SV_TARGET0;
// HLSL: fsA(float4 gIO{{[A-Za-z0-9_]*}} : COLOR0)

// MULTI_METAL: struct [[A_RESULT:_S[0-9]+]]
// MULTI_METAL-NEXT: {
// MULTI_METAL-NEXT: {{\[\[}}color(0){{\]\]}}
// MULTI_METAL: {{\[\[}}fragment{{\]\]}} [[A_RESULT]] fsA(float4 gIO{{[A-Za-z0-9_]*}} {{\[\[}}user(COLOR0){{\]\]}})
// MULTI_METAL: struct [[B_RESULT:_S[0-9]+]]
// MULTI_METAL-NEXT: {
// MULTI_METAL-NEXT: {{\[\[}}color(0){{\]\]}}
// MULTI_METAL: {{\[\[}}fragment{{\]\]}} [[B_RESULT]] fsB(float4 gIO{{[A-Za-z0-9_]*}} {{\[\[}}user(COLOR0){{\]\]}})

// MULTI_WGSL: struct [[A_RESULT:[A-Za-z0-9_]+]]
// MULTI_WGSL-NEXT: {
// MULTI_WGSL-NEXT: @location(0)
// MULTI_WGSL: fn fsA({{.*}}) -> [[A_RESULT]]
// MULTI_WGSL: struct [[B_RESULT:[A-Za-z0-9_]+]]
// MULTI_WGSL-NEXT: {
// MULTI_WGSL-NEXT: @location(0)
// MULTI_WGSL: fn fsB({{.*}}) -> [[B_RESULT]]

[shader("fragment")]
void fsA()
{
    touch();
}

[shader("fragment")]
void fsB()
{
    touch();
}
