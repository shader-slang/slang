// Regression test for shader-slang/slang#12897: a bindless RWStructuredBuffer<T>.Handle read with a
// dynamic (runtime) heap index must request GL_EXT_nonuniform_qualifier so glslang accepts the
// variable index into the unsized buffer-heap array.
//
// Keep this test buffer-only (no texture/sampler heap): a texture or sampler heap requests the
// extension on its own and would mask a missing request on the buffer path.
//
// The load-bearing regression signal is that -emit-spirv-via-glsl compiles at all: without the
// extension request glslang rejects the variable heap index and the run emits nothing for FileCheck.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-via-glsl

uniform RWStructuredBuffer<float>.Handle bindless;
RWStructuredBuffer<float> gOutput;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 d : SV_DispatchThreadID)
{
    gOutput[d.x] = bindless[d.x];
}

// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: _slang_resource_heap{{[0-9_]*}}[{{.*}}]._data[

// GLSLSPV: OpAccessChain{{.*}}%_slang_resource_heap
