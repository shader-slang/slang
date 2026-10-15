//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -line-directive-mode none

// Regression coverage for explicit texture format descriptor types inside
// DescriptorHandle<T>. The handle is loaded from ordinary constant-buffer data
// and passed across function boundaries before being converted to the texture.

// CHECK: [[IMAGE_TYPE:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 2 0 0 2 Rgba16f
// CHECK: [[IMAGE:%[a-zA-Z0-9_]+]] = OpLoad [[IMAGE_TYPE]]
// CHECK: OpImageWrite [[IMAGE]]

struct Params
{
    DescriptorHandle<RWTexture2D<RGBA16F>> textureHandle;
};

ConstantBuffer<Params> params;

[noinline]
void storeValue(DescriptorHandle<RWTexture2D<RGBA16F>> handle, uint2 pixel, float4 value)
{
    RWTexture2D<RGBA16F> texture = handle;
    texture[pixel] = value;
}

[noinline]
void forwardHandle(DescriptorHandle<RWTexture2D<RGBA16F>> handle, uint2 pixel)
{
    storeValue(handle, pixel, float4(1.0f, 0.0f, 0.0f, 1.0f));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    DescriptorHandle<RWTexture2D<RGBA16F>> handle = params.textureHandle;
    forwardHandle(handle, dispatchThreadID.xy);
}
