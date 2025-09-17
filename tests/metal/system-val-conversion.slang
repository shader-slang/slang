//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -output-using-type

// Test that we always emit correct type for system value and insert conversion logic
// if the declared type of the SV is different from the spec-defined type.

//TEST_INPUT: ubuffer(data=[0], stride=1):out,name outputBuffer
uniform RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT: ubuffer(data=[5 0 0 0 0 0 0 0], stride=4, count=32):name buffer
RWByteAddressBuffer buffer;

// METALLIB: define void @computeMain

struct TestStruct
{
    uint8_t a;
    float b;
}

// METAL: void computeMain(uint3 tid{{.*}}
// METAL: uint(int(tid{{.*}}.x))

[numthreads(1,1,1)]
void computeMain(int tid: SV_DispatchThreadID)
{
    buffer.Store(16, buffer.Load<TestStruct>(tid));
    // BUF: 5
    outputBuffer[0] = buffer.Load<TestStruct>(16).a;
}
