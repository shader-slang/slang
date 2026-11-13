//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -profile cs_6_5

// HLSL declares `fma` only for `double` scalars, vectors and matrices, and `mad` for every
// floating-point type. Slang's `fma` therefore lowers to HLSL `fma` for `double` and to `mad`
// for `float` and `half`.

RWStructuredBuffer<float> bF;
RWStructuredBuffer<half> bH;
RWStructuredBuffer<double> bD;
RWStructuredBuffer<float3> bF3;
RWStructuredBuffer<half3> bH3;
RWStructuredBuffer<double3> bD3;
RWStructuredBuffer<float3x3> bF33;
RWStructuredBuffer<half3x3> bH33;
RWStructuredBuffer<double3x3> bD33;
RWStructuredBuffer<double> bDg;
RWStructuredBuffer<float3> bF3g;
RWStructuredBuffer<double3> bD3g;

// HLSL-DAG: bF_0{{.*}} = mad(
// HLSL-DAG: bH_0{{.*}} = mad(
// HLSL-DAG: bD_0{{.*}} = fma(
// HLSL-DAG: bF3_0{{.*}} = mad(
// HLSL-DAG: bH3_0{{.*}} = mad(
// HLSL-DAG: bD3_0{{.*}} = fma(
// HLSL-DAG: bF33_0{{.*}} = mad(
// HLSL-DAG: bH33_0{{.*}} = mad(
// HLSL-DAG: bD33_0{{.*}} = fma(
// HLSL-DAG: bDg_0{{.*}} = fma(
// HLSL-DAG: bF3g_0{{.*}} = mad(
// HLSL-DAG: bD3g_0{{.*}} = fma(

// DXIL op 47 is Fma (double only) and op 46 is FMad. No `double` call lowers to FMad.
// DXIL: define void @main()
// DXIL-NOT: @dx.op.tertiary.f64(i32 46
// DXIL: @dx.op.tertiary.f64(i32 47
// DXIL-NOT: @dx.op.tertiary.f64(i32 46

// The wrappers reach `fma` with an element type that is concrete only after the caller is
// specialized, so the double/other selection must still fold correctly there. They are inlined so
// that the selected intrinsic appears at the `bDg`, `bF3g` and `bD3g` stores checked above.
[ForceInline]
T callFma<T : __BuiltinFloatingPointType>(T a, T b, T c)
{
    return fma(a, b, c);
}

[ForceInline]
vector<T, N> callFmaVector<T : __BuiltinFloatingPointType, let N : int>(
    vector<T, N> a,
    vector<T, N> b,
    vector<T, N> c)
{
    return fma(a, b, c);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    bF[0] = fma(bF[1], bF[2], bF[3]);
    bH[0] = fma(bH[1], bH[2], bH[3]);
    bD[0] = fma(bD[1], bD[2], bD[3]);
    bF3[0] = fma(bF3[1], bF3[2], bF3[3]);
    bH3[0] = fma(bH3[1], bH3[2], bH3[3]);
    bD3[0] = fma(bD3[1], bD3[2], bD3[3]);
    bF33[0] = fma(bF33[1], bF33[2], bF33[3]);
    bH33[0] = fma(bH33[1], bH33[2], bH33[3]);
    bD33[0] = fma(bD33[1], bD33[2], bD33[3]);

    bDg[0] = callFma(bDg[1], bDg[2], bDg[3]);
    bF3g[0] = callFmaVector(bF3g[1], bF3g[2], bF3g[3]);
    bD3g[0] = callFmaVector(bD3g[1], bD3g[2], bD3g[3]);
}
