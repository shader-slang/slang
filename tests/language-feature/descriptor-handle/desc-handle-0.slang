//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL): -target metallib -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=SPV): -target spirv
//TEST:SIMPLE(filecheck=SPVDH): -target spirv -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SPVDHS): -target spirv -capability spvDescriptorHeapEXT -spirv-resource-heap-stride 32 -spirv-sampler-heap-stride 64
//TEST:SIMPLE(filecheck=SPVDHERR): -target spirv -capability spvDescriptorHeapEXT -spirv-resource-heap-stride -32

// SPV: OpImageSample
// HLSL: ResourceDescriptorHeap[{{.*}}]
// HLSL: SamplerDescriptorHeap[{{.*}}]
// HLSL: SampleLevel
// DXIL: computeMain
// CUDA: computeMain
// MTL: define void @computeMain

// SPVDH-DAG: OpDecorate {{.*}} BuiltIn SamplerHeapEXT
// SPVDH-DAG: OpDecorate {{.*}} BuiltIn ResourceHeapEXT

// Verify that SPIRV with manual descriptor heap stride results in correct decorations.
// SPVDHS-DAG: OpUntypedAccessChainKHR %_ptr_UniformConstant %[[RESOURCE_HEAP_ARRAY:[a-zA-Z0-9_]+]] %slang_resourceHeap
// SPVDHS-DAG: OpUntypedAccessChainKHR %_ptr_UniformConstant %[[SAMPLER_HEAP_ARRAY:[a-zA-Z0-9_]+]] %slang_samplerHeap
// SPVDHS-DAG: OpDecorate %[[SAMPLER_HEAP_ARRAY]] ArrayStride 64
// SPVDHS-DAG: OpDecorate %[[RESOURCE_HEAP_ARRAY]] ArrayStride 32

// SPVDHERR: error 37: expecting an unsigned integer value for option '-spirv-resource-heap-stride', but got '-32'

uniform StructuredBuffer<DescriptorHandle<Sampler2D>> t;
uniform DescriptorHandle<RWStructuredBuffer<float4>> buffer;

[numthreads(1,1,1)]
void computeMain()
{
    buffer[0] = t[0]->SampleLevel(float2(0.0), 0.0);
}