// Test hash functions for grid encoding
// Reference: tiny-cuda-nn common_device.h hash implementations

// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -xslang -experimental-feature -output-using-type
import neural;

// Output buffer for test results (22 results total)
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=gOutputBuffer
RWStructuredBuffer<uint> gOutputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Test positions with pseudo-random values for better coverage
    // 1D position: [42]
    uvec<1> pos1D;
    pos1D[0] = 42u;

    // 2D position: [17, 53]
    uvec<2> pos2D;
    pos2D[0] = 17u;
    pos2D[1] = 53u;

    // 3D position: [137, 281, 419]
    uvec<3> pos3D;
    pos3D[0] = 137u;
    pos3D[1] = 281u;
    pos3D[2] = 419u;

    // 4D position: [523, 661, 797, 937]
    uvec<4> pos4D;
    pos4D[0] = 523u;
    pos4D[1] = 661u;
    pos4D[2] = 797u;
    pos4D[3] = 937u;

    int idx = 0;

    // primeHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = primeHash<1>(pos1D);
    gOutputBuffer[idx++] = primeHash<2>(pos2D);
    gOutputBuffer[idx++] = primeHash<3>(pos3D);
    gOutputBuffer[idx++] = primeHash<4>(pos4D);

    // coherentPrimeHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = coherentPrimeHash<1>(pos1D);
    gOutputBuffer[idx++] = coherentPrimeHash<2>(pos2D);
    gOutputBuffer[idx++] = coherentPrimeHash<3>(pos3D);
    gOutputBuffer[idx++] = coherentPrimeHash<4>(pos4D);

    // reversedPrimeHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = reversedPrimeHash<1>(pos1D);
    gOutputBuffer[idx++] = reversedPrimeHash<2>(pos2D);
    gOutputBuffer[idx++] = reversedPrimeHash<3>(pos3D);
    gOutputBuffer[idx++] = reversedPrimeHash<4>(pos4D);

    // baseConvertHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = baseConvertHash<1>(pos1D);
    gOutputBuffer[idx++] = baseConvertHash<2>(pos2D);
    gOutputBuffer[idx++] = baseConvertHash<3>(pos3D);
    gOutputBuffer[idx++] = baseConvertHash<4>(pos4D);

    // rngHash for 1D, 2D, 3D, 4D
    gOutputBuffer[idx++] = rngHash<1>(pos1D);
    gOutputBuffer[idx++] = rngHash<2>(pos2D);
    gOutputBuffer[idx++] = rngHash<3>(pos3D);
    gOutputBuffer[idx++] = rngHash<4>(pos4D);

    // gridHash dispatcher tests (using 3D to verify it calls the right function)
    gOutputBuffer[idx++] = gridHash<HashType.Prime, 3>(pos3D);
    gOutputBuffer[idx++] = gridHash<HashType.BaseConvert, 3>(pos3D);
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
