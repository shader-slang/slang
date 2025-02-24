// byte-address-buffer-array.slang
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -d3d12 -profile cs_6_0 -use-dxil -shaderobj -output-using-type
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -output-using-type

//TEST:SIMPLE(filecheck=CHECK1):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK2):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK3):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK3):-target spirv -emit-spirv-directly -entry computeMain -stage compute

// Confirm compilation of `(RW)ByteAddressBuffer` with aligned load / stores to wider data types.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=buffer
[vk::binding(2, 3)] RWByteAddressBuffer buffer;
struct Block {
    float4 val[2];
};
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK-NOT: warning
    // CHECK1: _Array_std430_vector{{.*}} _data[]
    // CHECK1: _Array_std430_vector{{.*}} packStorage_0(vec4 {{.*}}[2])
    // CHECK1: vec4  {{.*}}[2] = { {{.*}}[0], {{.*}}[1] };
    // CHECK1: _Array_std430_vector{{.*}} {{.*}} = { {{.*}} };
    // CHECK1: void unpackStorage_0(_Array_std430_vector{{.*}}, out vec4 {{.*}}[2])
    // CHECK1: {{.*}}[0] =  {{.*}}.data_0[0];
    // CHECK1: {{.*}}[1] =  {{.*}}.data_0[1];
    // CHECK1: vec4  {{.*}}[2];
    // CHECK1: unpackStorage_0(buffer_0._data[uint(0)], {{.*}});
    // CHECK1: vec4  {{.*}}[2] = buffer_0._data[0] = packStorage_0({{.*}});
    // CHECK1: vec4 {{.*}} = buffer_2._data[uint(1)];
    // CHECK1: vec4 {{.*}} = buffer_2._data[1] = vec4(buffer_1._data[uint(1)], buffer_1._data[uint(2)], buffer_1._data[uint(3)], buffer_1._data[uint(4)]);

    // CHECK2: float4 {{.*}}[int(2)] = (buffer_0).Load<float4 [int(2)] >(int(0));
    // CHECK2: buffer_0.Store(int(0),{{.*}});
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(int(4));
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(int(8));
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(int(12));
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(int(16));
    // CHECK2: float4 {{.*}} = float4({{.*}}, {{.*}}, {{.*}}, {{.*}});
    // CHECK2: float4 {{.*}} = (buffer_0).Load<float4 >(int(20));
    // CHECK2: buffer_0.Store(int(16),{{.*}});
    // CHECK2: buffer_0.Store(int(32),{{.*}});

    // CHECK3-DAG: %[[ARRV4_2:[a-zA-Z0-9_]+]] = OpTypeArray %v4float %int_2
    // CHECK3-DAG: %[[SARRV4_2:[a-zA-Z0-9_]+]] = OpTypeStruct %[[ARRV4_2]]
    // CHECK3-DAG: %[[SBA:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %[[SARRV4_2]]
    // CHECK3-DAG: %[[SBF:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %float
    // CHECK3-DAG: %[[SBVF:[a-zA-Z0-9_]+]] = OpTypePointer StorageBuffer %v4float
    // CHECK3-DAG: %[[V0:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBA]] %buffer {{.*}}
    // CHECK3-DAG: %[[V1:[a-zA-Z0-9_]+]] = OpLoad %[[SARRV4_2]] %[[V0]]
    // CHECK3-DAG: %[[V2:[a-zA-Z0-9_]+]] = OpCompositeExtract %[[ARRV4_2]] %[[V1]] 0
    // CHECK3-DAG: %[[V3:[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float %[[V2]] 0
    // CHECK3-DAG: %[[V4:[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float %[[V2]] 1
    // CHECK3-DAG: %[[V5:[a-zA-Z0-9_]+]] = OpCompositeConstruct %[[ARRV4_2]] %[[V3]] %[[V4]]
    // CHECK3-DAG: %[[V6:[a-zA-Z0-9_]+]] = OpCompositeConstruct %[[SARRV4_2]] %[[V5]]
    // CHECK3-DAG: %[[V7:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBA]] %buffer {{.*}}
    // CHECK3-DAG: OpStore %[[V7]] %[[V6]]
    // CHECK3-DAG: %[[V8:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBF]] %buffer_0 {{.*}}
    // CHECK3-DAG: %[[V9:[a-zA-Z0-9_]+]] = OpLoad %float %[[V8]]
    // CHECK3-DAG: %[[V10:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBF]] %buffer_0 {{.*}}
    // CHECK3-DAG: %[[V11:[a-zA-Z0-9_]+]] = OpLoad %float %[[V10]]
    // CHECK3-DAG: %[[V12:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBF]] %buffer_0 {{.*}}
    // CHECK3-DAG: %[[V13:[a-zA-Z0-9_]+]] = OpLoad %float %[[V12]]
    // CHECK3-DAG: %[[V14:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBF]] %buffer_0 {{.*}}
    // CHECK3-DAG: %[[V15:[a-zA-Z0-9_]+]] = OpLoad %float %[[V14]]
    // CHECK3-DAG: %[[V16:[a-zA-Z0-9_]+]] = OpCompositeConstruct %v4float %[[V9]] %[[V11]] %[[V13]] %[[V15]]
    // CHECK3-DAG: %[[V17:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBVF]] %buffer_1 {{.*}}
    // CHECK3-DAG: %[[V18:[a-zA-Z0-9_]+]] = OpLoad %v4float %[[V17]]
    // CHECK3-DAG: %[[V19:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBVF]] %buffer_1 {{.*}}
    // CHECK3-DAG: OpStore %[[V19]] %[[V16]]
    // CHECK3-DAG: %[[V20:[a-zA-Z0-9_]+]] = OpAccessChain %[[SBVF]] %buffer_1 {{.*}}
    // CHECK3-DAG: OpStore %[[V20]] %[[V18]]
    buffer.Store(0, buffer.LoadAligned<Block>(0));
    buffer.StoreAligned(16, buffer.Load<Block>(4, 16));
}

