//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry mainHalf -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry main -profile cs_6_5 -fp-mode precise
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry mainHalf -profile cs_6_5 -fp-mode precise
//TEST:SIMPLE(filecheck=DOUBLE): -target dxil-assembly -entry mainDouble -profile cs_6_5

// `rint` rounds halfway cases to the nearest even integer, as HLSL `round` does (DXIL `Round_ne`),
// so on HLSL it lowers to `round` instead of the halfway correction of the generic fallback.
// The DXIL test uses the precise floating-point mode, in which DXC keeps the correction of the
// fallback (its absolute value): in the default mode, its fast-math folding removes it.

RWStructuredBuffer<float> bF;
RWStructuredBuffer<float3> bF3;

[numthreads(1, 1, 1)]
void main(uint i : SV_DispatchThreadID)
{
    bF[i] = rint(bF[i]);
    bF3[i] = rint(bF3[i]);
}

RWStructuredBuffer<half> bH;
RWStructuredBuffer<vector<half, 3>> bH3;

[numthreads(1, 1, 1)]
void mainHalf(uint i : SV_DispatchThreadID)
{
    bH[i] = rint(bH[i]);
    bH3[i] = rint(bH3[i]);
}

// Neither the scalar nor the vector overload emits the correction, which would call `abs`
// before, between or after their `round` calls.
// HLSL-NOT: abs(
// HLSL: round(
// HLSL-NOT: abs(
// HLSL: round(
// HLSL-NOT: abs(

// DXIL-NOT: @dx.op.unary.{{f32|f16}}(i32 6,
// DXIL: call {{float|half}} @dx.op.unary.{{f32|f16}}(i32 26
// DXIL-NOT: @dx.op.unary.{{f32|f16}}(i32 6,

// HLSL has no `double` `round`: DXC rounds a `float` conversion of the argument and returns a
// `float`, so `double` keeps the fallback, whose result is a `double`. A byte-address store
// deduces its type from the value, so it stores 64 bits.

RWByteAddressBuffer bD;

[numthreads(1, 1, 1)]
void mainDouble(uint i : SV_DispatchThreadID)
{
    bD.Store(i * 32, rint(bD.Load<double>(i * 32)));
    bD.Store(i * 32 + 16, rint(bD.Load<double2>(i * 32 + 16)));
}

// DOUBLE-NOT: rawBufferStore.f32
// DOUBLE: rawBufferStore.f64
// DOUBLE-NOT: rawBufferStore.f32
