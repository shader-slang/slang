//TEST:SIMPLE(filecheck=CHECK):

// Regression test: when a user performs arithmetic on BFloat16/FloatE4M3/FloatE5M2
// the compiler fails overload resolution (these types are not __BuiltinArithmeticType)
// and should emit note E40017 pointing at the fp32-conversion pattern.
// See #10692 for the motivation — direct bf16/fp8 arithmetic is not supported by
// SPV_KHR_bfloat16 or SPV_EXT_float8 and is intentionally not offered.

RWStructuredBuffer<BFloat16>               outBufBF16;
RWStructuredBuffer<vector<BFloat16, 4>>    outBufBF16Vec;
RWStructuredBuffer<FloatE4M3>              outBufE4M3;
RWStructuredBuffer<FloatE5M2>              outBufE5M2;

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(1.0);
    BFloat16 b = BFloat16(2.0);
    //CHECK: E40017
    outBufBF16[0] = a + b;

    vector<BFloat16, 4> va = vector<BFloat16, 4>(BFloat16(1.0));
    vector<BFloat16, 4> vb = vector<BFloat16, 4>(BFloat16(2.0));
    //CHECK: E40017
    outBufBF16Vec[0] = va * vb;

    FloatE4M3 c = FloatE4M3(1.0);
    FloatE4M3 d = FloatE4M3(2.0);
    //CHECK: E40017
    outBufE4M3[0] = c + d;

    FloatE5M2 e = FloatE5M2(1.0);
    FloatE5M2 f = FloatE5M2(2.0);
    //CHECK: E40017
    outBufE5M2[0] = e - f;
}
