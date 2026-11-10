//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly -Xslang -O0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -output-using-type -emit-spirv-directly -Xslang -O1
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain -validate-ir -validate-ir-detailed

// We write through a selected image and read both images afterward. Each invocation writes
// a different texel, so the unselected image must still contain its initial zero value.
// Executing both writes speculatively would fail the comparison even if the selected image
// received the correct value.
// We run the write/read comparisons at both `-O0` and `-O1`.
// SPIRV: OpImageWrite
// GLSL: imageStore
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content=zero):name textureA
RWTexture2D<uint> textureA;
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content=zero):name textureB
RWTexture2D<uint> textureB;
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    bool chooseA = (threadID.x & 1) == 0;
    uint2 coordinate = uint2(threadID.x, 0);
    uint value = threadID.x + 1;
    RWTexture2D<uint> selected = select(chooseA, textureA, textureB);
    selected[coordinate] = value;
    AllMemoryBarrier();

    uint expectedA = chooseA ? value : 0;
    uint expectedB = chooseA ? 0 : value;
    bool correctA = textureA[coordinate] == expectedA;
    bool correctB = textureB[coordinate] == expectedB;
    outputBuffer[threadID.x] = correctA && correctB ? 1 : 0;
}

// CHECK: type: uint32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
