//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry computeMain -stage compute

// Positive test: these storage texture configurations should compile without error.
// 1. r32float with read_write access (always valid in WGSL)
// 2. rgba16float with write-only access (valid even though read_write is not allowed)
// GitHub issue: https://github.com/shader-slang/slang/issues/7164

// CHECK: texture_storage_2d<r32float, read_write>
[[vk::image_format("r32f")]]
RWTexture2D<float> rwTexture;

// CHECK: texture_storage_2d<rgba16float, write>
[[vk::image_format("rgba16f")]]
WTexture2D<float4> writeOnlyTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    rwTexture[threadId.xy] = 1.0;
    writeOnlyTexture.Store(int2(threadId.xy), float4(1, 0, 0, 1));
}
