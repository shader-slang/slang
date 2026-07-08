// Atomics on a legacy Uniform/BufferBlock SSBO (SPIR-V < 1.4); see issue #11731.
// SIMPLE pins the #11731 path; the runtime lane uses the default profile because the legacy
// `spirv_1_3` Vulkan lane emits an unsuppressible profile-upgrade warning under render-test.
//TEST:SIMPLE(filecheck=CHECK):-profile spirv_1_3 -target spirv
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST_INPUT: set buf = out ubuffer(data=[0 0 0 0], stride=4);
RWStructuredBuffer<Atomic<uint>> buf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: OpDecorate {{.*}} BufferBlock
    // CHECK: OpVariable {{.*}} Uniform
    // CHECK: OpAtomicLoad {{.*}} %uint_66
    // CHECK: OpAtomicStore {{.*}} %uint_68
    // CHECK: OpAtomicExchange {{.*}} %uint_72
    // CHECK: OpAtomicCompareExchange {{.*}} %uint_72 %uint_66
    let loaded = buf[0].load(MemoryOrder.Acquire);
    buf[0].store(loaded + 10u, MemoryOrder.Release);
    let prev = buf[0].exchange(100u, MemoryOrder.AcquireRelease);
    buf[0].compareExchange(100u, prev, MemoryOrder.AcquireRelease, MemoryOrder.Acquire);
    // BUF: 10
}
