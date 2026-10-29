// CUDA has no floating-point atomicMin/atomicMax, so Slang lowers float, double and half atomic
// min/max (InterlockedMin/Max, Atomic<T>.min/max/reduceMin/reduceMax) to compare-and-swap loops
// in the CUDA prelude. https://github.com/shader-slang/slang/issues/14026

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

// 16-bit atomicCAS needs sm_70, above the sm_60 that half alone requires, so the emitter raises
// the SM version for half min/max. NVRTC before 12.8 defaults to an older architecture and rejects
// this row without that raise. NVRTC 12.8 and later default to compute_75, where the row passes
// either way, so it guards the raise only on older NVRTC.
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry halfMinMain -stage compute
// PTX: atom{{.*}}.cas.b16

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=floatBuffer
RWStructuredBuffer<float> floatBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=atomicFloatBuffer
RWStructuredBuffer<Atomic<float>> atomicFloatBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=8):name=doubleBuffer
RWStructuredBuffer<double> doubleBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=8):name=atomicDoubleBuffer
RWStructuredBuffer<Atomic<double>> atomicDoubleBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=2):name=halfBuffer
RWStructuredBuffer<half> halfBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=2):name=atomicHalfBuffer
RWStructuredBuffer<Atomic<half>> atomicHalfBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

groupshared float sharedMin;

