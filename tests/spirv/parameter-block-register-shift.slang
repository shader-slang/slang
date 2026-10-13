//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -fvk-t-shift 300 0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -fvk-u-shift 300 0

// A `ParameterBlock` in space 1 gives each shifted member a register-space slot after its
// `ShaderResource`/`UnorderedAccess` slot. SPIR-V legalization must not treat that
// storage-class-less slot as a conflict (that aborted with E99997; same cause as
// shader-slang/slang#13096). The bindings match the `-emit-spirv-via-glsl` output.

struct P
{
    Texture2D<float4> t;
    RWStructuredBuffer<float4> o;
};
ParameterBlock<P> pb0;
ParameterBlock<P> pb1;

[numthreads(1, 1, 1)]
void main()
{
    pb0.o[0] = pb1.t.Load(int3(0, 0, 0)) + pb0.t.Load(int3(0, 0, 0));
    pb1.o[0] = 0;
}

// CHECK-NOT: E99997
// CHECK-DAG: OpDecorate %pb0_t DescriptorSet 0
// CHECK-DAG: OpDecorate %pb1_t DescriptorSet 1
// CHECK-DAG: OpDecorate %pb1_t Binding 0
// CHECK-DAG: OpDecorate %pb1_o DescriptorSet 1
// CHECK-DAG: OpDecorate %pb1_o Binding 0
