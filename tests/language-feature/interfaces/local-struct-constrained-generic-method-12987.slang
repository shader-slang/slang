// Regression test for https://github.com/shader-slang/slang/issues/12987:
// a function-local struct may implement an interface whose requirement is a
// constrained generic method (`apply<T : __BuiltinArithmeticType>`), exactly as a
// module-scope struct can. It must compile and run rather than crash the compiler.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface IOp
{
    int apply<T : __BuiltinArithmeticType>(T value);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    struct Op : IOp
    {
        int apply<T : __BuiltinArithmeticType>(T value) { return 1; }
    };

    outputBuffer[0] = Op().apply(1);

    // Verify the full 4-element buffer: only element 0 is written.
    // CHECK: 1
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
    // CHECK-NEXT: 0
}
