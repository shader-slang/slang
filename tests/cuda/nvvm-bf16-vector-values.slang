//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Records contain raw BF16 bits, Float32 bits, and independent narrowing expectations.
// They include both signs of zero, subnormals, boundary ties, overflow, infinities and NaNs.
// Expected 65535 denotes NaN classification, not a universal payload promise.
//TEST_INPUT:ubuffer(data=[0 0 0 32768 2147483648 32768 1 65536 1 127 8355840 128 128 8388608 128 16256 1065385984 16256 16257 1065451520 16258 49024 3212869633 49025 32639 2139062272 32640 65408 4286578688 65408 32640 2139095040 32640 32705 2143363909 65535 32641 2139095041 65535 32769 2147516416 32768 16258 1065451519 16257 16384 1065385985 16257], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
vector<BFloat16, 2> choose2(vector<BFloat16, 2> a, vector<BFloat16, 2> b, bool flag)
{
    if (flag)
        return a;
    return b;
}

[noinline]
vector<BFloat16, 3> choose3(vector<BFloat16, 3> a, vector<BFloat16, 3> b, bool flag)
{
    if (flag)
        return a;
    return b;
}

[noinline]
vector<BFloat16, 4> choose4(vector<BFloat16, 4> a, vector<BFloat16, 4> b, bool flag)
{
    if (flag)
        return a;
    return b;
}

