// Non-scalar `fwidth` must emit for a WGSL fragment shader. See issue #12165.
// The vector and matrix `fwidth` overloads share the same capability set as scalar
// `fwidth` (glsl_hlsl_metal_spirv_wgsl); this pins the WGSL emission so a regression in
// either the vector case or the matrix row-decomposition path fails on its own.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl

struct FSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// The vector overload emits a direct `fwidth(<vector>)` call at the use site, here wrapped
// in the return's `vec4<f32>(...)` — a shape the matrix path never produces.
[shader("fragment")]
float4 fragVector(FSInput input) : SV_Target
{
    // WGSL-DAG: vec4<f32>((fwidth(
    return float4(fwidth(input.uv), 0.0, 1.0);
}

// The matrix overload lowers via MATRIX_MAP_UNARY into a generated per-row helper named
// `fwidth_<n>` (which itself calls the vector overload); the call to that helper is the
// matrix path's use site, and its name is unique to this path.
[shader("fragment")]
float4 fragMatrix(FSInput input) : SV_Target
{
    float2x2 m = float2x2(input.uv.x, input.uv.y, input.uv.y, input.uv.x);
    // WGSL-DAG: = fwidth_{{[0-9]+}}(
    float2x2 mw = fwidth(m);
    return float4(mw[0][0], mw[1][1], 0.0, 1.0);
}
