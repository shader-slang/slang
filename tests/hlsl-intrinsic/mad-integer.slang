// Integer `mad` computes `mvalue * avalue + bvalue` in the operand type for scalar, vector, and
// matrix operands, so unsigned results wrap modulo the type's width.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature int64,int16 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -render-feature int64,int16 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type

// `GLSL.std.450 Fma` is only defined for floating-point result types, so integer `mad` lowers to
// `OpIMul` and `OpIAdd` while float `mad` keeps `Fma`. The `OpEntryPoint` anchor fails the `SPV`
// rows when compilation fails, which glslang does when the GLSL path emits `fma` on integers.
// The `-DAG` checks use their own prefix so that `SPV-NOT` scans the whole module instead of only
// the text before the first `-DAG` match. Each multiply is the only one of its type in the
// module, and each check pins the add that consumes it, because the shader's setup code and the
// matrix row loops emit unrelated `OpIAdd`s. The matrix calls are lowered row by row, so they
// show up as the `%v2int` and `%v2uint` pairs.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV_OPS):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV_OPS):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
// SPV: OpEntryPoint
// SPV-NOT: OpExtInst %{{[uv0-9]*(int|long|short|char)}} %{{[A-Za-z0-9_]+}} Fma
// SPV_OPS-DAG: %[[MUL_U32:[A-Za-z0-9_]+]] = OpIMul %uint
// SPV_OPS-DAG: OpIAdd %uint %[[MUL_U32]]{{ }}
// SPV_OPS-DAG: %[[MUL_I32:[A-Za-z0-9_]+]] = OpIMul %int
// SPV_OPS-DAG: OpIAdd %int %[[MUL_I32]]{{ }}
// SPV_OPS-DAG: %[[MUL_U32X3:[A-Za-z0-9_]+]] = OpIMul %v3uint
// SPV_OPS-DAG: OpIAdd %v3uint %[[MUL_U32X3]]{{ }}
// SPV_OPS-DAG: %[[MUL_I32X4:[A-Za-z0-9_]+]] = OpIMul %v4int
// SPV_OPS-DAG: OpIAdd %v4int %[[MUL_I32X4]]{{ }}
// SPV_OPS-DAG: %[[MUL_U64:[A-Za-z0-9_]+]] = OpIMul %ulong
// SPV_OPS-DAG: OpIAdd %ulong %[[MUL_U64]]{{ }}
// SPV_OPS-DAG: %[[MUL_I64:[A-Za-z0-9_]+]] = OpIMul %long
// SPV_OPS-DAG: OpIAdd %long %[[MUL_I64]]{{ }}
// SPV_OPS-DAG: %[[MUL_U64X2:[A-Za-z0-9_]+]] = OpIMul %v2ulong
// SPV_OPS-DAG: OpIAdd %v2ulong %[[MUL_U64X2]]{{ }}
// SPV_OPS-DAG: %[[MUL_I32X2:[A-Za-z0-9_]+]] = OpIMul %v2int
// SPV_OPS-DAG: OpIAdd %v2int %[[MUL_I32X2]]{{ }}
// SPV_OPS-DAG: %[[MUL_U32X2:[A-Za-z0-9_]+]] = OpIMul %v2uint
// SPV_OPS-DAG: OpIAdd %v2uint %[[MUL_U32X2]]{{ }}
// SPV_OPS-DAG: %[[MUL_U16:[A-Za-z0-9_]+]] = OpIMul %ushort
// SPV_OPS-DAG: OpIAdd %ushort %[[MUL_U16]]{{ }}
// SPV_OPS-DAG: %[[MUL_I16X2:[A-Za-z0-9_]+]] = OpIMul %v2short
// SPV_OPS-DAG: OpIAdd %v2short %[[MUL_I16X2]]{{ }}
// SPV_OPS-DAG: OpExtInst %float %{{[A-Za-z0-9_]+}} Fma

// HLSL keeps the native `mad` for all 12 calls: 11 integer calls and the float control.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
// HLSL-COUNT-12: mad(

