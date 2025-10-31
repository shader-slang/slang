// Test that 8-bit types use OpBitcast per SPIR-V spec
// Per spec: scalar or vector of numerical types must use OpBitcast

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Per SPIR-V spec: 8-bit types MUST use direct OpBitcast (no lowering/splitting)
// CHECK-DAG: OpBitcast %char %
// CHECK-DAG: OpBitcast %uchar %
// CHECK-DAG: OpBitcast %v2char %
// CHECK-DAG: OpBitcast %v3uchar %
// CHECK-DAG: OpBitcast %v4char %

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[42 200 10 20 30 100 150 200 1 2 3 4 5 6 7 8], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 42
// OUT: 200
// OUT: 10
// OUT: 100
// OUT: 150
// OUT: 4294967240

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding

    // Basic 8-bit scalar types
    uint8_t u8 = uint8_t(inputBuffer[0] & 0xFF);
    int8_t i8 = bit_cast<int8_t>(u8);

    int8_t i8_input = int8_t(inputBuffer[1] & 0xFF);
    uint8_t u8_result = bit_cast<uint8_t>(i8_input);

    // Vector types with 8-bit components
    vector<uint8_t, 2> u8v2 = vector<uint8_t, 2>(
        uint8_t(inputBuffer[2] & 0xFF),
        uint8_t(inputBuffer[3] & 0xFF)
    );
    vector<int8_t, 2> i8v2 = bit_cast<vector<int8_t, 2>>(u8v2);

    vector<int8_t, 3> i8v3 = vector<int8_t, 3>(
        int8_t(inputBuffer[4] & 0xFF),
        int8_t(inputBuffer[5] & 0xFF),
        int8_t(inputBuffer[6] & 0xFF)
    );
    vector<uint8_t, 3> u8v3 = bit_cast<vector<uint8_t, 3>>(i8v3);

    vector<uint8_t, 4> u8v4 = vector<uint8_t, 4>(
        uint8_t(inputBuffer[7] & 0xFF),
        uint8_t(inputBuffer[8] & 0xFF),
        uint8_t(inputBuffer[9] & 0xFF),
        uint8_t(inputBuffer[10] & 0xFF)
    );
    vector<int8_t, 4> i8v4 = bit_cast<vector<int8_t, 4>>(u8v4);

    // Store results to prevent optimization
    outputBuffer[0] = uint(i8);
    outputBuffer[1] = uint(u8_result);
    outputBuffer[2] = uint(i8v2.x);
    outputBuffer[3] = uint(u8v3.y);
    outputBuffer[4] = uint(u8v3.z);
    outputBuffer[5] = uint(i8v4.x);
}
