//TEST:SIMPLE(filecheck=CHECK): -entry computeMain -stage compute -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -entry computeMain -stage compute -target spirv
//TEST:SIMPLE(filecheck=CHECK-CUDA): -entry computeMain -stage compute -target cuda

layout(rgba8)
RWTexture2D<float4> texture;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    half h = 1.0h;
    // CHECK: OpImageRead
    // CHECK: OpCompositeInsert
    // CHECK: OpImageWrite
    // On CUDA the write needs a converting read from `rgba8`, which the CUDA prelude does not do
    // correctly.
    // CHECK-CUDA: error[E56007]
    // CHECK-CUDA: lowered to a CUDA surface read that converts
    texture[dispatchThreadID.xy].a = h;
}
