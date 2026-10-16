// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// Runs the issue's pattern on Vulkan and on the CPU: a struct whose texture field is
// never written is conditionally overwritten by another such struct. The texture is
// never sampled because `enabled` stays false. The `tag` field differs between the
// two structs, so the result also shows which one was selected.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -g
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct OptionalTexture
{
    bool enabled;
    float tag;
    Texture2D<float4> texture;
};

//TEST_INPUT: set inputBuffer = ubuffer(data=[0 1], stride=4)
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float4> outputBuffer;

float4 readTexture(OptionalTexture value)
{
    if (value.enabled)
        return value.texture.Load(int3(0, 0, 0));
    return float4(0, 0, value.tag, 1);
}

float4 readOptionalTexture(bool replaceValue)
{
    OptionalTexture value;
    value.enabled = false;
    value.tag = 1;
    if (replaceValue)
    {
        OptionalTexture replacement;
        replacement.enabled = false;
        replacement.tag = 2;
        value = replacement;
    }
    return readTexture(value);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: type: float
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 1.0
    outputBuffer[0] = readOptionalTexture(inputBuffer[0] != 0);
    outputBuffer[1] = readOptionalTexture(inputBuffer[1] != 0);
}