// Every `mad` call has an operand loaded from a buffer, so none of them is constant-folded.
//TEST_INPUT:ubuffer(data=[0xFFFFFFFF 3 5 0xFFFFFFF9 6 2 10 7], stride=4):name=inBuf
RWStructuredBuffer<uint> inBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint allOnes = inBuf[0];
    uint three = inBuf[1];
    uint five = inBuf[2];
    int minusSeven = int(inBuf[3]);
    int six = int(inBuf[4]);
    uint two = inBuf[5];
    uint ten = inBuf[6];
    uint seven = inBuf[7];

    // 0xFFFFFFFF * 3 + 5 wraps to 2. The explicit `mad<uint>` also covers overload selection
    // with an explicit generic argument.
    outBuf[0] = int(mad<uint>(allOnes, three, five));
    // CHECK: {{^}}2{{$}}
    outBuf[1] = mad(minusSeven, six, int(five));
    // CHECK-NEXT: {{^}}-37{{$}}

    uint3 u3 = mad(uint3(allOnes, two, seven), uint3(three, allOnes, 1), uint3(five, ten, 0));
    outBuf[2] = int(u3.x);
    // CHECK-NEXT: {{^}}2{{$}}
    outBuf[3] = int(u3.y);
    // CHECK-NEXT: {{^}}8{{$}}
    outBuf[4] = int(u3.z);
    // CHECK-NEXT: {{^}}7{{$}}

    int4 i4 = mad(
        int4(minusSeven, six, -1, 100),
        int4(int(three), -8, -1, 3),
        int4(int(five), int(five), 0, -1));
    outBuf[5] = i4.x;
    // CHECK-NEXT: {{^}}-16{{$}}
    outBuf[6] = i4.y;
    // CHECK-NEXT: {{^}}-43{{$}}
    outBuf[7] = i4.z;
    // CHECK-NEXT: {{^}}1{{$}}
    outBuf[8] = i4.w;
    // CHECK-NEXT: {{^}}299{{$}}

    // 2^32 * (2^32 + 1) + 7 wraps to 2^32 + 7, which needs a full 64-bit product.
    uint64_t twoPow32 = uint64_t(allOnes) + 1;
    uint64_t u64 = mad(twoPow32, twoPow32 + 1, uint64_t(seven));
    outBuf[9] = int(u64 >> 32);
    // CHECK-NEXT: {{^}}1{{$}}
    outBuf[10] = int(uint(u64));
    // CHECK-NEXT: {{^}}7{{$}}

    int64_t i64 = mad(-int64_t(twoPow32), int64_t(three), int64_t(minusSeven));
    outBuf[11] = int(i64 >> 32);
    // CHECK-NEXT: {{^}}-4{{$}}
    outBuf[12] = int(uint(i64));
    // CHECK-NEXT: {{^}}-7{{$}}

    uint64_t wideAllOnes = (uint64_t(allOnes) << 32) | allOnes;
    vector<uint64_t, 2> u64v = mad(
        vector<uint64_t, 2>(twoPow32, wideAllOnes),
        vector<uint64_t, 2>(twoPow32 + 1, uint64_t(three)),
        vector<uint64_t, 2>(uint64_t(seven), uint64_t(five)));
    // The high half of `x` needs the full 64-bit product, and the low half of `y` wraps
    // `(2^64 - 1) * 3 + 5` to 2.
    outBuf[13] = int(u64v.x >> 32);
    // CHECK-NEXT: {{^}}1{{$}}
    outBuf[14] = int(uint(u64v.y));
    // CHECK-NEXT: {{^}}2{{$}}

    int2x2 i22 = mad(
        int2x2(minusSeven, int(two), int(three), 4),
        int2x2(six, int(five), -2, 1),
        int2x2(int(five), 0, 1, -10));
    outBuf[15] = i22[0][0];
    // CHECK-NEXT: {{^}}-37{{$}}
    outBuf[16] = i22[0][1];
    // CHECK-NEXT: {{^}}10{{$}}
    outBuf[17] = i22[1][0];
    // CHECK-NEXT: {{^}}-5{{$}}
    outBuf[18] = i22[1][1];
    // CHECK-NEXT: {{^}}-6{{$}}

    uint2x2 u22 = mad(
        uint2x2(allOnes, two, seven, allOnes),
        uint2x2(three, allOnes, 1, allOnes),
        uint2x2(five, ten, 0, 0));
    // One wrapped lane from each matrix row is read back, so both row-wise calls are checked:
    // `2 * 0xFFFFFFFF + 10` wraps to 8 and `0xFFFFFFFF * 0xFFFFFFFF` wraps to 1.
    outBuf[19] = int(u22[0][1]);
    // CHECK-NEXT: {{^}}8{{$}}
    outBuf[20] = int(u22[1][1]);
    // CHECK-NEXT: {{^}}1{{$}}

    // 0xFFFF * 3 + 5 wraps to 2 in 16 bits.
    outBuf[21] = int(mad(uint16_t(allOnes), uint16_t(three), uint16_t(five)));
    // CHECK-NEXT: {{^}}2{{$}}
    vector<int16_t, 2> i16v = mad(
        vector<int16_t, 2>(int16_t(minusSeven), int16_t(six)),
        vector<int16_t, 2>(int16_t(six), int16_t(-9)),
        vector<int16_t, 2>(int16_t(five), int16_t(ten)));
    outBuf[22] = int(i16v.x);
    // CHECK-NEXT: {{^}}-37{{$}}
    outBuf[23] = int(i16v.y);
    // CHECK-NEXT: {{^}}-44{{$}}

    // Float control: 3 * 0.5 + 0.25 = 1.75.
    outBuf[24] = int(mad(float(three), 0.5f, 0.25f) * 4.0f);
    // CHECK-NEXT: {{^}}7{{$}}
}
