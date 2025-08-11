//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -compile-arg -obfuscate -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -compile-arg -obfuscate -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float testBool<let doStuff : bool>()
{
    if (doStuff)
    {
        return 1.0;
    }
    else
    {
        return 2.0;
    }
}

enum Color {
    RED,
    GREEN
}

float testEnum<let e : Color>()
{
    if (e == Color::RED)
    {
        return 3.0;
    }
    else
    {
        return 4.0;
    }
}

[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: 1.0
    outputBuffer[0] = testBool<true>();
    // CHECK: 2.0
    outputBuffer[1] = testBool<false>();

    // CHECK: 3.0
    outputBuffer[2] = testEnum<Color::RED>();
    // CHECK: 4.0
    outputBuffer[3] = testEnum<Color::GREEN>();
}
