//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -DTEST_DOUBLE

// Under `import glsl`, `*` on matrices and vectors is the algebraic product, which each
// target's native `*` also computes when it receives the operands in source order. The
// shared checks therefore look for products like `(((a_0) * (b_0)))`: the core module's
// component-wise `*`, or swapped operands, would not match. They use emitted names such as
// `a_0` so that they cannot match a source line quoted in a failed compile's diagnostics.
// The per-target checks pin the emitted signatures, so a wrong row/column mapping of the
// non-square shapes fails. Shapes follow GLSL conventions, so `float2x3` is GLSL `mat2x3`
// (2 columns, 3 rows).
//
// The `matrix<double, N, N>` `*=` overload is checked only on the GLSL target, because any
// `double` on WGSL or Metal currently ends in an internal emitter error (#10232).

import glsl;

RWStructuredBuffer<float4> buf;

float3x3 mulMatMat(float3x3 a, float3x3 b) { return a * b; }
// CHECK: mulMatMat_0(
// CHECK: return (((a_0) * (b_0)));

float3 mulMatVec(float2x3 a, float2 v) { return a * v; }
// CHECK: mulMatVec_0(
// CHECK: return (((a_1) * (v_0)));
// WGSL: fn mulMatVec_0( a_1 : mat2x3<f32>, v_0 : vec2<f32>) -> vec3<f32>
// METAL: float3 mulMatVec_0(matrix<float,int(2),int(3)> a_1, float2 v_0)
// GLSL: vec3 mulMatVec_0(mat2x3 a_1, vec2 v_0)

float2 mulVecMat(float3 v, float2x3 a) { return v * a; }
// CHECK: mulVecMat_0(
// CHECK: return (((v_1) * (a_2)));
// WGSL: fn mulVecMat_0( v_1 : vec3<f32>, a_2 : mat2x3<f32>) -> vec2<f32>
// METAL: float2 mulVecMat_0(float3 v_1, matrix<float,int(2),int(3)> a_2)
// GLSL: vec2 mulVecMat_0(vec3 v_1, mat2x3 a_2)

float2x3 mulMatMatNonSquare(float3x3 a, float2x3 c) { return a * c; }
// CHECK: mulMatMatNonSquare_0(
// CHECK: return (((a_3) * (c_0)));
// WGSL: fn mulMatMatNonSquare_0( a_3 : mat3x3<f32>, c_0 : mat2x3<f32>) -> mat2x3<f32>
// METAL: matrix<float,int(2),int(3)> mulMatMatNonSquare_0(matrix<float,int(3),int(3)> a_3, matrix<float,int(2),int(3)> c_0)
// GLSL: mat2x3 mulMatMatNonSquare_0(mat3x3 a_3, mat2x3 c_0)

float3x3 mulAssignFloat(float3x3 m, float3x3 n) { m *= n; return m; }
// CHECK: mulAssignFloat_0(
// CHECK: return (((m_0) * (n_0)));

half3x3 mulAssignHalf(half3x3 m, half3x3 n) { m *= n; return m; }
// CHECK: mulAssignHalf_0(
// CHECK: return (((m_1) * (n_1)));
// WGSL: fn mulAssignHalf_0( m_1 : mat3x3<f16>, n_1 : mat3x3<f16>) -> mat3x3<f16>
// METAL: matrix<half,int(3),int(3)> mulAssignHalf_0(matrix<half,int(3),int(3)> m_1, matrix<half,int(3),int(3)> n_1)
// GLSL: f16mat3x3 mulAssignHalf_0(f16mat3x3 m_1, f16mat3x3 n_1)

float3 mulAssignVec(float3 v, float3x3 m) { v *= m; return v; }
// CHECK: mulAssignVec_0(
// CHECK: return (((v_2) * (m_2)));

#ifdef TEST_DOUBLE
double3x3 mulAssignDouble(double3x3 m, double3x3 n) { m *= n; return m; }
#endif
// GLSL: dmat3x3 mulAssignDouble_0(dmat3x3 m_3, dmat3x3 n_2)
// GLSL: return (((m_3) * (n_2)));

// WGSL: @compute
// METAL: {{\[\[}}kernel{{\]\]}} void computeMain(
// GLSL: void main()
[numthreads(1, 1, 1)]
void computeMain()
{
    float3x3 a = float3x3(buf[0].xyz, buf[1].xyz, buf[2].xyz);
    float3x3 b = float3x3(buf[3].xyz, buf[4].xyz, buf[5].xyz);
    float2x3 c = float2x3(buf[6].xyz, buf[7].xyz);
    float3 v = buf[8].xyz;

    buf[9] = float4(mulMatMat(a, b)[0], 1);
    buf[10] = float4(mulMatVec(c, v.xy), 1);
    buf[11] = float4(mulVecMat(v, c), 1, 1);
    buf[12] = float4(mulMatMatNonSquare(a, c)[0], 1);
    buf[13] = float4(mulAssignFloat(a, b)[0], 1);
    buf[14] = float4(float3(mulAssignHalf(half3x3(a), half3x3(b))[0]), 1);
    buf[15] = float4(mulAssignVec(v, a), 1);
#ifdef TEST_DOUBLE
    buf[16] = float4(float3(mulAssignDouble(double3x3(a), double3x3(b))[0]), 1);
#endif
}
