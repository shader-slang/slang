// Regression/feature test for shader-slang/slang#11568:
// Allow HLSL SM6.6 ResourceDescriptorHeap[i] / SamplerDescriptorHeap[i] directly in Slang source.
// The resource type is recovered from the assignment target via implicit conversion from the
// untyped heap handle, then rides the existing DescriptorHandle<T> lowering on every gated target.
// The emitted HLSL and GLSL are additionally sent through DXC (-target dxil) and glslang
// (-target spirv-asm -emit-spirv-via-glsl) so downstream compilability is verified, not just text.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // T is solved from the declared variable type, not from the index argument.
    Texture2D t = ResourceDescriptorHeap[tid.x];
    SamplerState s = SamplerDescriptorHeap[tid.y];
    // A non-texture resource kind takes a different legalization path (structured buffer).
    RWStructuredBuffer<float> buf = ResourceDescriptorHeap[tid.z];
    // The .Handle (DescriptorHandle<T>) conversion spelling.
    Texture2D.Handle th = ResourceDescriptorHeap[tid.x];
    Texture2D t2 = th;
    outputBuffer[0] = t.SampleLevel(s, float2(0.0, 0.0), 0.0).x
        + t2.SampleLevel(s, float2(0.0, 0.0), 0.0).x
        + buf[0];
}

// Each spelling must emit a heap index derived from the corresponding thread component, so the
// checks pin the thread component inside the bracket (`.x` texture, `.y` sampler, `.z` buffer),
// not just the bare prefix, proving the three accesses route distinct indices.
// HLSL: ResourceDescriptorHeap[{{.*}}.x{{.*}}]
// HLSL: SamplerDescriptorHeap[{{.*}}.y{{.*}}]
// HLSL: ResourceDescriptorHeap[{{.*}}.z{{.*}}]

// Compiling that emitted HLSL through DXC (-target dxil) must succeed and produce the entry point,
// proving the SM6.6 heap syntax we emit is accepted by the downstream compiler, not just textual.
// DXIL: define void @computeMain()

// The SPIR-V output must declare the extension, declare both heap built-ins, and actually
// index each heap via an untyped access chain (not merely declare the extension).
// SPV: OpExtension "SPV_EXT_descriptor_heap"
// SPV: BuiltIn ResourceHeapEXT
// SPV: BuiltIn SamplerHeapEXT
// SPV: OpUntypedAccessChainKHR{{.*}}%slang_resourceHeap
// SPV: OpUntypedAccessChainKHR{{.*}}%slang_samplerHeap

// The emitted GLSL is compiled through glslang (-target spirv-asm -emit-spirv-via-glsl), so the
// resulting SPIR-V both proves glslang accepted the GLSL and shows the heap access-chain / load /
// sample sequence the descriptor-handle path lowers to.
// GLSLSPV: OpAccessChain
// GLSLSPV: OpLoad
// GLSLSPV: OpImageSampleExplicitLod

// WGSL stays a text-emit check (no downstream WGSL compiler here): it must declare a
// `_slang_resource_heap` array and index it with a thread-derived heap value, matching the
// `DescriptorHandle<T>` path in desc-handle-3.slang — verifying the path emits, not merely gates.
// WGSL: _slang_resource_heap{{[0-9_]+}}{{.*}}array<texture_2d
// WGSL: _slang_resource_heap{{[0-9_]+}}[vec2<u32>(tid
