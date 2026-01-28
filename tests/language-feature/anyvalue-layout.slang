//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12  -profile cs_6_1 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -output-using-type

interface IFoo
{
    float getVal();
    uint64_t getPtrVal();
    uint64_t getDescriptorHandleLow();
    uint64_t getDescriptorHandleHigh();
}

struct Foo : IFoo
{
    column_major float3x2 m;
    int x;
    uint64_t ptr;
    DescriptorHandle<Texture2D<float4>> texHandle;
    DescriptorHandle<StructuredBuffer<float>> bufHandle;
    float getVal()
    {
        return m[2][0];
    }
    uint64_t getPtrVal()
    {
        return ptr;
    }
    uint64_t getDescriptorHandleLow()
    {
        // Return the reinterpreted bits of texHandle
        uint2 bits = reinterpret<uint2>(texHandle);
        return bits.x | (uint64_t(bits.y) << 32);
    }
    uint64_t getDescriptorHandleHigh()
    {
        // Return the reinterpreted bits of bufHandle
        uint2 bits = reinterpret<uint2>(bufHandle);
        return bits.x | (uint64_t(bits.y) << 32);
    }
}

//TEST_INPUT: type_conformance Foo:IFoo = 0

//TEST_INPUT: set gFoo = ubuffer(data=[0 0 0 0 1.0 2.0 3.0 4.0 5.0 6.0 0 0 1 2 256 512 768 1024], stride=4)
RWStructuredBuffer<IFoo> gFoo;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 3.0
    outputBuffer[0] = gFoo[0].getVal();

    // CHECK: 1.0
    outputBuffer[1] = gFoo[0].getPtrVal()&0xFFFFFFFF;

    // CHECK: 2.0
    outputBuffer[2] = gFoo[0].getPtrVal()>>32;

    // Test DescriptorHandle marshalling
    // CHECK: 256.0
    outputBuffer[3] = gFoo[0].getDescriptorHandleLow()&0xFFFFFFFF;

    // CHECK: 512.0
    outputBuffer[4] = gFoo[0].getDescriptorHandleLow()>>32;

    // CHECK: 768.0
    outputBuffer[5] = gFoo[0].getDescriptorHandleHigh()&0xFFFFFFFF;

    // CHECK: 1024.0
    outputBuffer[6] = gFoo[0].getDescriptorHandleHigh()>>32;
}