//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain

// Issue #13114: min/max must resolve element-wise for a `vector<T,N>` whose element type is only
// constrained by `__BuiltinArithmeticType` (the common base of the integer and floating-point
// builtin interfaces), for both integer and floating-point elements. The new fallback overload
// must also leave concrete vector min/max unambiguous and must not displace the specialized
// integer/floating-point vector overloads (concrete vectors must still emit the vector intrinsic).

vector<T, N> componentMinimum<T : __BuiltinArithmeticType, let N : int>(vector<T, N> lhs, vector<T, N> rhs)
{
    return min(lhs, rhs);
}
vector<T, N> componentMaximum<T : __BuiltinArithmeticType, let N : int>(vector<T, N> lhs, vector<T, N> rhs)
{
    return max(lhs, rhs);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // `tid.x` is 0 at runtime (single-thread dispatch) so the results are deterministic, but it is
    // not compile-time foldable, so the min/max operations survive to the emitted code for the SPV
    // checks below.
    let n = float(tid.x);
    let m = int(tid.x);

    float4 fa = float4(1, 5, 3, 8) + n;
    float4 fb = float4(4, 2, 3, 6) + n;
    int4 ia = int4(7, 2, 9, 1) + m;
    int4 ib = int4(3, 8, 9, 4) + m;

    // Generic __BuiltinArithmeticType path — the feature under test — with float and int elements.
    float4 gfmin = componentMinimum(fa, fb);
    float4 gfmax = componentMaximum(fa, fb);
    int4 gimin = componentMinimum(ia, ib);
    int4 gimax = componentMaximum(ia, ib);
    // CHECK: 1.0
    outputBuffer[0] = gfmin.x;
    // CHECK: 2.0
    outputBuffer[1] = gfmin.y;
    // CHECK: 6.0
    outputBuffer[2] = gfmin.w;
    // CHECK: 5.0
    outputBuffer[3] = gfmax.y;
    // CHECK: 8.0
    outputBuffer[4] = gfmax.w;
    // CHECK: 3.0
    outputBuffer[5] = float(gimin.x);
    // CHECK: 2.0
    outputBuffer[6] = float(gimin.y);
    // CHECK: 8.0
    outputBuffer[7] = float(gimax.y);
    // CHECK: 4.0
    outputBuffer[8] = float(gimax.w);

    // Concrete vector calls must still resolve unambiguously to the specialized overloads.
    float4 cfmin = min(fa, fb);
    int4 cimax = max(ia, ib);
    // CHECK: 1.0
    outputBuffer[9] = cfmin.x;
    // CHECK: 8.0
    outputBuffer[10] = float(cimax.y);
}

// The concrete `float4`/`int4` calls emit the whole-vector intrinsic (the specialized overloads
// win; the __BuiltinArithmeticType fallback is ranked below them), rather than decomposing
// element-wise as the generic calls do.
// SPV-DAG: OpExtInst %v4float %{{[0-9]+}} FMin
// SPV-DAG: OpExtInst %v4int %{{[0-9]+}} SMax
