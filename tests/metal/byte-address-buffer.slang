//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST(compute, metal):COMPARE_COMPUTE_EX:-metal -compute -output-using-type
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -output-using-type

// VK test disabled due to unhandled global inst
// TODO: github issue #8480

//TEST_INPUT:ubuffer(data=[0xfffefdfc 0xfbfaf9f8 0xf7f6f5f4 0xf3f2f1f0 0xefeeedec 0xebeae9e8 0xe7e6e5e4 0xe3e2e1e0 0xdfdedddc 0xdbdad9d8 0xd7d6d5d4 0xd3d2d1d0 0xcfcecdcc 0xcbcac9c8 0xc7c6c5c4 0xc3c2c1c0 0xbfbebdbc 0xbbbab9b8 0xb7b6b5b4 0xb3b2b1b0 0xafaeadac 0xabaaa9a8 0xa7a6a5a4 0xa3a2a1a0 0x9f9e9d9c 0x9b9a9998 0x97969594 0x93929190 0x8f8e8d8c 0x8b8a8988 0x87868584 0x83828180 0x7f7e7d7c 0x7b7a7978 0x77767574 0x73727170 0x6f6e6d6c 0x6b6a6968 0x67666564 0x63626160 0x5f5e5d5c 0x5b5a5958 0x57565554 0x53525150 0x4f4e4d4c 0x4b4a4948 0x47464544 0x43424140 0x3f3e3d3c 0x3b3a3938 0x37363534 0x33323130 0x2f2e2d2c 0x2b2a2928 0x27262524 0x23222120 0x1f1e1d1c 0x1b1a1918 0x17161514 0x13121110 0x0f0e0d0c 0x0b0a0908 0x07060504 0x03020100 0xfffefdfc 0xfbfaf9f8 0xf7f6f5f4 0xf3f2f1f0 0xefeeedec 0xebeae9e8 0xe7e6e5e4 0xe3e2e1e0 0xdfdedddc 0xdbdad9d8 0xd7d6d5d4]):out,name=buffer
RWByteAddressBuffer buffer;

// METALLIB: define void @computeMain

struct TestStruct
{
    uint8_t a;
    float16_t h;
    float b;
    float4 c;
    float4x3 d;
}

[numthreads(1,1,1)]
[shader("compute")]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // METAL: uint [[WORD0:[a-zA-Z0-9_]+]] = as_type<uint>({{.*}}[(0U)>>2]);
    // METAL: uchar [[A:[a-zA-Z0-9_]+]] = uchar(([[WORD0]] >> 0U) & 255U);
    // METAL: uint [[WORD1:[a-zA-Z0-9_]+]] = as_type<uint>({{.*}}[(0U)>>2]);
    // METAL: half [[H:[a-zA-Z0-9_]+]] = as_type<half>(ushort(([[WORD1]] >> 16U) & 65535U));

    // METAL: {{.*}}[(128U)>>2] = as_type<uint32_t>(({{.*}} & 4294967040U) | (uint([[A]]) << 0U));
    // METAL: {{.*}}[(128U)>>2] = as_type<uint32_t>(({{.*}} & 65535U) | (uint(as_type<ushort>([[H]])) << 16U));
    buffer.Store(128, buffer.Load<TestStruct>(0));

    // METAL: {{.*}}[(256U)>>2] = as_type<uint32_t>(4294967295U);
    // METAL: {{.*}}[(260U)>>2] = as_type<uint32_t>(4294967295U);
    int64_t i64 = -1;
    buffer.Store(256, i64);

    // METAL: {{.*}}[(264U)>>2] = as_type<uint32_t>(123U);
    // METAL: {{.*}}[(268U)>>2] = as_type<uint32_t>(0U);
    uint64_t u64 = 123;
    buffer.Store(264, u64);

    int64_t* ptr = Ptr<int64_t>(0xFF);
    // METAL: {{.*}}[(272U)>>2] = as_type<uint32_t>({{.*}});
    // METAL: {{.*}}[(276U)>>2] = as_type<uint32_t>({{.*}});
    buffer.Store(272, ptr);

    // METAL: {{.*}}[(280U)>>2] = as_type<uint32_t>(4294967295U);
    // METAL: {{.*}}[(284U)>>2] = as_type<uint32_t>(4294967295U);
    uintptr_t uintptr_val = (uintptr_t)-1;
    buffer.Store(280, uintptr_val);
}
