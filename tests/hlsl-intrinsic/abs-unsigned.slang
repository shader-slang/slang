// `abs` of an unsigned 32-bit or 64-bit integer returns its input unchanged (#13983).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature int64 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -render-feature int64 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type

// Signed element types still lower to `SAbs`; unsigned ones emit no `SAbs` at all. The
// `OpEntryPoint` anchor fails the rows when compilation fails, which glslang does when the GLSL
// path emits `abs` of an unsigned value.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV_SIGNED):-target spirv-asm -entry computeMain -stage compute
// SPV: OpEntryPoint
// SPV-NOT: OpExtInst %{{(u|v[0-9]u)}}{{[a-z0-9]*}} %{{[A-Za-z0-9_]+}} SAbs
// SPV_SIGNED-DAG: OpExtInst %int %{{[A-Za-z0-9_]+}} SAbs
// SPV_SIGNED-DAG: OpExtInst %v2int %{{[A-Za-z0-9_]+}} SAbs

// The input comes from a buffer so that no constant folding is involved.
//TEST_INPUT:ubuffer(data=[0xFFFFFFFF 0x80000000 5 0xFFFFFFFB], stride=4):name=inBuf
RWStructuredBuffer<uint> inBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint allOnes = inBuf[0];
    uint five = inBuf[2];

    outBuf[0] = abs(allOnes);
    // The INT_MIN bit pattern is poison for a signed `llvm.abs` with `i1 true`, so this checks that
    // unsigned `abs` never reaches that lowering.
    uint intMinBits = inBuf[1];
    outBuf[1] = abs(intMinBits);
    outBuf[2] = abs(five);

    uint2 v = abs(uint2(allOnes, five));
    outBuf[3] = v.x;
    outBuf[4] = v.y;

    uint64_t wideAllOnes = (uint64_t(allOnes) << 32) | allOnes;
    uint64_t w = abs(wideAllOnes);
    outBuf[5] = uint(w >> 32);
    outBuf[6] = uint(w);
    vector<uint64_t, 2> wv = abs(vector<uint64_t, 2>(wideAllOnes, uint64_t(five)));
    outBuf[7] = uint(wv.x >> 32);
    outBuf[8] = uint(wv.y);

    uint2x2 m = abs(uint2x2(allOnes, five, five, allOnes));
    outBuf[9] = m[0][0];
    outBuf[10] = m[1][1];

    // Signed controls: `abs(-5)` is still 5.
    int minusFive = int(inBuf[3]);
    outBuf[11] = uint(abs(minusFive));
    outBuf[12] = uint(abs(int2(minusFive, 7)).x);
}

// CHECK: 4294967295
// CHECK-NEXT: 2147483648
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: 4294967295
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: 4294967295
// CHECK-NEXT: 4294967295
// CHECK-NEXT: 4294967295
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: 4294967295
// CHECK-NEXT: 4294967295
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}5{{$}}
