//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute

// Regression test for https://github.com/shader-slang/slang/issues/13072
//
// Arithmetic applied to a NonUniformResourceIndex(...) result must keep the
// resulting descriptor access NonUniform: nonuniformity is contagious (Vulkan
// VUID-RuntimeSpirv-None-10148), so an integer op on a non-uniform index still
// produces a non-uniform index. This must hold along both lowering paths an
// arithmetic index can take:
//  * buffers[NonUniformResourceIndex(i) * i][0] (the exact reported form) lowers to a
//    call whose index argument is the arithmetic value; the function-call specializer
//    must re-mark the index parameter inside the callee.
//  * rwbufs[...][0] = v keeps the resource getElement in this function, so the
//    NonUniformResourceIndex wrapper is floated through the arithmetic directly. Its
//    index chains all twelve ops of isNonUniformIndexArithmeticOp (neg, bit-not, mul,
//    div, irem, add, sub, shifts, and/or/xor), so removing any op from that predicate
//    fails the %[[SBAC]] NonUniform check.
// The third access is a width-changing call-boundary case (a uint64 index whose
// non-uniform part is a uint shift count) so the re-inserted wrapper must take the
// parameter's type, not the narrower inner wrapper's.
//
// For SPIR-V, Slang emits the NonUniform decoration + capabilities directly. For
// textual targets the specializer/float pass instead keeps the wrapper on the index
// expression so DXC/glslang can propagate it, which the CHECK_HLSL / CHECK_GLSL lines
// verify on the call-boundary access.

// CHECK-DAG: OpCapability ShaderNonUniform
// CHECK-DAG: OpCapability UniformTexelBufferArrayNonUniformIndexing
// CHECK-DAG: OpCapability StorageBufferArrayNonUniformIndexing

// Call-boundary path (reported form): the texel-buffer access chain and its load are NonUniform.
// CHECK-DAG: %[[UBAC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %buffers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[UBAC]] NonUniform
// CHECK-DAG: %[[UBLOAD:[A-Za-z0-9_]+]] = OpLoad %{{[A-Za-z0-9_]+}} %[[UBAC]]
// CHECK-DAG: OpDecorate %[[UBLOAD]] NonUniform

// Direct-getElement path (all twelve predicate ops): the storage-buffer access chain is NonUniform.
// CHECK-DAG: %[[SBAC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %rwbufs %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[SBAC]] NonUniform

// Width-changing call-boundary access: a second NonUniform access chain into the buffer array.
// CHECK-DAG: %[[WBAC:[A-Za-z0-9_]+]] = OpAccessChain %{{[A-Za-z0-9_]+}} %buffers %{{[A-Za-z0-9_]+}}
// CHECK-DAG: OpDecorate %[[WBAC]] NonUniform

// Textual targets keep the wrapper on the index expression (DXC/glslang propagate it).
// Anchored to the call-boundary multiply so another indexed access cannot satisfy it.
// CHECK_HLSL: buffers{{[0-9A-Za-z_]*}}[NonUniformResourceIndex({{[0-9A-Za-z_]*}}) *
// CHECK_GLSL: buffers{{[0-9A-Za-z_]*}}[nonuniformEXT({{[0-9A-Za-z_]*}}) *

Buffer<float> buffers[8];
RWStructuredBuffer<float> rwbufs[8];
RWStructuredBuffer<float> outputBuffer;

cbuffer Params
{
    uint index;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // A genuinely non-uniform index (varies per invocation).
    uint i = index + tid.x;

    // Call-boundary path (the exact reported form): the Buffer<T> subscript lowers to
    // a call whose index argument is the arithmetic value.
    float v = buffers[NonUniformResourceIndex(i) * i][0];

    // Direct-getElement path: float the wrapper through every op in the shared
    // predicate. Each step keeps the running value non-uniform, and the final `& 7`
    // keeps the index in range for the size-8 array.
    uint n = NonUniformResourceIndex(i);
    int s = -int(n);
    uint a = ~(uint)s;
    uint b = (a * 3u) / 2u;
    uint c = (b % 9u) + i;
    uint d = (c - 1u) << 1u;
    uint e = (d >> 2u) & 6u;
    uint f = (e | 1u) ^ 2u;
    rwbufs[f & 7u][0] = v;

    // Width-changing call-boundary path: a uint64 index whose non-uniform part is a
    // uint shift count.
    uint64_t w = (uint64_t(i) << NonUniformResourceIndex(i)) & 7;
    v += buffers[w][0];

    outputBuffer[0] = v;
}
