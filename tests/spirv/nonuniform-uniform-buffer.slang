//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly

// Regression test: a ConstantBuffer (uniform buffer) array indexed with
// NonUniformResourceIndex must emit UniformBufferArrayNonUniformIndexing. By the
// time NonUniform is emitted the ConstantBuffer has been lowered to a plain
// struct, so the capability is recovered from the lowered struct's Block
// decoration rather than the (erased) ConstantBuffer type.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability UniformBufferArrayNonUniformIndexing

// The access chain into the ConstantBuffer array must be NonUniform.
// CHECK-DAG: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %cbs %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[AC]] NonUniform

// The inner field-access pointer and the load that reads the field must also be
// NonUniform per VUID-RuntimeSpirv-None-10148, not just the outer array chain.
// CHECK-DAG: %[[FIELD:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %[[AC]] %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[FIELD]] NonUniform
// CHECK-DAG: %[[LOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[FIELD]]
// CHECK-DAG: OpDecorate %[[LOAD]] NonUniform

struct Foo { uint x; };
ConstantBuffer<Foo> cbs[];
RWStructuredBuffer<uint> outputBuffer;

cbuffer Params { uint idx; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = cbs[NonUniformResourceIndex(idx + tid.x)].x;
}
