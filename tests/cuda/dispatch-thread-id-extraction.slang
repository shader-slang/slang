//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK_NVVM2): -target ptx -emit-cuda-via-nvvm -entry computeMain2 -stage compute -capability cuda_sm_7_0 -o -
//TEST:SIMPLE(filecheck=CHECK_NVVM3): -target ptx -emit-cuda-via-nvvm -entry computeMain3 -stage compute -capability cuda_sm_7_0 -o -

// This test verifies that DispatchThreadID parameter of different types 
// correctly extracts components from the underlying uint3 value in CUDA.

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name cudaOutputBuffer
RWStructuredBuffer<float> cudaOutputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: uint {{.*}} = (blockIdx * blockDim + threadIdx).x;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain2(uint2 tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.y];
}
// The multi-component swizzle base is hoisted to a temporary (evaluated once)
// rather than re-emitted per component; see issue #12073.
// CHECK: uint3 [[T2:_S[0-9]+]] = blockIdx * blockDim + threadIdx;
// CHECK: uint2 {{.*}} = uint2 {[[T2]].x, [[T2]].y};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain3(int2 tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: uint3 [[T3:_S[0-9]+]] = blockIdx * blockDim + threadIdx;
// CHECK: uint2 {{.*}} = uint2 {[[T3]].x, [[T3]].y};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain4(int tid: SV_DispatchThreadID, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: int {{.*}} = int((blockIdx * blockDim + threadIdx).x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain5(int tid: SV_GroupIndex, StructuredBuffer<uint> src, RWStructuredBuffer<uint> dst)
{
    dst[tid.x] = src[tid.x];
}
// CHECK: int {{.*}} = int((threadIdx.z * blockDim.y + threadIdx.y) * blockDim.x + threadIdx.x);

// The direct libNVVM lanes prove that the ordinary uint2 swizzle and the uint2-to-int2 conversion
// reach accepted PTX. The unsigned entry uses both requested dimensions, while the optimizer
// removes the unused y lane from the signed entry.
// CHECK_NVVM2: .visible .entry computeMain2(
// CHECK_NVVM2-NEXT: {{[ \t]*}}.param .align 8 .b8 {{.*}}[16],
// CHECK_NVVM2-NEXT: {{[ \t]*}}.param .align 8 .b8 {{.*}}[16]
// CHECK_NVVM2-DAG: mov.u32 {{.*}}, %ctaid.y;
// CHECK_NVVM2-DAG: mov.u32 {{.*}}, %ctaid.x;
// CHECK_NVVM2-DAG: mov.u32 {{.*}}, %ntid.y;
// CHECK_NVVM2-DAG: mov.u32 {{.*}}, %ntid.x;
// CHECK_NVVM2-DAG: mov.u32 {{.*}}, %tid.y;
// CHECK_NVVM2-DAG: mov.u32 {{.*}}, %tid.x;
// CHECK_NVVM2: ld.global.nc.u32
// CHECK_NVVM2: st.global.u32

// CHECK_NVVM3: .visible .entry computeMain3(
// CHECK_NVVM3-NEXT: {{[ \t]*}}.param .align 8 .b8 {{.*}}[16],
// CHECK_NVVM3-NEXT: {{[ \t]*}}.param .align 8 .b8 {{.*}}[16]
// CHECK_NVVM3: mov.u32 {{.*}}, %ctaid.x;
// CHECK_NVVM3: mov.u32 {{.*}}, %ntid.x;
// CHECK_NVVM3: mov.u32 {{.*}}, %tid.x;
// CHECK_NVVM3: ld.global.nc.u32
// CHECK_NVVM3: st.global.u32
