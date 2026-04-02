//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Verify BFloat16 conversions to/from other types.

// float -> BFloat16 -> float roundtrip
// CHECK: 30

// half -> BFloat16 -> float
// CHECK: 20

// int -> BFloat16 -> float
// CHECK: 50

// BFloat16 -> half -> float
// CHECK: 40

// MakeVectorFromScalar: scalar broadcast to vector
// CHECK: 20
// CHECK: 20

// Mixed scalar-vector arithmetic
// CHECK: 40
// CHECK: 60

[numthreads(1,1,1)]
void computeMain()
{
    // float -> BFloat16 -> float
    float f = 3.0;
    BFloat16 bf = BFloat16(f);
    outputBuffer[0] = int(float(bf) * 10.0);

    // half -> BFloat16 -> float
    half h = half(2.0);
    BFloat16 bf2 = BFloat16(h);
    outputBuffer[1] = int(float(bf2) * 10.0);

    // int -> BFloat16 -> float
    int i = 5;
    BFloat16 bf3 = BFloat16(i);
    outputBuffer[2] = int(float(bf3) * 10.0);

    // BFloat16 -> half -> float
    BFloat16 bf4 = BFloat16(4.0);
    half h2 = half(bf4);
    outputBuffer[3] = int(float(h2) * 10.0);

    // MakeVectorFromScalar: construct vector from scalar
    BFloat16 scalar = BFloat16(2.0);
    vector<BFloat16, 2> v = vector<BFloat16, 2>(scalar);
    outputBuffer[4] = int(float(v[0]) * 10.0);
    outputBuffer[5] = int(float(v[1]) * 10.0);

    // Mixed scalar-vector: scalar * vector
    vector<BFloat16, 2> va = vector<BFloat16, 2>(BFloat16(2.0), BFloat16(3.0));
    let result = va * BFloat16(2.0);
    outputBuffer[6] = int(float(result[0]) * 10.0);
    outputBuffer[7] = int(float(result[1]) * 10.0);
}
