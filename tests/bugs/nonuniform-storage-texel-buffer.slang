//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// CHECK: OpCapability StorageTexelBufferArrayNonUniformIndexing
// CHECK-DAG: %[[TEXEL_PTR:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_UniformConstant_{{[A-Za-z0-9_]+}} %buffers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: %[[TEXEL:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[TEXEL_PTR]]
// CHECK-DAG: OpDecorate %[[TEXEL_PTR]] NonUniform
// CHECK-DAG: OpDecorate %[[TEXEL]] NonUniform

[vk::binding(0, 0)]
RWBuffer<float> buffers[8];

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint index: SV_DispatchThreadID)
{
    buffers[NonUniformResourceIndex(index)][0] = 1.0;
}
