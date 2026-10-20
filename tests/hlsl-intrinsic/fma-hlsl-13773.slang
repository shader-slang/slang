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

// HLSL-DAG: bF_0{{.*}} = mad(
// HLSL-DAG: bH_0{{.*}} = mad(
// HLSL-DAG: bD_0{{.*}} = fma(
// HLSL-DAG: bF3_0{{.*}} = mad(
// HLSL-DAG: bH3_0{{.*}} = mad(
// HLSL-DAG: bD3_0{{.*}} = fma(
// HLSL-DAG: bF33_0{{.*}} = mad(
// HLSL-DAG: bH33_0{{.*}} = mad(
// HLSL-DAG: bD33_0{{.*}} = fma(

// DXIL op 47 is Fma (double only) and op 46 is FMad. No `double` call lowers to FMad. DXC inlines
// every call into `main`, so this also covers the generic wrappers and the forward derivative.
// DXIL: define void @main()
// DXIL-NOT: @dx.op.tertiary.f64(i32 46
// DXIL: @dx.op.tertiary.f64(i32 47
// DXIL-NOT: @dx.op.tertiary.f64(i32 46

T callFma<T : __BuiltinFloatingPointType>(T a, T b, T c)
{
    return fma(a, b, c);
}

vector<T, N> callFmaVector<T : __BuiltinFloatingPointType, let N : int>(
    vector<T, N> a,
    vector<T, N> b,
    vector<T, N> c)
{
    return fma(a, b, c);
}

[Differentiable]
double diffFma(double a, double b, double c)
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

    bD[4] = callFma(bD[5], bD[6], bD[7]);
    bF3[4] = callFmaVector(bF3[5], bF3[6], bF3[7]);
    bD3[4] = callFmaVector(bD3[5], bD3[6], bD3[7]);
    bD[8] = fwd_diff(diffFma)(diffPair(bD[9], 1.0l), diffPair(bD[10], 0.0l), diffPair(bD[11], 0.0l)).p;
}
