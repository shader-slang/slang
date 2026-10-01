//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Four invocations write distinct slots. The barrier orders mirrored reads; coherence alone
// does not synchronize invocations. Ordinary reads independently check the coherent stores.
// Each allocation has a guard on both sides, including both words of each 64-bit guard.
//TEST_INPUT:ubuffer(data=[305419896 0 0 0 0 305419896],stride=4):name=globalI32
uniform int* globalI32;
//TEST_INPUT:ubuffer(data=[305419896 0 0 0 0 305419896],stride=4):name=globalU32
uniform uint* globalU32;
//TEST_INPUT:ubuffer(data=[305419896 305419896 0 0 0 0 0 0 0 0 305419896 305419896],stride=8):name=globalI64
uniform int64_t* globalI64;
//TEST_INPUT:ubuffer(data=[305419896 305419896 0 0 0 0 0 0 0 0 305419896 305419896],stride=8):name=globalU64
uniform uint64_t* globalU64;
//TEST_INPUT:ubuffer(data=[324508639 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

groupshared int sharedI32[6];
groupshared uint sharedU32[6];
groupshared int64_t sharedI64[6];
groupshared uint64_t sharedU64[6];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_GroupThreadID)
{
    uint lane = tid.x;
    int i32 = -2147483600 + int(lane);
    uint u32 = 0xfedcba90 + lane;
    int64_t i64 = -int64_t(4886718345) - int64_t(lane);
    uint64_t u64 = (uint64_t(0xfedcba98) << 32) | uint64_t(0x76543210 + lane);
    uint64_t guard64 = (uint64_t(0x12345678) << 32) | uint64_t(0x12345678);
    if (lane == 0)
    {
        sharedI32[0] = sharedI32[5] = 0x12345678;
        sharedU32[0] = sharedU32[5] = 0x12345678;
        sharedI64[0] = sharedI64[5] = int64_t(guard64);
        sharedU64[0] = sharedU64[5] = guard64;
    }
    storeCoherent<4, MemoryScope::Device>(globalI32 + lane + 1, i32);
    storeCoherent<4, MemoryScope::Device>(globalU32 + lane + 1, u32);
    storeCoherent<8, MemoryScope::Device>(globalI64 + lane + 1, i64);
    storeCoherent<8, MemoryScope::Device>(globalU64 + lane + 1, u64);
    storeCoherent<4, MemoryScope::Workgroup>(__getAddress(sharedI32[lane + 1]), i32);
    storeCoherent<4, MemoryScope::Workgroup>(__getAddress(sharedU32[lane + 1]), u32);
    storeCoherent<8, MemoryScope::Workgroup>(__getAddress(sharedI64[lane + 1]), i64);
    storeCoherent<8, MemoryScope::Workgroup>(__getAddress(sharedU64[lane + 1]), u64);
    AllMemoryBarrierWithGroupSync();

    uint mirror = 3 - lane;
    int expectedI32 = -2147483600 + int(mirror);
    uint expectedU32 = 0xfedcba90 + mirror;
    int64_t expectedI64 = -int64_t(4886718345) - int64_t(mirror);
    uint64_t expectedU64 = (uint64_t(0xfedcba98) << 32) | uint64_t(0x76543210 + mirror);
    uint slot = mirror + 1;
    int readI32 = loadCoherent<4, MemoryScope::Device>(globalI32 + slot);
    uint readU32 = loadCoherent<4, MemoryScope::Device>(globalU32 + slot);
    int64_t readI64 = loadCoherent<8, MemoryScope::Device>(globalI64 + slot);
    uint64_t readU64 = loadCoherent<8, MemoryScope::Device>(globalU64 + slot);
    uint bad = uint(loadCoherent<4, MemoryScope::Workgroup>(__getAddress(sharedI32[slot])) != expectedI32);
    bad |= uint(loadCoherent<4, MemoryScope::Workgroup>(__getAddress(sharedU32[slot])) != expectedU32) << 1;
    bad |= uint(loadCoherent<8, MemoryScope::Workgroup>(__getAddress(sharedI64[slot])) != expectedI64) << 2;
    bad |= uint(loadCoherent<8, MemoryScope::Workgroup>(__getAddress(sharedU64[slot])) != expectedU64) << 3;
    bad |= uint(globalI32[slot] != expectedI32 || globalU32[slot] != expectedU32
        || globalI64[slot] != expectedI64 || globalU64[slot] != expectedU64) << 4;
    bad |= uint(sharedI32[slot] != expectedI32 || sharedU32[slot] != expectedU32
        || sharedI64[slot] != expectedI64 || sharedU64[slot] != expectedU64) << 5;
    bad |= uint(globalI32[0] != 0x12345678 || globalI32[5] != 0x12345678
        || globalU32[0] != 0x12345678 || globalU32[5] != 0x12345678
        || uint64_t(globalI64[0]) != guard64 || uint64_t(globalI64[5]) != guard64
        || globalU64[0] != guard64 || globalU64[5] != guard64) << 6;
    bad |= uint(sharedI32[0] != 0x12345678 || sharedI32[5] != 0x12345678
        || sharedU32[0] != 0x12345678 || sharedU32[5] != 0x12345678
        || uint64_t(sharedI64[0]) != guard64 || uint64_t(sharedI64[5]) != guard64
        || sharedU64[0] != guard64 || sharedU64[5] != guard64) << 7;
    uint base = 1 + lane * 8;
    output[base] = uint(readI32);
    output[base + 1] = readU32;
    output[base + 2] = uint(uint64_t(readI64));
    output[base + 3] = uint(uint64_t(readI64) >> 32);
    output[base + 4] = uint(readU64);
    output[base + 5] = uint(readU64 >> 32);
    output[base + 6] = bad;
    output[base + 7] = 60070 + lane;
}

// CHECK: 324508639
// CHECK-NEXT: 2147483699
// CHECK-NEXT: 4275878547
// CHECK-NEXT: 3703216244
// CHECK-NEXT: 4294967294
// CHECK-NEXT: 1985229331
// CHECK-NEXT: 4275878552
// CHECK-NEXT: 0
// CHECK-NEXT: 60070
// CHECK-NEXT: 2147483698
// CHECK-NEXT: 4275878546
// CHECK-NEXT: 3703216245
// CHECK-NEXT: 4294967294
// CHECK-NEXT: 1985229330
// CHECK-NEXT: 4275878552
// CHECK-NEXT: 0
// CHECK-NEXT: 60071
// CHECK-NEXT: 2147483697
// CHECK-NEXT: 4275878545
// CHECK-NEXT: 3703216246
// CHECK-NEXT: 4294967294
// CHECK-NEXT: 1985229329
// CHECK-NEXT: 4275878552
// CHECK-NEXT: 0
// CHECK-NEXT: 60072
// CHECK-NEXT: 2147483696
// CHECK-NEXT: 4275878544
// CHECK-NEXT: 3703216247
// CHECK-NEXT: 4294967294
// CHECK-NEXT: 1985229328
// CHECK-NEXT: 4275878552
// CHECK-NEXT: 0
// CHECK-NEXT: 60073
// CHECK-NEXT: 3735928559
