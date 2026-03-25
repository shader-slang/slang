//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Test that NonUniform decorations and texel buffer capabilities are emitted
// for non-uniformly indexed Buffer<T> and RWBuffer<T> arrays.

// CHECK-DAG: OpCapability UniformTexelBufferArrayNonUniformIndexing
// CHECK-DAG: OpCapability StorageTexelBufferArrayNonUniformIndexing

// Uniform texel buffer: access chain, load, and image fetch must be NonUniform.
// CHECK-DAG: %[[UBAC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %uniformBufs %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[UBAC]] NonUniform
// CHECK-DAG: %[[UBLOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[UBAC]]
// CHECK-DAG: OpDecorate %[[UBLOAD]] NonUniform

// Storage texel buffer: access chain and loaded image must be NonUniform.
// CHECK-DAG: %[[SBAC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %storageBufs %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[SBAC]] NonUniform
// CHECK-DAG: %[[SBLOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[SBAC]]
// CHECK-DAG: OpDecorate %[[SBLOAD]] NonUniform

Buffer<float> uniformBufs[8];
RWBuffer<float> storageBufs[8];
RWTexture1D<float> output;

cbuffer Params { uint idx; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint i = idx + tid.x;
    float v = uniformBufs[NonUniformResourceIndex(i)].Load(0);
    storageBufs[NonUniformResourceIndex(i)][0] = v;
    output[0] = v;
}
