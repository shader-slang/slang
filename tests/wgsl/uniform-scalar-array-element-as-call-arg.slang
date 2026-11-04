// An element of a scalar uniform array (`float[N]` / `int[N]`) passed as a function argument
// must compile for WGSL.
//
// WGSL requires a 16-byte array stride in the uniform address space, so a scalar uniform array
// is lowered to `_Array_std140_<T>4 { vector<T,4> data[N]; }` and each element read becomes
// `vectorReshape(vector<T,4> -> T)`. The peephole rewrite of that scalar-result reshape into a
// swizzle used to give the swizzle a null type, which crashed dead-code elimination when the
// value fed a call (a plain arithmetic use of the element did not crash).

//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage vertex -entry vertMain
//TEST:SIMPLE(filecheck=WGSL_FRAG): -target wgsl -stage fragment -entry fragMain

uniform float uFloats[4];
uniform int uInts[4];

float scaleBy(float s) { return s * 2.0; }
float fromInt(int i) { return float(i); }

// WGSL: struct _Array_std140_float4
// WGSL: array<vec4<f32>, i32(4)>
// WGSL: fn vertMain
// WGSL: scaleBy{{.*}}(globalParams{{.*}}.uFloats{{.*}}.data{{.*}}[i32(1)].x)
// WGSL: fromInt{{.*}}(globalParams{{.*}}.uInts{{.*}}.data{{.*}}[i32(2)].x)
[shader("vertex")]
float4 vertMain(float3 pos : POSITION) : SV_Position
{
    return float4(pos.xy * scaleBy(uFloats[1]) + fromInt(uInts[2]), 0.0, 1.0);
}

// WGSL_FRAG: fn fragMain
// WGSL_FRAG: scaleBy{{.*}}(globalParams{{.*}}.uFloats{{.*}}.data{{.*}}[i32(3)].x)
[shader("fragment")]
float4 fragMain() : SV_Target
{
    return float4(scaleBy(uFloats[3]));
}
