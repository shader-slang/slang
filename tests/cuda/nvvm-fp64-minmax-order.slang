//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime-loaded quiet/signaling NaNs, signed zeros, and finite/subnormal bases.
// Each double is stored as low/high uint32 words; payloads exercise both halves.
//TEST_INPUT:ubuffer(data=[305397761 2146959361 305397762 4294443010 1164413355 2146959651 1164413356 4294443299 305397761 2146435073 305397762 4293918722 1164413355 2146435363 1164413356 4293919011 0 0 0 2147483648 0 1072693248 0 0], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;

uint64_t loadBits(uint index)
{
    return uint64_t(inputBits[2 * index]) | (uint64_t(inputBits[2 * index + 1]) << 32);
}
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = true;
    for (uint partition = 0; partition < 5; ++partition)
    {
        uint mask = 0xFFFFFFFFu;
        uint selectedLane = lane ^ 31u;
        uint firstLane = 0, lastLane = 31;
        if (partition == 1)
        {
            mask = lane < 16 ? 0xFFFFu : 0xFFFF0000u;
            selectedLane = lane < 16 ? lane ^ 15u : 31;
            firstLane = lane < 16 ? 0 : 16;
            lastLane = lane < 16 ? 15 : 31;
        }
        if (partition == 2)
        {
            mask = lane < 15 ? 0x7FFFu : 0xFFFF8000u;
            selectedLane = lane < 15 ? 14 : 31;
            firstLane = lane < 15 ? 0 : 15;
            lastLane = lane < 15 ? 14 : 31;
        }
        if (partition == 3)
        {
            mask = (lane & 1) == 0 ? 0x55555555u : 0xAAAAAAAAu;
            selectedLane = (lane & 1) == 0 ? 30 : 31;
            firstLane = lane & 1;
            lastLane = selectedLane;
        }
        if (partition == 4)
        {
            mask = 1u << lane;
            selectedLane = lane;
            firstLane = lastLane = lane;
        }
        uint4 members = uint4(mask, 0, 0, 0);
        for (uint family = 0; family < 6; ++family)
        {
            uint first = family == 1 ? 4 : family == 3 ? 8 : 0;
            uint count = family == 2 ? 8 : family == 3 ? 2 : 4;
            uint64_t4 bits, expectedMin, expectedMax;
            for (uint component = 0; component < 4; ++component)
            {
                bits[component] = loadBits(first + (lane + component) % count);
                expectedMin[component] = loadBits(first + (selectedLane + component) % count);
                expectedMax[component] = expectedMin[component];
                if (family >= 4)
                {
                    // Positive finite values increase with their integer low-word payload.
                    // Family 4 distinguishes adjacent doubles; family 5 stays subnormal.
                    uint64_t base = loadBits(family == 4 ? 10 : 11);
                    bits[component] = base + lane + component + 1;
                    expectedMin[component] = base + firstLane + component + 1;
                    expectedMax[component] = base + lastLane + component + 1;
                }
            }
            // In the first four families comparisons are unordered or tied, selecting the second operand.
            // XOR stages select lane^(population-1); an ascending scan selects its last lane.
            // These closed-form indices provide the oracle without another wave operation.
            double4 value = double4(bit_cast<double>(bits.x), bit_cast<double>(bits.y),
                bit_cast<double>(bits.z), bit_cast<double>(bits.w));
            ok &= bit_cast<uint64_t>(WaveMultiMin(value.x, members)) == expectedMin.x;
            ok &= bit_cast<uint64_t>(WaveMultiMax(value.x, members)) == expectedMax.x;
            double4 minimum = WaveMultiMin(value, members);
            double4 maximum = WaveMultiMax(value, members);
            double2x2 matrixValue = double2x2(value.x, value.y, value.z, value.w);
            double2x2 matrixMinimum = WaveMultiMin(matrixValue, members);
            double2x2 matrixMaximum = WaveMultiMax(matrixValue, members);
            for (uint component = 0; component < 4; ++component)
            {
                ok &= bit_cast<uint64_t>(minimum[component]) == expectedMin[component];
                ok &= bit_cast<uint64_t>(maximum[component]) == expectedMax[component];
                ok &= bit_cast<uint64_t>(matrixMinimum[component / 2][component % 2]) == expectedMin[component];
                ok &= bit_cast<uint64_t>(matrixMaximum[component / 2][component % 2]) == expectedMax[component];
            }
        }
    }
    outputBuffer[lane] = ok ? 6200 + lane : 0;
}

// CHECK: 6200
// CHECK: 6201
// CHECK: 6202
// CHECK: 6203
// CHECK: 6204
// CHECK: 6205
// CHECK: 6206
// CHECK: 6207
// CHECK: 6208
// CHECK: 6209
// CHECK: 6210
// CHECK: 6211
// CHECK: 6212
// CHECK: 6213
// CHECK: 6214
// CHECK: 6215
// CHECK: 6216
// CHECK: 6217
// CHECK: 6218
// CHECK: 6219
// CHECK: 6220
// CHECK: 6221
// CHECK: 6222
// CHECK: 6223
// CHECK: 6224
// CHECK: 6225
// CHECK: 6226
// CHECK: 6227
// CHECK: 6228
// CHECK: 6229
// CHECK: 6230
// CHECK: 6231
