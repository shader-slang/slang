//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib

RWByteAddressBuffer buffer;

// CHECK-ASM: define void @main_kernel

struct TestStruct
{
    uint8_t a;
    float16_t h;
    float b;
    float4 c;
    float4x3 d;
}

[numthreads(1, 1, 1)]
void main_kernel(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: uint [[WORD0:[a-zA-Z0-9_]+]] = as_type<uint>({{.*}}[(0U)>>2]);
    // CHECK: uint8_t [[A:[a-zA-Z0-9_]+]] = uint8_t(([[WORD0]] >> 0U) & 255U);
    // CHECK: uint [[WORD1:[a-zA-Z0-9_]+]] = as_type<uint>({{.*}}[(0U)>>2]);
    // CHECK: half [[H:[a-zA-Z0-9_]+]] = as_type<half>(ushort(([[WORD1]] >> 16U) & 65535U));

    // CHECK: {{.*}}[(128U)>>2] = as_type<uint32_t>(({{.*}} & 4294967040U) | (uint([[A]]) << 0U));
    // CHECK: {{.*}}[(128U)>>2] = as_type<uint32_t>(({{.*}} & 65535U) | (uint(as_type<ushort>([[H]])) << 16U));
    buffer.Store(128, buffer.Load<TestStruct>(0));

    // CHECK: {{.*}}[(256U)>>2] = as_type<uint32_t>(4294967295U);
    // CHECK: {{.*}}[(260U)>>2] = as_type<uint32_t>(4294967295U);
    int64_t i64 = -1;
    buffer.Store(256, i64);

    // CHECK: {{.*}}[(264U)>>2] = as_type<uint32_t>(123U);
    // CHECK: {{.*}}[(268U)>>2] = as_type<uint32_t>(0U);
    uint64_t u64 = 123;
    buffer.Store(264, u64);

    int64_t* ptr = Ptr<int64_t>(0xFF);
    // CHECK: {{.*}}[(272U)>>2] = as_type<uint32_t>({{.*}});
    // CHECK: {{.*}}[(276U)>>2] = as_type<uint32_t>({{.*}});
    buffer.Store(272, ptr);

    // CHECK: {{.*}}[(280U)>>2] = as_type<uint32_t>(4294967295U);
    // CHECK: {{.*}}[(284U)>>2] = as_type<uint32_t>(4294967295U);
    uintptr_t uintptr_val = (uintptr_t)-1;
    buffer.Store(280, uintptr_val);
}
