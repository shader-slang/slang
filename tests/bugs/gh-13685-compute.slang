// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// Runs the issue's pattern on a GPU: a struct whose texture field is never written is
// conditionally overwritten by another such struct, and the texture is never
// sampled because `enabled` stays false. Vulkan used to reject this with E56004.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct OptionalTexture
{
    bool enabled;
    Texture2D<float> texture;
};

//TEST_INPUT: set inputBuffer = ubuffer(data=[0 1], stride=4)
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

float readTexture(OptionalTexture value)
{
    if (value.enabled)
        return value.texture.Load(int3(0, 0, 0));
    return 1.0;
}

float select(bool replaceValue)
{
    OptionalTexture value;
    value.enabled = false;
    if (replaceValue)
    {
        OptionalTexture replacement;
        replacement.enabled = false;
        value = replacement;
    }
    return readTexture(value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1.0
    // CHECK: 1.0
    outputBuffer[0] = select(inputBuffer[0] != 0);
    outputBuffer[1] = select(inputBuffer[1] != 0);
}
