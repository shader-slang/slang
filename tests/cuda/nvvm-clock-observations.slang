//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// The expected affine-work results below were evaluated independently modulo 2^32.
//TEST_INPUT:ubuffer(data=[0 2 5 16 0 1013904223 0 1196435762 1649599747 2210837584], stride=4):name=inputs
StructuredBuffer<uint> inputs;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

uint64_t joinWords(uint2 words)
{
    return uint64_t(words.x) | (uint64_t(words.y) << 32);
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool ok = true;
    for (uint test = 0; test < 4; ++test)
    {
        uint rounds = inputs[test];
        uint work = inputs[4];
        uint64_t first = 0;
        uint64_t previous = 0;
        for (uint i = 0; i < rounds; ++i)
        {
            uint64_t a = joinWords(getRealtimeClock());
            uint middle = getRealtimeClockLow();
            uint64_t c = joinWords(getRealtimeClock());
            work = work * 1664525u + inputs[5];

            // These short observations use modular forward distances, allowing low-word wrap.
            // No clock frequency or exact tick value is assumed. Each read must remain live.
            uint64_t span = c - a;
            ok = ok && span < (uint64_t(1) << 63);
            ok = ok && uint64_t(middle - uint(a)) <= span;
            if (i == 0)
                first = a;
            else
                ok = ok && (a - previous) < (uint64_t(1) << 63);
            previous = c;
        }
        if (rounds > 1)
            ok = ok && previous != first && (previous - first) < (uint64_t(1) << 63);
        ok = ok && work == inputs[6 + test];
    }
    outputBuffer[tid.x] = uint(ok);
}

//CHECK: type: uint
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
//CHECK-NEXT: 1
