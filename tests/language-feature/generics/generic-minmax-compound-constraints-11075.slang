//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Regression test for shader-slang/slang#11075: generic code that names `IComparable` or
// `IArithmetic` next to `IFloat` or `IInteger` also reaches the concrete vector overloads. Overload
// resolution compares conversion cost before rank. A directly named `IComparable` costs nothing,
// and `IArithmetic` reaches the `IComparable` overload through one refinement, as `IFloat` reaches
// `__IMinMax`, so these spellings rely on `min`/`max` overloads constrained by `IFloat` and
// `IInteger` themselves, ranked above the `IComparable` ones.

T minFloatComparable<T : IFloat & IComparable>(T a, T b)
{
    return min(a, b);
}
T maxFloatComparableWhere<T>(T a, T b)
    where T : IFloat
    where T : IComparable
{
    return max(a, b);
}
T minFloatArithmetic<T : IFloat & IArithmetic>(T a, T b)
{
    return min(a, b);
}
T maxIntegerComparable<T : IInteger & IComparable>(T a, T b)
{
    return max(a, b);
}
T minIntegerArithmetic<T : IInteger & IArithmetic>(T a, T b)
{
    return min(a, b);
}

// This file only compiles if `min` on a type that is both `IFloat` and `IInteger` is unambiguous.
// Both candidate overloads dispatch to the same `__IMinMax` witness, so either choice is correct.
T minFloatAndInteger<T : IFloat & IInteger>(T a, T b)
{
    return min(a, b);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float3 a = float3(1, 5, 3), b = float3(4, 2, 6);
    int3 c = int3(9, 2, 5), d = int3(3, 8, 5);

    float3 r0 = minFloatComparable(a, b);
    float3 r1 = maxFloatComparableWhere(a, b);
    float3 r2 = minFloatArithmetic(a, b);
    int3 r3 = maxIntegerComparable(c, d);
    int3 r4 = minIntegerArithmetic(c, d);

    int i = 0;
    outputBuffer[i++] = r0.x; // CHECK: 1.0
    outputBuffer[i++] = r0.y; // CHECK-NEXT: 2.0
    outputBuffer[i++] = r0.z; // CHECK-NEXT: 3.0
    outputBuffer[i++] = r1.x; // CHECK-NEXT: 4.0
    outputBuffer[i++] = r1.y; // CHECK-NEXT: 5.0
    outputBuffer[i++] = r1.z; // CHECK-NEXT: 6.0
    outputBuffer[i++] = r2.x; // CHECK-NEXT: 1.0
    outputBuffer[i++] = r2.y; // CHECK-NEXT: 2.0
    outputBuffer[i++] = r2.z; // CHECK-NEXT: 3.0
    outputBuffer[i++] = float(r3.x); // CHECK-NEXT: 9.0
    outputBuffer[i++] = float(r3.y); // CHECK-NEXT: 8.0
    outputBuffer[i++] = float(r3.z); // CHECK-NEXT: 5.0
    outputBuffer[i++] = float(r4.x); // CHECK-NEXT: 3.0
    outputBuffer[i++] = float(r4.y); // CHECK-NEXT: 2.0
    outputBuffer[i++] = float(r4.z); // CHECK-NEXT: 5.0
}