// Each width uses different lanes from the independent scalar boundary records.
// All BF16 vectors are values; no inout helper or addressable vector storage is used.
[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    bool ok = true;
    bool flag = (i & 1) != 0;
    uint r0 = (i + 0) % 16;
    uint b0 = inputs[3 * r0];
    float f0 = asfloat(inputs[3 * r0 + 1]);
    BFloat16 v0 = bit_cast<BFloat16>(uint16_t(b0));
    uint r1 = (i + 5) % 16;
    uint b1 = inputs[3 * r1];
    float f1 = asfloat(inputs[3 * r1 + 1]);
    BFloat16 v1 = bit_cast<BFloat16>(uint16_t(b1));
    uint r2 = (i + 10) % 16;
    uint b2 = inputs[3 * r2];
    float f2 = asfloat(inputs[3 * r2 + 1]);
    BFloat16 v2 = bit_cast<BFloat16>(uint16_t(b2));
    uint r3 = (i + 15) % 16;
    uint b3 = inputs[3 * r3];
    float f3 = asfloat(inputs[3 * r3 + 1]);
    BFloat16 v3 = bit_cast<BFloat16>(uint16_t(b3));
    let made2 = vector<BFloat16, 2>(v0, v1);
    let splat2 = vector<BFloat16, 2>(v0);
    let selected2 = choose2(made2, splat2, flag);
    let expanded2 = vector<float, 2>(made2);
    let narrowed2 = vector<BFloat16, 2>(vector<float, 2>(f0, f1));
    let transported2 = bit_cast<vector<BFloat16, 2>>(b0 | (b1 << 16));
    ok = ok && bit_cast<uint>(transported2) == (b0 | (b1 << 16));
    uint narrow2x = uint(bit_cast<uint16_t>(narrowed2.x));
    uint expected2x = inputs[3 * r0 + 2];
    ok = ok && (expected2x == 65535 ?
        ((narrow2x & 0x7f80) == 0x7f80 && (narrow2x & 0x7f) != 0) :
        narrow2x == expected2x);
    ok = ok && asuint(expanded2.x) == (b0 << 16) &&
        uint(bit_cast<uint16_t>(splat2.x)) == b0 &&
        uint(bit_cast<uint16_t>(selected2.x)) == (flag ? b0 : b0) &&
        uint(bit_cast<uint16_t>(transported2.x)) == b0;
    uint narrow2y = uint(bit_cast<uint16_t>(narrowed2.y));
    uint expected2y = inputs[3 * r1 + 2];
    ok = ok && (expected2y == 65535 ?
        ((narrow2y & 0x7f80) == 0x7f80 && (narrow2y & 0x7f) != 0) :
        narrow2y == expected2y);
    ok = ok && asuint(expanded2.y) == (b1 << 16) &&
        uint(bit_cast<uint16_t>(splat2.y)) == b0 &&
        uint(bit_cast<uint16_t>(selected2.y)) == (flag ? b1 : b0) &&
        uint(bit_cast<uint16_t>(transported2.y)) == b1;
    let made3 = vector<BFloat16, 3>(v0, v1, v2);
    let splat3 = vector<BFloat16, 3>(v0);
    let selected3 = choose3(made3, splat3, flag);
    let expanded3 = vector<float, 3>(made3);
    let narrowed3 = vector<BFloat16, 3>(vector<float, 3>(f0, f1, f2));
    let transported3 = bit_cast<vector<BFloat16, 3>>(uint16_t3(uint16_t(b0), uint16_t(b1), uint16_t(b2)));
    let packed3 = bit_cast<uint16_t3>(transported3);
    ok = ok && uint(packed3.x) == b0 && uint(packed3.y) == b1 && uint(packed3.z) == b2;
    uint narrow3x = uint(bit_cast<uint16_t>(narrowed3.x));
    uint expected3x = inputs[3 * r0 + 2];
    ok = ok && (expected3x == 65535 ?
        ((narrow3x & 0x7f80) == 0x7f80 && (narrow3x & 0x7f) != 0) :
        narrow3x == expected3x);
    ok = ok && asuint(expanded3.x) == (b0 << 16) &&
        uint(bit_cast<uint16_t>(splat3.x)) == b0 &&
        uint(bit_cast<uint16_t>(selected3.x)) == (flag ? b0 : b0) &&
        uint(bit_cast<uint16_t>(transported3.x)) == b0;
    uint narrow3y = uint(bit_cast<uint16_t>(narrowed3.y));
    uint expected3y = inputs[3 * r1 + 2];
    ok = ok && (expected3y == 65535 ?
        ((narrow3y & 0x7f80) == 0x7f80 && (narrow3y & 0x7f) != 0) :
        narrow3y == expected3y);
    ok = ok && asuint(expanded3.y) == (b1 << 16) &&
        uint(bit_cast<uint16_t>(splat3.y)) == b0 &&
        uint(bit_cast<uint16_t>(selected3.y)) == (flag ? b1 : b0) &&
        uint(bit_cast<uint16_t>(transported3.y)) == b1;
    uint narrow3z = uint(bit_cast<uint16_t>(narrowed3.z));
    uint expected3z = inputs[3 * r2 + 2];
    ok = ok && (expected3z == 65535 ?
        ((narrow3z & 0x7f80) == 0x7f80 && (narrow3z & 0x7f) != 0) :
        narrow3z == expected3z);
    ok = ok && asuint(expanded3.z) == (b2 << 16) &&
        uint(bit_cast<uint16_t>(splat3.z)) == b0 &&
        uint(bit_cast<uint16_t>(selected3.z)) == (flag ? b2 : b0) &&
        uint(bit_cast<uint16_t>(transported3.z)) == b2;
    let made4 = vector<BFloat16, 4>(v0, v1, v2, v3);
    let splat4 = vector<BFloat16, 4>(v0);
    let selected4 = choose4(made4, splat4, flag);
    let expanded4 = vector<float, 4>(made4);
    let narrowed4 = vector<BFloat16, 4>(vector<float, 4>(f0, f1, f2, f3));
    uint64_t packed4 = uint64_t(b0) | (uint64_t(b1) << 16) | (uint64_t(b2) << 32) | (uint64_t(b3) << 48);
    let transported4 = bit_cast<vector<BFloat16, 4>>(packed4);
    ok = ok && bit_cast<uint64_t>(transported4) == packed4;
    uint narrow4x = uint(bit_cast<uint16_t>(narrowed4.x));
    uint expected4x = inputs[3 * r0 + 2];
    ok = ok && (expected4x == 65535 ?
        ((narrow4x & 0x7f80) == 0x7f80 && (narrow4x & 0x7f) != 0) :
        narrow4x == expected4x);
    ok = ok && asuint(expanded4.x) == (b0 << 16) &&
        uint(bit_cast<uint16_t>(splat4.x)) == b0 &&
        uint(bit_cast<uint16_t>(selected4.x)) == (flag ? b0 : b0) &&
        uint(bit_cast<uint16_t>(transported4.x)) == b0;
    uint narrow4y = uint(bit_cast<uint16_t>(narrowed4.y));
    uint expected4y = inputs[3 * r1 + 2];
    ok = ok && (expected4y == 65535 ?
        ((narrow4y & 0x7f80) == 0x7f80 && (narrow4y & 0x7f) != 0) :
        narrow4y == expected4y);
    ok = ok && asuint(expanded4.y) == (b1 << 16) &&
        uint(bit_cast<uint16_t>(splat4.y)) == b0 &&
        uint(bit_cast<uint16_t>(selected4.y)) == (flag ? b1 : b0) &&
        uint(bit_cast<uint16_t>(transported4.y)) == b1;
    uint narrow4z = uint(bit_cast<uint16_t>(narrowed4.z));
    uint expected4z = inputs[3 * r2 + 2];
    ok = ok && (expected4z == 65535 ?
        ((narrow4z & 0x7f80) == 0x7f80 && (narrow4z & 0x7f) != 0) :
        narrow4z == expected4z);
    ok = ok && asuint(expanded4.z) == (b2 << 16) &&
        uint(bit_cast<uint16_t>(splat4.z)) == b0 &&
        uint(bit_cast<uint16_t>(selected4.z)) == (flag ? b2 : b0) &&
        uint(bit_cast<uint16_t>(transported4.z)) == b2;
    uint narrow4w = uint(bit_cast<uint16_t>(narrowed4.w));
    uint expected4w = inputs[3 * r3 + 2];
    ok = ok && (expected4w == 65535 ?
        ((narrow4w & 0x7f80) == 0x7f80 && (narrow4w & 0x7f) != 0) :
        narrow4w == expected4w);
    ok = ok && asuint(expanded4.w) == (b3 << 16) &&
        uint(bit_cast<uint16_t>(splat4.w)) == b0 &&
        uint(bit_cast<uint16_t>(selected4.w)) == (flag ? b3 : b0) &&
        uint(bit_cast<uint16_t>(transported4.w)) == b3;
    // The existing constructor/extraction paths also preserve mixed operands, swizzles
    // and runtime indices. Check them independently before reporting the combined result.
    let mixed3 = vector<BFloat16, 3>(made2, v2);
    let mixed4 = vector<BFloat16, 4>(made2, vector<BFloat16, 2>(v2, v3));
    let reversed4 = mixed4.wzyx;
    uint dynamicLane = i % 4;
    uint dynamicExpected = inputs[3 * ((i + dynamicLane * 5) % 16)];
    ok = ok && uint(bit_cast<uint16_t>(mixed3.z)) == b2 &&
        uint(bit_cast<uint16_t>(mixed4[dynamicLane])) == dynamicExpected &&
        uint(bit_cast<uint16_t>(reversed4.x)) == b3 &&
        uint(bit_cast<uint16_t>(reversed4.y)) == b2 &&
        uint(bit_cast<uint16_t>(reversed4.z)) == b1 &&
        uint(bit_cast<uint16_t>(reversed4.w)) == b0;
    outputBuffer[i] = uint(ok);
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
