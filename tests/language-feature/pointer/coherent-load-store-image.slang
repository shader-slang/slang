//DISABLE_TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv -capability vk_mem_model
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -profile spirv_1_3 -capability vk_mem_model
// These tests are expected to fail, pointers to texels are
// currently a broken feature and do not work.
// Additionally, we do not allow texel pointers with `__getAddress`.


// Ensure SPIRV emits coherent operations here
// SPIRV: MakeTexelAvailable
// SPIRV: MakeTexelVisible

// CHECK: 0
// CHECK-NEXT: 5

//TEST_INPUT: RWTexture1D(format=R32Uint, size=8, content = one, mipMaps = 1):name=texture
RWTexture1D<uint> texture;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain()
{
    Ptr<uint> ptrIn = __getAddress(texture[1]);
    Ptr<uint> secondPtrIn = ptrIn;

    storeCoherent<4, MemoryScope::Device>(ptrIn, 5);
    outputBuffer[0] = loadCoherent<4, MemoryScope::Device>(ptrIn);
}
