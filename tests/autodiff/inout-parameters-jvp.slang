//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__differentiate_jvp void h(float x, float y, inout float z)
{
    float m = x + y;
    float n = x - y;
    z = z + m * n + 2 * x * y;
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float x = 2.0;
    float y = 3.5;
    float z = 1.0;
    float dx = 1.0;
    float dy = 0.5;
    float dz = 2.5;

    __jvp(h)(x, y, z, dx, dy, dz);

    outputBuffer[0] = dz; // Expect: 12.0
    outputBuffer[1] = z;  // Expect: 1.0

}