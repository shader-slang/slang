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

// All-unordered/tied families select the first lane in a shuffle-up tree, but the
// last earlier lane in an ascending scan. Finite increasing families have closed-form
// extrema. No second wave operation computes the expected result.
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = true;
    for (uint partition = 0; partition < 5; ++partition)
    {
        uint mask = 0xFFFFFFFFu;
        uint firstLane = 0;
        uint previousLane = lane - 1;
        bool tree = true;
        if (partition == 1)
        {
            mask = lane < 16 ? 0xFFFFu : 0xFFFF0000u;
            firstLane = lane < 16 ? 0 : 16;
            tree = lane < 16;
        }
        if (partition == 2)
        {
            mask = lane < 15 ? 0x7FFFu : 0xFFFF8000u;
            firstLane = lane < 15 ? 0 : 15;
            tree = false;
        }
        if (partition == 3)
        {
            mask = (lane & 1) != 0 ? 0xAAAAAAAAu : 0x55555555u;
            firstLane = lane & 1;
            previousLane = lane - 2;
            tree = false;
        }
        if (partition == 4)
        {
            mask = 1u << lane;
            firstLane = lane;
            tree = lane == 0;
        }
        uint selectedLane = lane == firstLane ? lane : tree ? firstLane : previousLane;
        uint4 members = uint4(mask, 0, 0, 0);
        for (uint family = 0; family < 5; ++family)
        {
            double4 value;
            uint64_t4 inclusiveMin, inclusiveMax, exclusiveMin, exclusiveMax;
            for (uint component = 0; component < 4; ++component)
            {
                uint first = family == 1 ? 4 : family == 2 ? 8 : 0;
                uint count = family == 2 ? 2 : 4;
                uint64_t bits = loadBits(first + (lane + component) % count);
                uint64_t selected = loadBits(first + (selectedLane + component) % count);
                inclusiveMin[component] = inclusiveMax[component] = selected;
                exclusiveMin[component] = exclusiveMax[component] = selected;
                if (family >= 3)
                {
                    uint64_t base = loadBits(family == 3 ? 10 : 11);
                    bits = base + lane + component + 1;
                    inclusiveMin[component] = exclusiveMin[component] = base + firstLane + component + 1;
                    inclusiveMax[component] = bits;
                    exclusiveMax[component] = base + previousLane + component + 1;
                }
                if (lane == firstLane)
                {
                    exclusiveMin[component] = uint64_t(0x7FF00000u) << 32;
                    exclusiveMax[component] = uint64_t(0xFFF00000u) << 32;
                }
                value[component] = bit_cast<double>(bits);
            }
            ok &= bit_cast<uint64_t>(WaveMultiPrefixInclusiveMin(value.x, members)) == inclusiveMin.x;
            ok &= bit_cast<uint64_t>(WaveMultiPrefixInclusiveMax(value.x, members)) == inclusiveMax.x;
            ok &= bit_cast<uint64_t>(WaveMultiPrefixExclusiveMin(value.x, members)) == exclusiveMin.x;
            ok &= bit_cast<uint64_t>(WaveMultiPrefixExclusiveMax(value.x, members)) == exclusiveMax.x;
            double4 imin = WaveMultiPrefixInclusiveMin(value, members);
            double4 imax = WaveMultiPrefixInclusiveMax(value, members);
            double4 emin = WaveMultiPrefixExclusiveMin(value, members);
            double4 emax = WaveMultiPrefixExclusiveMax(value, members);
            for (uint component = 0; component < 4; ++component)
            {
                ok &= bit_cast<uint64_t>(imin[component]) == inclusiveMin[component];
                ok &= bit_cast<uint64_t>(imax[component]) == inclusiveMax[component];
                ok &= bit_cast<uint64_t>(emin[component]) == exclusiveMin[component];
                ok &= bit_cast<uint64_t>(emax[component]) == exclusiveMax[component];
            }
        }
    }
    outputBuffer[lane] = ok ? 6700 + lane : 0;
}

// CHECK: 6700
// CHECK: 6701
// CHECK: 6702
// CHECK: 6703
// CHECK: 6704
// CHECK: 6705
// CHECK: 6706
// CHECK: 6707
// CHECK: 6708
// CHECK: 6709
// CHECK: 6710
// CHECK: 6711
// CHECK: 6712
// CHECK: 6713
// CHECK: 6714
// CHECK: 6715
// CHECK: 6716
// CHECK: 6717
// CHECK: 6718
// CHECK: 6719
// CHECK: 6720
// CHECK: 6721
// CHECK: 6722
// CHECK: 6723
// CHECK: 6724
// CHECK: 6725
// CHECK: 6726
// CHECK: 6727
// CHECK: 6728
// CHECK: 6729
// CHECK: 6730
// CHECK: 6731
