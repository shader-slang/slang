// Regression test for shader-slang/slang#12897:
// A bindless storage-buffer descriptor-heap handle (`RWStructuredBuffer<T>.Handle`) read with a
// DYNAMIC (runtime) heap index emitted GLSL that glslang rejected, because the GLSL emitter never
// requested `GL_EXT_nonuniform_qualifier` for the bindless buffer-heap declaration.
//
// The emitter requests that extension for any unsized resource array, but the request lived after
// the point where structured/byte-address/parameter-group types are intercepted and emitted, so
// buffer heaps returned early and skipped it. Texture/sampler heaps fall through and reach the
// request, which is why a shader that also uses a texture heap (e.g. desc-heap-direct-index.slang)
// incidentally compiled while a buffer-only shader like this one did not.
//
// This test uses ONLY a bindless buffer heap (no texture/sampler) so the buffer path is not masked
// by an unrelated extension request.

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

// The bindless buffer heap must request the nonuniform-qualifier extension so glslang accepts the
// variable heap index below; without this line glslang treats `_slang_resource_heap[]` as an
// implicitly-sized array and rejects the variable index.
// GLSL: #extension GL_EXT_nonuniform_qualifier : require

// The heap is a well-formed unsized array of buffer blocks, indexed by the dynamic handle value and
// then dereferenced through `._data[...]`. Pinning the dereference confirms the emit is structurally
// correct (the issue's second reported symptom was only glslang's error-recovery cascade of the
// missing extension, not a real dereference bug).
// GLSL: _slang_resource_heap{{[0-9_]*}}[{{.*}}]._data[

// Routing the emitted GLSL through glslang (`-emit-spirv-via-glsl`) must now succeed and produce an
// access chain for the heap indexing, proving glslang accepted the variable index.
// GLSLSPV: OpAccessChain
