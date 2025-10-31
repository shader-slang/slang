// Test vector bitcasts per SPIR-V spec
// Per SPIR-V spec: OpBitcast allows different component counts if:
// - Total bits match
// - Larger component count is an integer multiple of smaller component count

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// uint2 (2×32) <-> double (1×64)
// CHECK-DAG: OpBitcast %double %
// CHECK-DAG: OpBitcast %v2uint %

// uint64 (1×64) <-> float2 (2×32)
// CHECK-DAG: OpBitcast %v2float %

// float4 (4×32) <-> double2 (2×64)
// CHECK-DAG: OpBitcast %v2double %
// CHECK-DAG: OpBitcast %v4float %

// uint (1×32) <-> uint16_t×2 (2×16)
// CHECK-DAG: OpBitcast %v2ushort %
// CHECK-DAG: OpBitcast %uint %

// uint (1×32) <-> uint8_t×4 (4×8)
// CHECK-DAG: OpBitcast %v4uchar %

// int (1×32) <-> int16_t×2 (2×16)
// CHECK-DAG: OpBitcast %v2short %

// float (1×32) <-> float16_t×2 (2×16)
// CHECK-DAG: OpBitcast %v2half %

// uint64 (1×64) <-> uint16_t×4 (4×16)
// CHECK-DAG: OpBitcast %v4ushort %

// uint2 (2×32) <-> uint16_t×4 (4×16)
// (already checked above with v4ushort)

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[1072693248 1074790400 1065353216 1073741824 1077936128 1082130432 15360 16384 305419896 572662306], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 1082130434
// OUT: 1072693248
// OUT: 1072693248
// OUT: 1073741826
// OUT: 1065353216
// OUT: 15360
// OUT: 16384
// OUT: 86
// OUT: 0
// OUT: 0
// OUT: 1072693248
// OUT: 0
// OUT: 305419896
// OUT: 0
// OUT: 22136

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Test 1: uint2 (64-bit) <-> double (64-bit scalar)
    uint2 u2 = uint2(inputBuffer[0], inputBuffer[1]);
    double d1 = bit_cast<double>(u2);

    // Test 2: double (64-bit scalar) -> uint2 (64-bit)
    double d2 = d1;
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
    double2 d2v_input = d2v;
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

    // Test 8: uint (32-bit) -> vector<uint8_t, 4> (32-bit = 4×8-bit)
    uint u32_for_u8v4 = inputBuffer[8];
    vector<uint8_t, 4> u8v4 = bit_cast<vector<uint8_t, 4>>(u32_for_u8v4);

    // Test 9: vector<uint8_t, 4> (32-bit) -> uint (32-bit)
    vector<uint8_t, 4> u8v4_input = vector<uint8_t, 4>(
        uint8_t(inputBuffer[8] & 0xFF),
        uint8_t((inputBuffer[8] >> 8) & 0xFF),
        uint8_t((inputBuffer[8] >> 16) & 0xFF),
        uint8_t((inputBuffer[8] >> 24) & 0xFF)
    );
    uint u32_from_u8v4 = bit_cast<uint>(u8v4_input);

    // Test 10: int (32-bit) -> vector<int16_t, 2> (32-bit = 2×16-bit)
    int i32 = asint(inputBuffer[2]);
    vector<int16_t, 2> i16v2 = bit_cast<vector<int16_t, 2>>(i32);

    // Test 11: float (32-bit) -> vector<float16_t, 2> (32-bit = 2×16-bit)
    float f32 = asfloat(inputBuffer[2]);
    vector<float16_t, 2> f16v2 = bit_cast<vector<float16_t, 2>>(f32);

    // Test 12: uint64_t (64-bit) -> vector<uint16_t, 4> (64-bit = 4×16-bit)
    uint64_t u64_for_u16v4 = uint64_t(inputBuffer[0]) | (uint64_t(inputBuffer[1]) << 32);
    vector<uint16_t, 4> u16v4 = bit_cast<vector<uint16_t, 4>>(u64_for_u16v4);

    // Test 14: uint2 (64-bit) -> vector<uint16_t, 4> (64-bit = 4×16-bit)
    uint2 u2_for_u16v4 = uint2(inputBuffer[8], inputBuffer[9]);
    vector<uint16_t, 4> u16v4_from_u2 = bit_cast<vector<uint16_t, 4>>(u2_for_u16v4);

    outputBuffer[0] = asuint(float(d1));
    outputBuffer[1] = u2_result.x;
    outputBuffer[2] = asuint(f2.x);
    outputBuffer[3] = asuint(float(d2v.x));
    outputBuffer[4] = asuint(f4_result.x);
    outputBuffer[5] = uint(u16v2.x);
    outputBuffer[6] = u32_result;
    outputBuffer[7] = uint(u8v4.y);
    outputBuffer[8] = uint(i16v2.x);
    outputBuffer[9] = asuint(f16v2.x);
    outputBuffer[10] = asuint(f16v2.y);
    outputBuffer[11] = asuint(float(f16v2.x));
    outputBuffer[12] = u32_from_u8v4;
    outputBuffer[13] = uint(u16v4.x);
    outputBuffer[14] = uint(u16v4_from_u2.x);
}
