//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECK_SPIRV):-target spirv -profile glsl_450 -stage compute -entry main

// Test that Ptr<> is correctly translated to non-readonly buffer reference in GLSL when writing to it
// CHECK_GLSL: layout(buffer_reference, std430, buffer_reference_align = 4) buffer BufferPointer
// CHECK_SPIRV: OpEntryPoint GLCompute

struct TestStruct {
    int value;
};

struct TestStructData {
    TestStruct testDatas[];
};

struct PushConstants {
    Ptr<TestStructData> test;
    int someCount;
};

[[vk_push_constant]]
PushConstants pc;

RWStructuredBuffer<int> Output;

[numthreads(16,1,1)]
void main(uint2 dispatchThreadId : SV_DispatchThreadID) {
    uint idx = dispatchThreadId.x;
    if(idx >= pc.someCount)
        return;
    
    // Read value
    uint someVal = pc.test.testDatas[0].value;
    
    // Write value - this should make the buffer reference non-readonly
    pc.test.testDatas[0].value = someVal + 1;
    
    Output[idx] = someVal;
} 