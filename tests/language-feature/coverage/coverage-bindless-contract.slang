// Pins three parts of the bindless contract that the other tests leave
// unverified, each of which would fail silently rather than loudly.
//
// 1. Index 0 selects the array form. The feature keys on `bindlessIndex >= 0`;
//    narrowing that to `> 0` would drop element 0 back to the single-buffer
//    form -- one binding for that shader instead of the shared array, which is
//    the exact outcome the mode exists to prevent. Every other test uses index
//    7, so none of them would notice.
//
// 2. The flag implies `-trace-coverage`. The CLI parser sets it, and the help
//    text promises it, but every other test also passes `-trace-coverage`
//    explicitly, so removing that implication would break no test.
//
// 3. The array shape survives auto-allocation. Every other test pins placement
//    with `-trace-coverage-binding`; the documented "leave it to
//    auto-allocation" path had no emit-level coverage.

//TEST:SIMPLE(filecheck=ZERO):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-binding 0 31 -trace-coverage-bindless-index 0
//TEST:SIMPLE(filecheck=IMPLIED):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage-bindless-index 7
//TEST:SIMPLE(filecheck=AUTO):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-bindless-index 7

//TEST_INPUT: set outBuf = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a = (float)tid.x;
    outBuf[0] = a * 2.0f;
}

// 1. Element 0 is still the array form, not a bare buffer.
//ZERO-DAG: OpCapability RuntimeDescriptorArray
//ZERO-DAG: OpAccessChain {{.*}} %__slang_coverage %int_0

// 2. The bindless flag alone instruments: no `-trace-coverage` was passed.
//IMPLIED-DAG: OpCapability RuntimeDescriptorArray
//IMPLIED-DAG: OpAccessChain {{.*}} %__slang_coverage %int_7

// 3. Auto-allocated placement keeps the unbounded array and the two-level
//    access; only where it lands is left to the allocator.
//AUTO-DAG: OpCapability RuntimeDescriptorArray
//AUTO-DAG: OpDecorate %__slang_coverage DescriptorSet
//AUTO-DAG: OpAccessChain {{.*}} %__slang_coverage %int_7
