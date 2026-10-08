// Non-scalar `fwidth` must emit for a Metal fragment shader. See issue #12165.
// METAL checks the emitted MSL text; METALLIB confirms the Metal compiler accepts it.
// The vector and matrix overloads have separate entry points, and each emit-check pins
// a token unique to one overload's lowering, so a regression in either path fails on its
// own rather than being masked by the other path's `fwidth` emission.
//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib

struct FSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// The vector overload emits a direct `fwidth(<vector>)` call at the use site, here wrapped
// in the return's `float4(...)` — a shape the matrix path never produces.
// METALLIB-DAG: define {{.*}} @fragVector
[shader("fragment")]
float4 fragVector(FSInput input) : SV_Target
{
    // METAL-DAG: float4((fwidth(
    return float4(fwidth(input.uv), 0.0, 1.0);
}

// The matrix overload lowers via MATRIX_MAP_UNARY into a generated per-row helper named
// `fwidth_<n>` (which itself calls the vector overload); the call to that helper is the
// matrix path's use site, and its name is unique to this path.
// METALLIB-DAG: define {{.*}} @fragMatrix
[shader("fragment")]
float4 fragMatrix(FSInput input) : SV_Target
{
    float2x2 m = float2x2(input.uv.x, input.uv.y, input.uv.y, input.uv.x);
    // METAL-DAG: = fwidth_{{[0-9]+}}(
    float2x2 mw = fwidth(m);
    return float4(mw[0][0], mw[1][1], 0.0, 1.0);
}
