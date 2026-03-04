//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct test2
{
    uint32_t v;
    [mutating]
    void DoStuff() { v = 1337; }
};

struct test
{
    static test2 a;
    static void DoStuff() { a.DoStuff(); }
};

[shader("compute")]
void computeMain()
{
    test::DoStuff();
    outputBuffer[0] = test::a.v;

    // CHECK: type: int32_t
    // CHECK-NEXT: 1337
}
