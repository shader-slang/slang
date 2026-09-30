//TEST:SIMPLE(filecheck=CHECK,WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK,METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK,GLSL): -target glsl -stage compute -entry computeMain -DTEST_DOUBLE

// The double overload is exercised only on GLSL: WGSL and Metal have no 64-bit float,
// and emitting `double` for them is tracked by #10232.

import glsl;

RWStructuredBuffer<float4> buf;

float3x3 mulMatMat(float3x3 a, float3x3 b) { return a * b; }
// CHECK: mulMatMat_0(
// CHECK: return (((a_0) * (b_0)));

float3 mulMatVec(float2x3 a, float2 v) { return a * v; }
// CHECK: mulMatVec_0(
// CHECK: return (((a_1) * (v_0)));

float2 mulVecMat(float3 v, float2x3 a) { return v * a; }
// CHECK: mulVecMat_0(
// CHECK: return (((v_1) * (a_2)));

float2x3 mulNonSquare(float3x3 a, float2x3 c) { return a * c; }
// CHECK: mulNonSquare_0(
// CHECK: return (((a_3) * (c_0)));

float3x3 mulAssignFloat(float3x3 m, float3x3 n) { m *= n; return m; }
// CHECK: mulAssignFloat_0(
// CHECK: return (((m_0) * (n_0)));

half3x3 mulAssignHalf(half3x3 m, half3x3 n) { m *= n; return m; }
// WGSL: mulAssignHalf_0( m_1 : mat3x3<f16>,  n_1 : mat3x3<f16>)
// METAL: mulAssignHalf_0(matrix<half,int(3),int(3)>  m_1, matrix<half,int(3),int(3)>  n_1)
// GLSL: mulAssignHalf_0(f16mat3x3 m_1, f16mat3x3 n_1)
// CHECK: return (((m_1) * (n_1)));

float3 mulAssignVec(float3 v, float3x3 m) { v *= m; return v; }
// CHECK: mulAssignVec_0(
// CHECK: return (((v_2) * (m_2)));

#ifdef TEST_DOUBLE
double3x3 mulAssignDouble(double3x3 m, double3x3 n) { m *= n; return m; }
#endif
// GLSL: mulAssignDouble_0(dmat3x3 m_3, dmat3x3 n_2)
// GLSL: return (((m_3) * (n_2)));

// WGSL: @compute
// METAL: [[kernel]] void computeMain(
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
    buf[12] = float4(mulNonSquare(a, c)[0], 1);
    buf[13] = float4(mulAssignFloat(a, b)[0], 1);
    buf[14] = float4(float3(mulAssignHalf(half3x3(a), half3x3(b))[0]), 1);
    buf[15] = float4(mulAssignVec(v, a), 1);
#ifdef TEST_DOUBLE
    buf[16] = float4(float3(mulAssignDouble(double3x3(a), double3x3(b))[0]), 1);
#endif
}
