//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

void accept<params T>(expand each T value) {}

T writeSingle<T : __BuiltinFloatingPointType>(T value)
{
    static int counter = 0;
    outputBuffer[counter++] = __realCast<float>(value);
    return value;
}

void write<params T : __BuiltinFloatingPointType>(expand each T value)
{
    accept(expand writeSingle(each value));
}

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 1.0
    // CHECK: 2.0
    // CHECK: 3.0
    // CHECK: 4.0

    write(1.0, 2.0, 3.0, 4.0);
}