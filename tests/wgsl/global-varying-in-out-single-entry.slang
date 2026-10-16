// Guard for shader-slang/slang#13687: the declared semantic of a module-scope `in out` varying
// describes its input. The output field of the entry point's result takes the stage's default
// fragment output semantic, as it did before global varyings were translated per entry point.

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsA -stage fragment
//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsA -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsA -stage fragment

in out float4 gIO : COLOR0;

void touch()
{
    gIO = gIO * 2.0;
}

// WGSL: @location(0) {{[A-Za-z0-9_]+}} : vec4<f32>
// WGSL: fn fsA( gIO{{[A-Za-z0-9_]*}} : vec4<f32>)

// METAL: {{\[\[}}color(0){{\]\]}}
// METAL: fsA(float4 gIO{{[A-Za-z0-9_]*}} {{\[\[}}user(COLOR0){{\]\]}})

// HLSL: : SV_TARGET0;
// HLSL: fsA(float4 gIO{{[A-Za-z0-9_]*}} : COLOR0)

[shader("fragment")]
void fsA()
{
    touch();
}
