//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

bool sameMatrix(double2x2 actual, double2x2 expected)
{
    return all(actual[0] == expected[0]) & all(actual[1] == expected[1]);
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint rank = lane / 2;
    uint mask = (lane & 1) == 0 ? 0x55555555u : 0xAAAAAAAAu;
    double base = 16777216.25l;
    double x = base + double(rank) * 0.25l;
    double factor = 1.0l + 1.0l / 1073741824.0l;
    double p = rank == 0 ? factor : 1.0l;
    double2x2 value = double2x2(x, -x, x * 2.0l, -x * 4.0l);
    double2x2 productValue = double2x2(p, p * 2.0l, p * 4.0l, -p);
    double sum = 16.0l * base + 30.0l;
    double2x2 reduced = WaveMultiSum(value, uint4(mask, 0, 0, 0));
    double2x2 product = WaveMultiProduct(productValue, uint4(mask, 0, 0, 0));
    bool ok = sameMatrix(reduced, double2x2(sum, -sum, sum * 2.0l, -sum * 4.0l));
    ok &= sameMatrix(product, double2x2(factor, factor * 65536.0l, factor * 4294967296.0l, factor));
    // The aggregate leaf recognizer also serves indexed transport. Exercise nested matrix values so widening that boundary has its own executable coverage.
    uint source = 30 - rank * 2 + (lane & 1);
    double expectedX = base + double(15 - rank) * 0.25l;
    double2x2 matrixShuffled = WaveMaskReadLaneAt(mask, value, int(source));
    ok &= sameMatrix(matrixShuffled, double2x2(expectedX, -expectedX, expectedX * 2.0l, -expectedX * 4.0l));
    if (lane < 16)
    {
        double2x2 explicitResult = WaveMaskReadLaneAt(0xFFFFu, value, int(15 - lane));
        double partialX = base + double((15 - lane) / 2) * 0.25l;
        double2x2 expected = double2x2(partialX, -partialX, partialX * 2.0l, -partialX * 4.0l);
        ok &= sameMatrix(explicitResult, expected);
    }
    outputBuffer[lane] = ok ? 4000 + lane : 0;
}

// CHECK: 4000
// CHECK: 4001
// CHECK: 4002
// CHECK: 4003
// CHECK: 4004
// CHECK: 4005
// CHECK: 4006
// CHECK: 4007
// CHECK: 4008
// CHECK: 4009
// CHECK: 4010
// CHECK: 4011
// CHECK: 4012
// CHECK: 4013
// CHECK: 4014
// CHECK: 4015
// CHECK: 4016
// CHECK: 4017
// CHECK: 4018
// CHECK: 4019
// CHECK: 4020
// CHECK: 4021
// CHECK: 4022
// CHECK: 4023
// CHECK: 4024
// CHECK: 4025
// CHECK: 4026
// CHECK: 4027
// CHECK: 4028
// CHECK: 4029
// CHECK: 4030
// CHECK: 4031
