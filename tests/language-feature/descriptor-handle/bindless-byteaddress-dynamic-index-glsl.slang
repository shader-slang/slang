// Regression test for shader-slang/slang#12897 (byte-address companion of
// bindless-buffer-dynamic-index-glsl.slang): a bindless RWByteAddressBuffer.Handle read with a
// dynamic (runtime) heap index must request GL_EXT_nonuniform_qualifier so glslang accepts the
// variable index into the unsized buffer-heap array.
//
// Byte-address buffers use the distinct _emitGLSLByteAddressBuffer early-return path; it must not
// bypass the shared unsized-resource-array extension request that this test pins (the sibling
// bindless-buffer-dynamic-index-glsl.slang covers the structured-buffer path).
//
// Keep this test buffer-only (no texture/sampler heap): a texture or sampler heap requests the
// extension on its own and would mask a missing request on the byte-address path. The load-bearing
// regression signal is that -emit-spirv-via-glsl compiles at all.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-via-glsl

uniform RWByteAddressBuffer.Handle bindless;
RWStructuredBuffer<uint> gOutput;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 d : SV_DispatchThreadID)
{
    gOutput[d.x] = bindless.Load<uint>(d.x * 4);
}

// GLSL: #extension GL_EXT_nonuniform_qualifier : require
// GLSL: _slang_resource_heap{{[0-9_]*}}[{{.*}}]._data[

// GLSLSPV: OpAccessChain{{.*}}%_slang_resource_heap
