// `[vk::offset]` on a struct field moves the field only on SPIR-V and GLSL.
// Every other target lays out the struct naturally, which is the layout that
// reflection reports, so `ByteAddressBuffer.Load<S>` reads `b` from byte 4
// there and from byte 16 on Vulkan.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=VK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=NATURAL): -cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=NATURAL): -cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

struct S
{
    int a;
    [vk::offset(16)] int b;
}

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6], stride=4):name=input
ByteAddressBuffer input;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s = input.Load<S>(0);
    outputBuffer[0] = s.a;
    outputBuffer[1] = s.b;
}

// VK: type: int32_t
// VK-NEXT: 1
// VK-NEXT: 5

// NATURAL: type: int32_t
// NATURAL-NEXT: 1
// NATURAL-NEXT: 2

// HLSL: .Load<int >(4U)
// METAL: [(4U)>>2]
// WGSL: [(u32(4))/4]
