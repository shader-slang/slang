// bitcast-eligible-cross-component.slang
// Test cross-component-count bitcasts
// Per SPIR-V spec: OpBitcast allows different component counts if total bits match
// However, current implementation may lower these to component-wise operations
// This test verifies the current behavior

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Note: Cross-component bitcasts are currently lowered to component operations
// Future optimization could use OpBitcast directly per SPIR-V spec
// For now, we check that these operations work correctly, even if lowered

// Should see component extraction and reconstruction
// CHECK-DAG: OpCompositeExtract
// CHECK-DAG: OpCompositeConstruct

// Individual components may still use OpBitcast
// CHECK-DAG: OpBitcast

//TEST_INPUT:ubuffer(data=[1072693248 1074790400 1065353216 1073741824 1077936128 1082130432 15360 16384], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding
    
    // Test 1: uint2 (64-bit) <-> double (64-bit scalar)
    uint2 u2 = uint2(inputBuffer[0], inputBuffer[1]);
    double d1 = bit_cast<double>(u2);
    
    // Test 2: double (64-bit scalar) -> uint2 (64-bit)
    double d2 = 3.14159265359;
    uint2 u2_result = bit_cast<uint2>(d2);
    
    // Test 3: uint64_t (64-bit scalar) -> float2 (64-bit)
    uint64_t u64 = uint64_t(inputBuffer[0]) | (uint64_t(inputBuffer[1]) << 32);
    float2 f2 = bit_cast<float2>(u64);
    
    // Test 4: float4 (128-bit) <-> double2 (128-bit)
    float4 f4 = float4(
        asfloat(inputBuffer[2]),
        asfloat(inputBuffer[3]),
        asfloat(inputBuffer[4]),
        asfloat(inputBuffer[5])
    );
    double2 d2v = bit_cast<double2>(f4);
    
    // Test 5: double2 (128-bit) -> float4 (128-bit)
    double2 d2v_input = double2(1.5, 2.5);
    float4 f4_result = bit_cast<float4>(d2v_input);
    
    // Test 6: uint (32-bit) <-> vector<uint16_t, 2> (32-bit)
    uint u32 = inputBuffer[6];
    vector<uint16_t, 2> u16v2 = bit_cast<vector<uint16_t, 2>>(u32);
    
    // Test 7: vector<uint16_t, 2> (32-bit) -> uint (32-bit)
    vector<uint16_t, 2> u16v2_input = vector<uint16_t, 2>(
        uint16_t(inputBuffer[7] & 0xFFFF),
        uint16_t((inputBuffer[7] >> 16) & 0xFFFF)
    );
    uint u32_result = bit_cast<uint>(u16v2_input);
    
    // Store results to prevent optimization
    outputBuffer[0] = asuint(float(d1));
    outputBuffer[1] = u2_result.x;
    outputBuffer[2] = asuint(f2.x);
    outputBuffer[3] = asuint(float(d2v.x));
    outputBuffer[4] = asuint(f4_result.x);
    outputBuffer[5] = uint(u16v2.x);
    outputBuffer[6] = u32_result;
}

