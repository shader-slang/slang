// byte-address-buffer-array.slang
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -d3d12 -profile cs_6_0 -use-dxil -shaderobj -output-using-type
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -output-using-type

//TEST:SIMPLE(filecheck=CHECK1):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK2):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK3):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK3):-target spirv -emit-spirv-directly -entry computeMain -stage compute

// Confirm compilation of `(RW)ByteAddressBuffer` with aligned load / stores to wider data types.

[vk::binding(2, 3)] RWByteAddressBuffer buffer0;
struct BoundingBoxBlock {
    float qBox[4];
};
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK-NOT: warning
    // CHECK1: _Array_std430_float4_0 packStorage_0(float {{.*}}[4])
    // CHECK1: void unpackStorage_0(_Array_std430_float4_0 {{.*}}, out float {{.*}}[4])
    // CHECK1: unpackStorage_0(buffer0_0._data[0], {{.*}});
    // CHECK1: float {{.*}}[4] = buffer0_0._data[0] = packStorage_0({{.*}});

    // CHECK2: float  {{.*}}[int(4)] = (buffer0_0).Load<float [int(4)] >(int(0));
    // CHECK2: buffer0_0.Store(int(0),{{.*}});

    // CHECK3-DAG: %[[AI4:[a-zA-Z0-9_]+]] = OpTypeArray %float %int_4
    // CHECK3-DAG: %[[ASF4:[a-zA-Z0-9_]+]] = OpTypeStruct %[[AI4]]
    // CHECK3-DAG: %[[SBA:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %[[ASF4]]
    // CHECK3-DAG: %[[V1:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBA]] %buffer0 %int_0 %int_0
    // CHECK3-DAG: %[[V2:[a-zA-Z0-9_]+]] = OpLoad %[[ASF4]] %[[V1]]
    // CHECK3-DAG: %[[V3:[a-zA-Z0-9_]+]] = OpCompositeExtract %[[AI4]] %[[V2]] 0
    // CHECK3-DAG: %[[V4:[a-zA-Z0-9_]+]] = OpCompositeExtract %float %[[V3]] 0
    // CHECK3-DAG: %[[V5:[a-zA-Z0-9_]+]] = OpCompositeExtract %float %[[V3]] 1
    // CHECK3-DAG: %[[V6:[a-zA-Z0-9_]+]] = OpCompositeExtract %float %[[V3]] 2
    // CHECK3-DAG: %[[V7:[a-zA-Z0-9_]+]] = OpCompositeExtract %float %[[V3]] 3
    // CHECK3-DAG: %[[V8:[a-zA-Z0-9_]+]] = OpCompositeConstruct %[[AI4]] %[[V4]] %[[V5]] %[[V6]] %[[V7]]
    // CHECK3-DAG: %[[V9:[a-zA-Z0-9_]+]] = OpCompositeConstruct %[[ASF4]] %[[V8]]
    // CHECK3-DAG: %[[V10:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBA]] %buffer0 %int_0 %int_0
    // CHECK3-DAG: OpStore %[[V10]] %[[V9]]
    buffer0.StoreAligned(0, buffer0.LoadAligned<BoundingBoxBlock>(0));
}

