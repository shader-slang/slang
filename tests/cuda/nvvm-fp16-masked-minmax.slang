//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Raw binary16 inputs include both infinity signs, NaN payloads, signed zeros and
// both sides of the normal/subnormal boundary. No floating conversion builds them.
//TEST_INPUT:ubuffer(data=[0 32768 1 32769 1023 33791 1024 33792 15360 48128 15359 15361 31742 31743 64511 31744 64512 31745 64513 32085 64853 32256 65024 32767 65535 32001 64769 1008 33776], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// The expected operands have closed-form lane indices. For all-NaN or tied
// inputs, a reduction XOR tree selects the opposite lane, a prefix tree selects
// the first lane, and a scan selects the last eligible original lane. Increasing
// magnitudes instead select the first or last member. These expectations use no
// wave operation and do not reproduce the emitter's tree/scan loops.
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = true;
    for (uint partition = 0; partition < 5; ++partition)
    {
        uint mask = 0xFFFFFFFFu;
        uint firstLane = 0;
        uint lastLane = 31;
        uint previousLane = lane - 1;
        bool tree = true;
        if (partition == 1)
        {
            mask = lane < 16 ? 0xFFFFu : 0xFFFF0000u;
            firstLane = lane < 16 ? 0 : 16;
            lastLane = lane < 16 ? 15 : 31;
            tree = lane < 16;
        }
        if (partition == 2)
        {
            mask = lane < 15 ? 0x7FFFu : 0xFFFF8000u;
            firstLane = lane < 15 ? 0 : 15;
            lastLane = lane < 15 ? 14 : 31;
            tree = false;
        }
        if (partition == 3)
        {
            mask = (lane & 1) != 0 ? 0xAAAAAAAAu : 0x55555555u;
            firstLane = lane & 1;
            lastLane = firstLane + 30;
            previousLane = lane - 2;
            tree = false;
        }
        if (partition == 4)
        {
            mask = 1u << lane;
            firstLane = lastLane = lane;
            tree = lane == 0;
        }
        uint prefixLane = lane == firstLane ? lane : tree ? firstLane : previousLane;
        uint reductionLane = tree ? lane ^ lastLane : lastLane;
        uint4 members = uint4(mask, 0, 0, 0);
        for (uint family = 0; family < 30; ++family)
        {
            half4 value;
            uint4 rmin, rmax, imin, imax, emin, emax;
            for (uint component = 0; component < 4; ++component)
            {
                uint bits = inputBits[family < 25 ? family : 0];
                rmin[component] = rmax[component] = bits;
                imin[component] = imax[component] = bits;
                emin[component] = emax[component] = bits;
                if (family == 25 || family == 26)
                {
                    uint base = inputBits[family];
                    bits = base + lane + component * 32;
                    rmin[component] = rmax[component] = base + reductionLane + component * 32;
                    imin[component] = imax[component] = base + prefixLane + component * 32;
                    emin[component] = emax[component] = base + prefixLane + component * 32;
                }
                if (family == 27)
                {
                    bits = inputBits[(lane + component) & 1];
                    rmin[component] = rmax[component] = inputBits[(reductionLane + component) & 1];
                    imin[component] = imax[component] = inputBits[(prefixLane + component) & 1];
                    emin[component] = emax[component] = inputBits[(prefixLane + component) & 1];
                }
                if (family >= 28)
                {
                    uint base = inputBits[family - 1];
                    bits = base + lane + component;
                    uint first = base + firstLane + component;
                    uint last = base + lastLane + component;
                    uint previous = base + previousLane + component;
                    bool negative = family == 29;
                    rmin[component] = negative ? last : first;
                    rmax[component] = negative ? first : last;
                    imin[component] = negative ? bits : first;
                    imax[component] = negative ? first : bits;
                    emin[component] = negative ? previous : first;
                    emax[component] = negative ? first : previous;
                }
                // The exclusive source seeds are finite, so infinities do not
                // replace them in the corresponding direction, even after lane 0.
                if (bits == 0x7C00u)
                    emin[component] = 0x7BFFu;
                if (bits == 0xFC00u)
                    emax[component] = 0xFBFFu;
                if (lane == firstLane)
                {
                    emin[component] = 0x7BFFu;
                    emax[component] = 0xFBFFu;
                }
                value[component] = bit_cast<half>(uint16_t(bits));
            }
            half rminscalar = WaveMultiMin(value.x, members);
            ok &= uint(bit_cast<uint16_t>(rminscalar)) == rmin.x;
            half rmaxscalar = WaveMultiMax(value.x, members);
            ok &= uint(bit_cast<uint16_t>(rmaxscalar)) == rmax.x;
            half iminscalar = WaveMultiPrefixInclusiveMin(value.x, members);
            ok &= uint(bit_cast<uint16_t>(iminscalar)) == imin.x;
            half imaxscalar = WaveMultiPrefixInclusiveMax(value.x, members);
            ok &= uint(bit_cast<uint16_t>(imaxscalar)) == imax.x;
            half eminscalar = WaveMultiPrefixExclusiveMin(value.x, members);
            ok &= uint(bit_cast<uint16_t>(eminscalar)) == emin.x;
            half emaxscalar = WaveMultiPrefixExclusiveMax(value.x, members);
            ok &= uint(bit_cast<uint16_t>(emaxscalar)) == emax.x;
            half2 rminvector2 = WaveMultiMin(value.xy, members);
            for (uint component = 0; component < 2; ++component)
                ok &= uint(bit_cast<uint16_t>(rminvector2[component])) == rmin[component];
            half2 rmaxvector2 = WaveMultiMax(value.xy, members);
            for (uint component = 0; component < 2; ++component)
                ok &= uint(bit_cast<uint16_t>(rmaxvector2[component])) == rmax[component];
            half2 iminvector2 = WaveMultiPrefixInclusiveMin(value.xy, members);
            for (uint component = 0; component < 2; ++component)
                ok &= uint(bit_cast<uint16_t>(iminvector2[component])) == imin[component];
            half2 imaxvector2 = WaveMultiPrefixInclusiveMax(value.xy, members);
            for (uint component = 0; component < 2; ++component)
                ok &= uint(bit_cast<uint16_t>(imaxvector2[component])) == imax[component];
            half2 eminvector2 = WaveMultiPrefixExclusiveMin(value.xy, members);
            for (uint component = 0; component < 2; ++component)
                ok &= uint(bit_cast<uint16_t>(eminvector2[component])) == emin[component];
            half2 emaxvector2 = WaveMultiPrefixExclusiveMax(value.xy, members);
            for (uint component = 0; component < 2; ++component)
                ok &= uint(bit_cast<uint16_t>(emaxvector2[component])) == emax[component];
            half4 rminvector4 = WaveMultiMin(value, members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(rminvector4[component])) == rmin[component];
            half4 rmaxvector4 = WaveMultiMax(value, members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(rmaxvector4[component])) == rmax[component];
            half4 iminvector4 = WaveMultiPrefixInclusiveMin(value, members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(iminvector4[component])) == imin[component];
            half4 imaxvector4 = WaveMultiPrefixInclusiveMax(value, members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(imaxvector4[component])) == imax[component];
            half4 eminvector4 = WaveMultiPrefixExclusiveMin(value, members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(eminvector4[component])) == emin[component];
            half4 emaxvector4 = WaveMultiPrefixExclusiveMax(value, members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(emaxvector4[component])) == emax[component];
            half2x2 rminmatrix = WaveMultiMin(half2x2(value.x, value.y, value.z, value.w), members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(rminmatrix[component / 2][component % 2])) == rmin[component];
            half2x2 rmaxmatrix = WaveMultiMax(half2x2(value.x, value.y, value.z, value.w), members);
            for (uint component = 0; component < 4; ++component)
                ok &= uint(bit_cast<uint16_t>(rmaxmatrix[component / 2][component % 2])) == rmax[component];
        }
    }
    outputBuffer[lane] = ok ? 7330 + lane : 0;
}

// CHECK: 7330
// CHECK: 7331
// CHECK: 7332
// CHECK: 7333
// CHECK: 7334
// CHECK: 7335
// CHECK: 7336
// CHECK: 7337
// CHECK: 7338
// CHECK: 7339
// CHECK: 7340
// CHECK: 7341
// CHECK: 7342
// CHECK: 7343
// CHECK: 7344
// CHECK: 7345
// CHECK: 7346
// CHECK: 7347
// CHECK: 7348
// CHECK: 7349
// CHECK: 7350
// CHECK: 7351
// CHECK: 7352
// CHECK: 7353
// CHECK: 7354
// CHECK: 7355
// CHECK: 7356
// CHECK: 7357
// CHECK: 7358
// CHECK: 7359
// CHECK: 7360
// CHECK: 7361
