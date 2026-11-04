//TEST:SIMPLE(filecheck=CHECK):-target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

// Only integer scalar initializers constant-fold, so a float scalar or a vector `static
// const` global that names a specialization constant also reaches the E31226 check. The
// specialization constant is not a mutable global, so these initializers are accepted and
// read the constant's default value when it is not overridden.

// CHECK-NOT: E31226
// CHECK: OpSpecConstant %float 1.5

[vk::constant_id(1)]
const float F = 1.5;

static const float scaled = F * 2.0;
static const float2 vec = float2(F, 1.0);

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = scaled;
    outputBuffer[1] = vec.x;
    outputBuffer[2] = vec.y;
    // BUF: 3.0
    // BUF-NEXT: 1.5
    // BUF-NEXT: 1.0
}
