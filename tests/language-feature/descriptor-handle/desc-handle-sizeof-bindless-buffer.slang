//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability spvBindlessTextureNV -stage compute -entry computeMain

// Companion to desc-handle-sizeof-bindless.slang. Under `spvBindlessTextureNV` only the
// texture/sampler-family kinds use the `uint64` representation (size 8, align 8, verified there);
// the kinds the extension cannot encode keep the `uint2` representation (size 8, align 4). This
// checks that `sizeof`/`alignof` of a buffer / acceleration-structure handle report the `uint2`
// layout, i.e. the representation is kind-dependent rather than capability-wide. See
// shader-slang/slang#12185.

struct Data { float v; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // uint2 kinds: size 8 ...
    outputBuffer[0] = sizeof(DescriptorHandle<ConstantBuffer<Data>>);
    outputBuffer[1] = sizeof(DescriptorHandle<RWStructuredBuffer<float>>);
    outputBuffer[2] = sizeof(DescriptorHandle<RaytracingAccelerationStructure>);

    // ... but 4-byte aligned (a uint2, not a uint64).
    outputBuffer[3] = alignof(DescriptorHandle<ConstantBuffer<Data>>);
    outputBuffer[4] = alignof(DescriptorHandle<RWStructuredBuffer<float>>);
    outputBuffer[5] = alignof(DescriptorHandle<RaytracingAccelerationStructure>);
}

// CHECK-DAG: %int_8 = OpConstant %int 8
// CHECK-DAG: %int_4 = OpConstant %int 4

// The three sizeof results are 8 ...
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
// CHECK: OpStore {{%[0-9]+}} %int_8
// ... and the three alignof results are 4 (uint2 alignment, not uint64's 8).
// CHECK: OpStore {{%[0-9]+}} %int_4
// CHECK: OpStore {{%[0-9]+}} %int_4
// CHECK: OpStore {{%[0-9]+}} %int_4
