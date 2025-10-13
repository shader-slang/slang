// bitcast-eligible-64bit.slang
// Test that 64-bit types use native OpBitcast optimization

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// 64-bit types should use direct OpBitcast (no lowering/splitting)
// CHECK-DAG: OpBitcast %double %
// CHECK-DAG: OpBitcast %ulong %
// CHECK-DAG: OpBitcast %long %
// CHECK-DAG: OpBitcast %v2double %

//TEST_INPUT:ubuffer(data=[1072693248 1074790400 1075838976 1077936128 0 1 2 3 4 5 6 7 8 9], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding
    
    // Basic 64-bit scalar types
    uint64_t u64 = uint64_t(inputBuffer[0]) | (uint64_t(inputBuffer[1]) << 32);
    double d64 = bit_cast<double>(u64);
    
    int64_t i64 = int64_t(inputBuffer[2]) | (int64_t(inputBuffer[3]) << 32);
    uint64_t u64_result = bit_cast<uint64_t>(i64);
    
    double d_input = 3.14159;
    int64_t i64_from_double = bit_cast<int64_t>(d_input);
    
    // Vector types with 64-bit components
    vector<uint64_t, 2> u64v2 = vector<uint64_t, 2>(
        uint64_t(inputBuffer[4]) | (uint64_t(inputBuffer[5]) << 32),
        uint64_t(inputBuffer[6]) | (uint64_t(inputBuffer[7]) << 32)
    );
    vector<double, 2> dv2 = bit_cast<vector<double, 2>>(u64v2);
    
    vector<int64_t, 3> i64v3 = vector<int64_t, 3>(
        int64_t(42),
        int64_t(84),
        int64_t(126)
    );
    vector<uint64_t, 3> u64v3 = bit_cast<vector<uint64_t, 3>>(i64v3);
    
    vector<double, 4> dv4 = vector<double, 4>(1.0, 2.0, 3.0, 4.0);
    vector<int64_t, 4> i64v4 = bit_cast<vector<int64_t, 4>>(dv4);
    
    // Store results to prevent optimization
    outputBuffer[0] = uint(asuint(float(d64)));
    outputBuffer[1] = uint(u64_result & 0xFFFFFFFF);
    outputBuffer[2] = uint(i64_from_double & 0xFFFFFFFF);
    outputBuffer[3] = uint(asuint(float(dv2.x)));
    outputBuffer[4] = uint(u64v3.x & 0xFFFFFFFF);
    outputBuffer[5] = uint(i64v4.x & 0xFFFFFFFF);
}

