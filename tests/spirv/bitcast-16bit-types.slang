// Test that 16-bit types use OpBitcast directly in SPIR-V
// Tests uint16 -> float16 and uint16 -> int16 in scalar, vec2, vec3, vec4 forms

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Scalar: uint16 -> float16, uint16 -> int16
// CHECK-DAG: %f16 = OpBitcast %half %u16
// CHECK-DAG: %i16 = OpBitcast %short %u16

// Vec2: uint16×2 -> float16×2, uint16×2 -> int16×2
// CHECK-DAG: %f16v2 = OpBitcast %v2half %u16v2
// CHECK-DAG: %i16v2 = OpBitcast %v2short %u16v2

// Vec3: uint16×3 -> float16×3, uint16×3 -> int16×3
// CHECK-DAG: %f16v3 = OpBitcast %v3half %u16v3
// CHECK-DAG: %i16v3 = OpBitcast %v3short %u16v3

// Vec4: uint16×4 -> float16×4, uint16×4 -> int16×4
// CHECK-DAG: %f16v4 = OpBitcast %v4half %u16v4
// CHECK-DAG: %i16v4 = OpBitcast %v4short %u16v4

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[15360 49120 16384 17408 18432 100 200 300 400 500], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 15360
// OUT: 15360
// OUT: 49120
// OUT: 49120
// OUT: 17408
// OUT: 17408
// OUT: 200
// OUT: 200

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Scalar: uint16 -> float16, uint16 -> int16
    uint16_t u16 = uint16_t(inputBuffer[0] & 0xFFFF);
    float16_t f16 = bit_cast<float16_t>(u16);
    int16_t i16 = bit_cast<int16_t>(u16);

    // Vec2: uint16×2 -> float16×2, uint16×2 -> int16×2
    vector<uint16_t, 2> u16v2 = vector<uint16_t, 2>(
        uint16_t(inputBuffer[1] & 0xFFFF),
        uint16_t(inputBuffer[2] & 0xFFFF)
    );
    vector<float16_t, 2> f16v2 = bit_cast<vector<float16_t, 2>>(u16v2);
    vector<int16_t, 2> i16v2 = bit_cast<vector<int16_t, 2>>(u16v2);

    // Vec3: uint16×3 -> float16×3, uint16×3 -> int16×3
    vector<uint16_t, 3> u16v3 = vector<uint16_t, 3>(
        uint16_t(inputBuffer[3] & 0xFFFF),
        uint16_t(inputBuffer[4] & 0xFFFF),
        uint16_t(inputBuffer[5] & 0xFFFF)
    );
    vector<float16_t, 3> f16v3 = bit_cast<vector<float16_t, 3>>(u16v3);
    vector<int16_t, 3> i16v3 = bit_cast<vector<int16_t, 3>>(u16v3);

    // Vec4: uint16×4 -> float16×4, uint16×4 -> int16×4
    vector<uint16_t, 4> u16v4 = vector<uint16_t, 4>(
        uint16_t(inputBuffer[6] & 0xFFFF),
        uint16_t(inputBuffer[7] & 0xFFFF),
        uint16_t(inputBuffer[8] & 0xFFFF),
        uint16_t(inputBuffer[9] & 0xFFFF)
    );
    vector<float16_t, 4> f16v4 = bit_cast<vector<float16_t, 4>>(u16v4);
    vector<int16_t, 4> i16v4 = bit_cast<vector<int16_t, 4>>(u16v4);

    // Store results
    outputBuffer[0] = uint(asuint16(f16));
    outputBuffer[1] = uint(uint16_t(i16));
    outputBuffer[2] = uint(asuint16(f16v2.x));
    outputBuffer[3] = uint(uint16_t(i16v2.x));
    outputBuffer[4] = uint(asuint16(f16v3.x));
    outputBuffer[5] = uint(uint16_t(i16v3.x));
    outputBuffer[6] = uint(asuint16(f16v4.x));
    outputBuffer[7] = uint(uint16_t(i16v4.x));
}
