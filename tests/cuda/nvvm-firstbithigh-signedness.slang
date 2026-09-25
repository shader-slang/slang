//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Each row contains u32, u64 low/high, then expected u32/i32/u64/i64 bit indices.
// Expectations use integer bit_length-1; only signed negative words are complemented first.
// Runtime loads prevent the compiler from constant-folding the operation under test.
//TEST_INPUT:ubuffer(data=[0 0 0 4294967295 4294967295 4294967295 4294967295 1 1 0 0 0 0 0 2 2 0 1 1 1 1 3 3 0 1 1 1 1 4 4 0 2 2 2 2 7 7 0 2 2 2 2 8 8 0 3 3 3 3 15 15 0 3 3 3 3 16 16 0 4 4 4 4 31 31 0 4 4 4 4 32 32 0 5 5 5 5 63 63 0 5 5 5 5 64 64 0 6 6 6 6 255 4294967295 0 7 7 31 31 256 0 1 8 8 32 32 65535 1 1 15 15 32 32 65536 4294967295 1073741823 16 16 61 61 1073741823 0 1073741824 29 29 62 62 1073741824 1 1073741824 30 30 62 62 1073741825 1431655765 1431655765 30 30 62 62 1431655765 4294967294 2147483647 30 30 62 62 2147483646 4294967295 2147483647 30 30 62 62 2147483647 0 2147483648 30 30 63 62 2147483648 1 2147483648 31 30 63 62 2147483649 1 2147483649 31 30 63 62 2147549183 2863311530 2863311530 31 30 63 62 2863311530 0 3221225472 31 30 63 61 3221225472 4294967292 4294967295 31 29 63 1 4294967292 4294967293 4294967295 31 1 63 1 4294967293 4294967294 4294967295 31 1 63 0 4294967294 4294967295 4294967295 31 0 63 4294967295 4294967295 0 32768 31 4294967295 47 47], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint2 u;
    uint64_t2 w;
    uint2 expectedU, expectedI, expectedW, expectedS;
    for (uint component = 0; component < 2; ++component)
    {
        uint base = 7 * (lane ^ component);
        u[component] = inputWords[base];
        w[component] = uint64_t(inputWords[base + 1]) | (uint64_t(inputWords[base + 2]) << 32);
        expectedU[component] = inputWords[base + 3];
        expectedI[component] = inputWords[base + 4];
        expectedW[component] = inputWords[base + 5];
        expectedS[component] = inputWords[base + 6];
    }
    bool ok = firstbithigh(u.x) == expectedU.x;
    ok &= firstbithigh(int(u.x)) == expectedI.x;
    ok &= firstbithigh(w.x) == expectedW.x;
    ok &= firstbithigh(int64_t(w.x)) == expectedS.x;
    ok &= all(firstbithigh(u) == expectedU);
    ok &= all(firstbithigh(int2(u)) == expectedI);
    ok &= all(firstbithigh(w) == expectedW);
    ok &= all(firstbithigh(int64_t2(w)) == expectedS);
    outputBuffer[lane] = ok ? 6300 + lane : 0;
}

// CHECK: 6300
// CHECK: 6301
// CHECK: 6302
// CHECK: 6303
// CHECK: 6304
// CHECK: 6305
// CHECK: 6306
// CHECK: 6307
// CHECK: 6308
// CHECK: 6309
// CHECK: 6310
// CHECK: 6311
// CHECK: 6312
// CHECK: 6313
// CHECK: 6314
// CHECK: 6315
// CHECK: 6316
// CHECK: 6317
// CHECK: 6318
// CHECK: 6319
// CHECK: 6320
// CHECK: 6321
// CHECK: 6322
// CHECK: 6323
// CHECK: 6324
// CHECK: 6325
// CHECK: 6326
// CHECK: 6327
// CHECK: 6328
// CHECK: 6329
// CHECK: 6330
// CHECK: 6331
