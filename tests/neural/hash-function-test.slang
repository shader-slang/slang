// Test hash functions for grid encoding
// Reference: tiny-cuda-nn common_device.h hash implementations

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -xslang -experimental-feature -output-using-type
import slang.neural;

// Output buffer for test results (22 results total)
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<uint> gOutputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Test positions with pseudo-random values for better coverage
    // 1D position: [42]
    Array<uint32_t, 1> pos1D;
    pos1D[0] = 42u;

    // 2D position: [17, 53]
    Array<uint32_t, 2> pos2D;
    pos2D[0] = 17u;
    pos2D[1] = 53u;

    // 3D position: [137, 281, 419]
    Array<uint32_t, 3> pos3D;
    pos3D[0] = 137u;
    pos3D[1] = 281u;
    pos3D[2] = 419u;

    // 4D position: [523, 661, 797, 937]
    Array<uint32_t, 4> pos4D;
    pos4D[0] = 523u;
    pos4D[1] = 661u;
    pos4D[2] = 797u;
    pos4D[3] = 937u;

    int idx = 0;

    // primeHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = neural_unittest.testPrimeHash<1>(pos1D);
    gOutputBuffer[idx++] = neural_unittest.testPrimeHash<2>(pos2D);
    gOutputBuffer[idx++] = neural_unittest.testPrimeHash<3>(pos3D);
    gOutputBuffer[idx++] = neural_unittest.testPrimeHash<4>(pos4D);

    // coherentPrimeHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = neural_unittest.testCoherentPrimeHash<1>(pos1D);
    gOutputBuffer[idx++] = neural_unittest.testCoherentPrimeHash<2>(pos2D);
    gOutputBuffer[idx++] = neural_unittest.testCoherentPrimeHash<3>(pos3D);
    gOutputBuffer[idx++] = neural_unittest.testCoherentPrimeHash<4>(pos4D);

    // reversedPrimeHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = neural_unittest.testReversedPrimeHash<1>(pos1D);
    gOutputBuffer[idx++] = neural_unittest.testReversedPrimeHash<2>(pos2D);
    gOutputBuffer[idx++] = neural_unittest.testReversedPrimeHash<3>(pos3D);
    gOutputBuffer[idx++] = neural_unittest.testReversedPrimeHash<4>(pos4D);

    // baseConvertHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = neural_unittest.testBaseConvertHash<1>(pos1D);
    gOutputBuffer[idx++] = neural_unittest.testBaseConvertHash<2>(pos2D);
    gOutputBuffer[idx++] = neural_unittest.testBaseConvertHash<3>(pos3D);
    gOutputBuffer[idx++] = neural_unittest.testBaseConvertHash<4>(pos4D);

    // rngHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = neural_unittest.testRngHash<1>(pos1D);
    gOutputBuffer[idx++] = neural_unittest.testRngHash<2>(pos2D);
    gOutputBuffer[idx++] = neural_unittest.testRngHash<3>(pos3D);
    gOutputBuffer[idx++] = neural_unittest.testRngHash<4>(pos4D);

    // gridHash dispatcher tests (using 3D to verify it calls the right function)
    gOutputBuffer[idx++] = neural_unittest.testGridHashPrime< 3>(pos3D);
    gOutputBuffer[idx++] = neural_unittest.testGridHashBaseConvert< 3>(pos3D);
}

// Expected values (run hashgrid_reference.cu to generate these):
// BUF: 647341262
// BUF-NEXT: 18683294
// BUF-NEXT: 1324500725
// BUF-NEXT: 554810016
// BUF-NEXT: 42
// BUF-NEXT: 3246141876
// BUF-NEXT: 959564575
// BUF-NEXT: 1506282578
// BUF-NEXT: 744915738
// BUF-NEXT: 623453704
// BUF-NEXT: 1733721819
// BUF-NEXT: 4199532894
// BUF-NEXT: 106302462
// BUF-NEXT: 3521949560
// BUF-NEXT: 1000400445
// BUF-NEXT: 491664306
// BUF-NEXT: 2625262495
// BUF-NEXT: 245355646
// BUF-NEXT: 3354014297
// BUF-NEXT: 4162357713
// BUF-NEXT: 1324500725
// BUF-NEXT: 1000400445
