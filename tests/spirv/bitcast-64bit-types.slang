// Test that 64-bit types use OpBitcast directly in SPIR-V
// Tests uint64 -> double and uint64 -> int64 in scalar, vec2, vec3, vec4 forms

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Scalar: uint64 -> double, uint64 -> int64
// CHECK-DAG: %d64 = OpBitcast %double %u64
// CHECK-DAG: %i64 = OpBitcast %long %u64

// Vec2: uint64×2 -> double×2, uint64×2 -> int64×2
// CHECK-DAG: %d64v2 = OpBitcast %v2double %u64v2
// CHECK-DAG: %i64v2 = OpBitcast %v2long %u64v2

// Vec3: uint64×3 -> double×3, uint64×3 -> int64×3
// CHECK-DAG: %d64v3 = OpBitcast %v3double %u64v3
// CHECK-DAG: %i64v3 = OpBitcast %v3long %u64v3

// Vec4: uint64×4 -> double×4, uint64×4 -> int64×4
// CHECK-DAG: %d64v4 = OpBitcast %v4double %u64v4
// CHECK-DAG: %i64v4 = OpBitcast %v4long %u64v4

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[100 0 200 0 300 0 400 0 500 0 600 0 700 0 800 0 900 0 1000 0], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 100
// OUT: 0
// OUT: 100
// OUT: 0
// OUT: 200
// OUT: 0
// OUT: 200
// OUT: 0
// OUT: 400
// OUT: 0
// OUT: 400
// OUT: 0
// OUT: 700
// OUT: 0
// OUT: 700
// OUT: 0

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Scalar: uint64 -> double, uint64 -> int64
    uint64_t u64 = uint64_t(inputBuffer[0]) | (uint64_t(inputBuffer[1]) << 32);
    double d64 = bit_cast<double>(u64);
    int64_t i64 = bit_cast<int64_t>(u64);

    // Vec2: uint64×2 -> double×2, uint64×2 -> int64×2
    vector<uint64_t, 2> u64v2 = vector<uint64_t, 2>(
        uint64_t(inputBuffer[2]) | (uint64_t(inputBuffer[3]) << 32),
        uint64_t(inputBuffer[4]) | (uint64_t(inputBuffer[5]) << 32)
    );
    vector<double, 2> d64v2 = bit_cast<vector<double, 2>>(u64v2);
    vector<int64_t, 2> i64v2 = bit_cast<vector<int64_t, 2>>(u64v2);

    // Vec3: uint64×3 -> double×3, uint64×3 -> int64×3
    vector<uint64_t, 3> u64v3 = vector<uint64_t, 3>(
        uint64_t(inputBuffer[6]) | (uint64_t(inputBuffer[7]) << 32),
        uint64_t(inputBuffer[8]) | (uint64_t(inputBuffer[9]) << 32),
        uint64_t(inputBuffer[10]) | (uint64_t(inputBuffer[11]) << 32)
    );
    vector<double, 3> d64v3 = bit_cast<vector<double, 3>>(u64v3);
    vector<int64_t, 3> i64v3 = bit_cast<vector<int64_t, 3>>(u64v3);

    // Vec4: uint64×4 -> double×4, uint64×4 -> int64×4
    vector<uint64_t, 4> u64v4 = vector<uint64_t, 4>(
        uint64_t(inputBuffer[12]) | (uint64_t(inputBuffer[13]) << 32),
        uint64_t(inputBuffer[14]) | (uint64_t(inputBuffer[15]) << 32),
        uint64_t(inputBuffer[16]) | (uint64_t(inputBuffer[17]) << 32),
        uint64_t(inputBuffer[18]) | (uint64_t(inputBuffer[19]) << 32)
    );
    vector<double, 4> d64v4 = bit_cast<vector<double, 4>>(u64v4);
    vector<int64_t, 4> i64v4 = bit_cast<vector<int64_t, 4>>(u64v4);

    // Store results as pairs of uint32s
    uint64_t r0 = bit_cast<uint64_t>(d64);
    outputBuffer[0] = uint(r0 & 0xFFFFFFFF);
    outputBuffer[1] = uint(r0 >> 32);

    uint64_t r1 = uint64_t(i64);
    outputBuffer[2] = uint(r1 & 0xFFFFFFFF);
    outputBuffer[3] = uint(r1 >> 32);

    uint64_t r2 = bit_cast<uint64_t>(d64v2.x);
    outputBuffer[4] = uint(r2 & 0xFFFFFFFF);
    outputBuffer[5] = uint(r2 >> 32);

    uint64_t r3 = uint64_t(i64v2.x);
    outputBuffer[6] = uint(r3 & 0xFFFFFFFF);
    outputBuffer[7] = uint(r3 >> 32);

    uint64_t r4 = bit_cast<uint64_t>(d64v3.x);
    outputBuffer[8] = uint(r4 & 0xFFFFFFFF);
    outputBuffer[9] = uint(r4 >> 32);

    uint64_t r5 = uint64_t(i64v3.x);
    outputBuffer[10] = uint(r5 & 0xFFFFFFFF);
    outputBuffer[11] = uint(r5 >> 32);

    uint64_t r6 = bit_cast<uint64_t>(d64v4.x);
    outputBuffer[12] = uint(r6 & 0xFFFFFFFF);
    outputBuffer[13] = uint(r6 >> 32);

    uint64_t r7 = uint64_t(i64v4.x);
    outputBuffer[14] = uint(r7 & 0xFFFFFFFF);
    outputBuffer[15] = uint(r7 >> 32);
}
