// bitcast-eligible-16bit.slang
// Test that 16-bit types use native OpBitcast optimization

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// 16-bit types should use direct OpBitcast (no lowering/splitting)
// CHECK-DAG: OpBitcast %half %
// CHECK-DAG: OpBitcast %ushort %
// CHECK-DAG: OpBitcast %short %
// CHECK-DAG: OpBitcast %v2half %
// CHECK-DAG: OpBitcast %v4short %

//TEST_INPUT:ubuffer(data=[15360 49120 16384 17408 18432 100 200 300 400 500 600 700], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding
    
    // Basic 16-bit scalar types
    uint16_t u16 = uint16_t(inputBuffer[0] & 0xFFFF);
    float16_t f16 = bit_cast<float16_t>(u16);
    
    int16_t i16 = int16_t(inputBuffer[1] & 0xFFFF);
    uint16_t u16_result = bit_cast<uint16_t>(i16);
    
    float16_t f16_input = float16_t(1.5);
    int16_t i16_from_half = bit_cast<int16_t>(f16_input);
    
    // Vector types with 16-bit components
    vector<uint16_t, 2> u16v2 = vector<uint16_t, 2>(
        uint16_t(inputBuffer[2] & 0xFFFF),
        uint16_t(inputBuffer[3] & 0xFFFF)
    );
    vector<float16_t, 2> f16v2 = bit_cast<vector<float16_t, 2>>(u16v2);
    
    vector<int16_t, 3> i16v3 = vector<int16_t, 3>(
        int16_t(inputBuffer[4] & 0xFFFF),
        int16_t(inputBuffer[5] & 0xFFFF),
        int16_t(inputBuffer[6] & 0xFFFF)
    );
    vector<uint16_t, 3> u16v3 = bit_cast<vector<uint16_t, 3>>(i16v3);
    
    vector<float16_t, 4> f16v4 = vector<float16_t, 4>(
        float16_t(asfloat16(uint16_t(inputBuffer[7] & 0xFFFF))),
        float16_t(asfloat16(uint16_t(inputBuffer[8] & 0xFFFF))),
        float16_t(asfloat16(uint16_t(inputBuffer[9] & 0xFFFF))),
        float16_t(asfloat16(uint16_t(inputBuffer[10] & 0xFFFF)))
    );
    vector<int16_t, 4> i16v4 = bit_cast<vector<int16_t, 4>>(f16v4);
    
    // Store results to prevent optimization
    outputBuffer[0] = uint(asuint16(f16));
    outputBuffer[1] = uint(u16_result);
    outputBuffer[2] = uint(i16_from_half);
    outputBuffer[3] = uint(asuint16(f16v2.x));
    outputBuffer[4] = uint(u16v3.x);
    outputBuffer[5] = uint(i16v4.x);
}

