//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

extension<
    T0 : __BuiltinArithmeticType,
    each Ts0 : __BuiltinArithmeticType,
    each Ts1 : __BuiltinArithmeticType
> Tuple<T0, T0, expand each Ts0, expand each Ts1>
{
    static int getSize_Ts0() { return countof(Ts0); }
    static int getSize_Ts1() { return countof(Ts1); }
}

[numthreads(1,1,1)]
void computeMain()
{
    int i = 2;
    float f0 = 3, f1 = 5;
    uint ui0 = 4, ui1 = 6;

    let s0 = makeTuple(i, i); // T, T
    let s1 = makeTuple(i, i, f0, ui0); // T, T, Ts0, Ts1
    let s2 = makeTuple(i, i, f0, ui0, f1, ui1); // T, T, Ts0, Ts0, Ts1, Ts1

    outputBuffer[0] = s0.getSize_Ts0();
    outputBuffer[1] = s0.getSize_Ts1();
    outputBuffer[2] = s1.getSize_Ts0();
    outputBuffer[3] = s1.getSize_Ts1();
    outputBuffer[4] = s2.getSize_Ts0();
    outputBuffer[5] = s2.getSize_Ts1();

    // CHECK-COUNT-2:0
    // CHECK-COUNT-2:1
    // CHECK-COUNT-2:2
}
