//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Regression test for issue #13114: element-wise vector min/max under __BuiltinArithmeticType.
//
// Before the fix, calling min/max on a `vector<T,N>` from inside a function generic over
// `T : __BuiltinArithmeticType` failed overload resolution with error E39999 ("no overload for
// 'min' applicable to arguments of type (vector<T,N>, vector<T,N>)"): vector min/max overloads
// existed only under the narrower __BuiltinIntegerType / __BuiltinFloatingPointType bounds, and the
// common base __BuiltinArithmeticType satisfies neither. This test confirms the generic helper now
// resolves and computes element-wise correctly for both integer and floating-point element types,
// and that concrete (non-generic) float4 calls still resolve unambiguously.

vector<T, N> componentMinimum<T : __BuiltinArithmeticType, let N : int>(
    vector<T, N> lhs,
    vector<T, N> rhs)
{
    return min(lhs, rhs);
}

vector<T, N> componentMaximum<T : __BuiltinArithmeticType, let N : int>(
    vector<T, N> lhs,
    vector<T, N> rhs)
{
    return max(lhs, rhs);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Floating-point element type through the generic __BuiltinArithmeticType helper.
    float4 fa = float4(1.0f, 5.0f, 3.0f, 8.0f);
    float4 fb = float4(4.0f, 2.0f, 3.0f, 6.0f);
    float4 fmin = componentMinimum(fa, fb); // (1, 2, 3, 6)
    float4 fmax = componentMaximum(fa, fb); // (4, 5, 3, 8)

    // CHECK: 1.0
    outputBuffer[0] = fmin.x;
    // CHECK: 2.0
    outputBuffer[1] = fmin.y;
    // CHECK: 3.0
    outputBuffer[2] = fmin.z;
    // CHECK: 6.0
    outputBuffer[3] = fmin.w;
    // CHECK: 4.0
    outputBuffer[4] = fmax.x;
    // CHECK: 5.0
    outputBuffer[5] = fmax.y;
    // CHECK: 3.0
    outputBuffer[6] = fmax.z;
    // CHECK: 8.0
    outputBuffer[7] = fmax.w;

    // Integer element type exercises the same generic overload with a different builtin element,
    // and its element-wise body decomposes to the scalar path (no whole-vector intrinsic).
    int3 ia = int3(7, 2, 9);
    int3 ib = int3(3, 8, 9);
    int3 imin = componentMinimum(ia, ib); // (3, 2, 9)
    int3 imax = componentMaximum(ia, ib); // (7, 8, 9)

    // CHECK: 3.0
    outputBuffer[8] = float(imin.x);
    // CHECK: 2.0
    outputBuffer[9] = float(imin.y);
    // CHECK: 9.0
    outputBuffer[10] = float(imin.z);
    // CHECK: 7.0
    outputBuffer[11] = float(imax.x);
    // CHECK: 8.0
    outputBuffer[12] = float(imax.y);
    // CHECK: 9.0
    outputBuffer[13] = float(imax.z);

    // Concrete float4 calls must still resolve to the existing narrow overload (no ambiguity
    // introduced by the new __BuiltinArithmeticType overload).
    float4 dmin = min(fa, fb); // (1, 2, 3, 6)
    float4 dmax = max(fa, fb); // (4, 5, 3, 8)
    // CHECK: 1.0
    outputBuffer[14] = dmin.x;
    // CHECK: 8.0
    outputBuffer[15] = dmax.w;
}