static const uint NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    if (tid == 0)
    {
        floatBuffer[0] = 1.0;
        floatBuffer[1] = 1.0;
        atomicFloatBuffer[0].store(1.0);
        atomicFloatBuffer[1].store(1.0);
        atomicFloatBuffer[2].store(1.0);
        atomicFloatBuffer[3].store(1.0);
        doubleBuffer[0] = 1.0;
        doubleBuffer[1] = 1.0;
        doubleBuffer[2] = 2.0;
        doubleBuffer[3] = 0.0;
        atomicDoubleBuffer[0].store(1.0);
        atomicDoubleBuffer[1].store(1.0);
        halfBuffer[0] = half(1.0);
        halfBuffer[1] = half(1.0);
        atomicHalfBuffer[0].store(half(1.0));
        atomicHalfBuffer[1].store(half(1.0));
        sharedMin = 1.0;
    }
    AllMemoryBarrierWithGroupSync();

    // Mixed signs with every value used by two threads: -8.0, -7.5, ..., 7.5.
    // Comparing the bits as integers would order the negative values backwards.
    float v = float(int((tid * 7) % 32) - 16) * 0.5;

    InterlockedMin(floatBuffer[0], v);
    InterlockedMax(floatBuffer[1], v);
    atomicFloatBuffer[0].min(v);
    atomicFloatBuffer[1].max(v);
    atomicFloatBuffer[2].reduceMin(v);
    atomicFloatBuffer[3].reduceMax(v);

    InterlockedMin(doubleBuffer[0], double(v));
    InterlockedMax(doubleBuffer[1], double(v));
    atomicDoubleBuffer[0].reduceMin(double(v));
    atomicDoubleBuffer[1].reduceMax(double(v));

    InterlockedMin(halfBuffer[0], half(v));
    InterlockedMax(halfBuffer[1], half(v));
    atomicHalfBuffer[0].reduceMin(half(v));
    atomicHalfBuffer[1].max(half(v));

    InterlockedMin(sharedMin, v);

    // 1.0 and the next double above it round to the same float, so a min/max computed in float
    // precision could not tell them apart.
    double nextAfterOne = asdouble(0x00000001u, 0x3FF00000u);
    InterlockedMin(doubleBuffer[2], (tid & 1) != 0 ? nextAfterOne : 1.0);
    InterlockedMax(doubleBuffer[3], (tid & 1) != 0 ? nextAfterOne : 1.0);

    AllMemoryBarrierWithGroupSync();

    if (tid == 0)
    {
        outputBuffer[0] = floatBuffer[0];
        outputBuffer[1] = floatBuffer[1];
        outputBuffer[2] = atomicFloatBuffer[0].load();
        outputBuffer[3] = atomicFloatBuffer[1].load();
        outputBuffer[4] = atomicFloatBuffer[2].load();
        outputBuffer[5] = atomicFloatBuffer[3].load();
        outputBuffer[6] = float(doubleBuffer[0]);
        outputBuffer[7] = float(doubleBuffer[1]);
        outputBuffer[8] = float(atomicDoubleBuffer[0].load());
        outputBuffer[9] = float(atomicDoubleBuffer[1].load());
        outputBuffer[10] = float(halfBuffer[0]);
        outputBuffer[11] = float(halfBuffer[1]);
        outputBuffer[12] = float(atomicHalfBuffer[0].load());
        outputBuffer[13] = float(atomicHalfBuffer[1].load());
        outputBuffer[14] = sharedMin;
        outputBuffer[15] = (doubleBuffer[2] == 1.0 && doubleBuffer[3] == nextAfterOne) ? 1.0 : 0.0;

        // For each width: the original value is returned, the write is skipped when the stored
        // value already wins, a NaN operand is ignored, and a stored NaN is replaced (the
        // OpAtomicFMinEXT/OpAtomicFMaxEXT rules).
        floatBuffer[2] = 5.0;
        floatBuffer[3] = asfloat(0x7FC00000u);
        float o1, o2, o3, o4;
        InterlockedMin(floatBuffer[2], 3.0, o1);                   // 5 -> 3
        InterlockedMin(floatBuffer[2], 4.0, o2);                   // stays 3
        InterlockedMax(floatBuffer[2], asfloat(0x7FC00000u), o3);  // NaN operand: stays 3
        InterlockedMax(floatBuffer[3], -2.0, o4);                  // stored NaN -> -2
        bool ok = o1 == 5.0 && o2 == 3.0 && o3 == 3.0 && floatBuffer[2] == 3.0;
        ok = ok && isnan(o4) && floatBuffer[3] == -2.0;
        outputBuffer[16] = ok ? 1.0 : 0.0;

        doubleBuffer[4] = 5.0;
        doubleBuffer[5] = asdouble(0x00000000u, 0x7FF80000u);
        double d1, d2, d3, d4;
        InterlockedMin(doubleBuffer[4], 3.0, d1);
        InterlockedMin(doubleBuffer[4], 4.0, d2);
        InterlockedMax(doubleBuffer[4], asdouble(0x00000000u, 0x7FF80000u), d3);
        InterlockedMax(doubleBuffer[5], -2.0, d4);
        ok = d1 == 5.0 && d2 == 3.0 && d3 == 3.0 && doubleBuffer[4] == 3.0;
        ok = ok && isnan(d4) && doubleBuffer[5] == -2.0;
        outputBuffer[17] = ok ? 1.0 : 0.0;

        halfBuffer[2] = half(5.0);
        halfBuffer[3] = asfloat16(uint16_t(0x7E00));
        half h1, h2, h3, h4;
        InterlockedMin(halfBuffer[2], half(3.0), h1);
        InterlockedMin(halfBuffer[2], half(4.0), h2);
        InterlockedMax(halfBuffer[2], asfloat16(uint16_t(0x7E00)), h3);
        InterlockedMax(halfBuffer[3], half(-2.0), h4);
        ok = h1 == half(5.0) && h2 == half(3.0) && h3 == half(3.0) && halfBuffer[2] == half(3.0);
        ok = ok && isnan(h4) && halfBuffer[3] == half(-2.0);
        outputBuffer[18] = ok ? 1.0 : 0.0;
    }
}

// float: InterlockedMin/Max, Atomic<float>.min/max, .reduceMin/.reduceMax
// BUF: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// BUF-NEXT: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// BUF-NEXT: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// double: InterlockedMin/Max, Atomic<double>.reduceMin/.reduceMax
// BUF-NEXT: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// BUF-NEXT: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// half: InterlockedMin/Max, Atomic<half>.reduceMin, Atomic<half>.max
// BUF-NEXT: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// BUF-NEXT: {{^}}-8.0
// BUF-NEXT: {{^}}7.5
// groupshared float InterlockedMin
// BUF-NEXT: {{^}}-8.0
// double precision; float, double and half return-value and NaN checks
// BUF-NEXT: {{^}}1.0
// BUF-NEXT: {{^}}1.0
// BUF-NEXT: {{^}}1.0
// BUF-NEXT: {{^}}1.0

// A separate entry point keeps reduceMin's own sm_70 requirement out of the PTX row, so only the
// emitter's raise for half min/max is in play.
[numthreads(1, 1, 1)]
void halfMinMain(uint tid: SV_DispatchThreadID)
{
    InterlockedMin(halfBuffer[0], half(tid));
}
