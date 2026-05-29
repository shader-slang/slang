//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly

// Regression coverage for #11130: explicit texture format descriptor types
// should preserve the storage image format through DescriptorHandle<T> when
// the resulting image is used for SPIR-V image atomics.

// CHECK-DAG: [[UINT:%[a-zA-Z0-9_]+]] = OpTypeInt 32 0
// CHECK-DAG: [[IMAGE_TYPE:%[a-zA-Z0-9_]+]] = OpTypeImage [[UINT]] 2D 2 0 0 2 R32ui
// CHECK-DAG: [[IMAGE_HANDLE_PTR:%[a-zA-Z0-9_]+]] = OpTypePointer UniformConstant [[IMAGE_TYPE]]
// CHECK-DAG: [[IMAGE_PTR:%[a-zA-Z0-9_]+]] = OpTypePointer Image [[UINT]]
// CHECK: [[IMAGE:%[a-zA-Z0-9_]+]] = OpAccessChain [[IMAGE_HANDLE_PTR]]
// CHECK: [[PTR:%[a-zA-Z0-9_]+]] = OpImageTexelPointer [[IMAGE_PTR]] [[IMAGE]]
// CHECK: OpAtomicIAdd [[UINT]] [[PTR]]

struct Params
{
    DescriptorHandle<RWTexture2D<R32UI>> textureHandle;
};

ConstantBuffer<Params> params;

[noinline]
void atomicAddValue(DescriptorHandle<RWTexture2D<R32UI>> handle, uint2 pixel, uint value)
{
    RWTexture2D<R32UI> texture = handle;
    InterlockedAdd(texture[pixel], value);
}

[noinline]
void forwardAtomicHandle(DescriptorHandle<RWTexture2D<R32UI>> handle, uint2 pixel)
{
    atomicAddValue(handle, pixel, 1);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    DescriptorHandle<RWTexture2D<R32UI>> handle = params.textureHandle;
    forwardAtomicHandle(handle, dispatchThreadID.xy);
}
