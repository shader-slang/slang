//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT -stage fragment -entry main
//TEST:SIMPLE(filecheck=SAMP):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT -stage fragment -entry sampMain

// Regression test for shader-slang/slang#11498.
//
// A `[noinline]` callee that takes a `Texture2D` parameter and is called with
// `DescriptorHandle<Texture2D>` under -capability spvDescriptorHeapEXT used to
// crash mid-emit (orphan IRParam reaching emitSpvAsmOperand). The texture
// argument lowers to kIROp_SPIRVLoadDescriptorFromHeap, which the function-call
// specialization allowlists did not accept, leaving the texture parameter
// unspecialized and its asm-operand back-reference desynced by later passes.
//
// Primary regression guard: the shader compiles to SPIR-V at all (was SIGSEGV).
// The CHECK lines below confirm we stayed on the descriptor-heap-EXT path AND
// that the specialized callee's `OpUntypedAccessChainKHR` reads from the heap
// builtin global (`%slang_resourceHeap`), not from a `uint` `OpFunctionParameter`
// — the invalid-SPIR-V failure mode that an early version of the fix produced.
//
// The `float4 values[N]` parameter on `bad_function` is load-bearing for the
// repro: the deferable array argument triggers `specializeFuncsForBufferLoadArgs`,
// which clones `bad_function`. Combined with `ResourceParameterSpecializationCondition`
// firing on the `Texture2D` param under -target spirv, that clone path is what
// previously left the texture parameter as an unspecialized IRParam in the
// cloned callee body.

SamplerState sampler;
static const uint N = 2;

[noinline]
float4 bad_function(Texture2D tex, float4 values[N])
{
    return tex.Sample(sampler, float2(0.0));
}

struct Data { float4 values[N]; };
struct PushConstants {
    DescriptorHandle<Texture2D> tex;
    ImmutablePtr<Data> data;
};

[shader("fragment")]
float4 main(uniform PushConstants pc) : SV_Target
{
    return bad_function(pc.tex, pc.data->values);
}

// Stayed on the descriptor-heap-EXT path:
// CHECK-DAG: OpCapability DescriptorHeapEXT
// CHECK-DAG: OpExtension "SPV_EXT_descriptor_heap"

// Inside the specialized callee, the reconstructed heap load uses the heap
// builtin global as the access-chain base, NOT a uint OpFunctionParameter
// (an early version of the fix produced the latter, which is invalid SPIR-V).
// The strict-ordered CHECKs walk the function body forward to bind the asserts
// to bad_function specifically:
// CHECK: %bad_function = OpFunction
// CHECK: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap
// CHECK: OpImageSampleImplicitLod

// ----------------------------------------------------------------------------
// End-to-end smoke for the sampler-heap variant. `DescriptorHandle<SamplerState>`
// also lowers to `__spirvLoadDescriptorFromHeap` but with `__spirvSamplerHeap()`
// as the heap operand (`hlsl.meta.slang:27249`). For a non-array `SamplerState`
// param, `isIllegalSPIRVParameterType` returns false, so the sampler doesn't go
// through Sites 1-3 directly; the sampler-heap load is emitted in the caller
// and the materialized sampler is passed by value. This SAMP variant locks in
// that the combined texture+sampler heap-EXT path emits without regressing —
// resource heap reached via the specialized callee, sampler heap reached via
// the caller, and an actual sample op present.
// ----------------------------------------------------------------------------

[noinline]
float4 sampleWith(Texture2D tex, SamplerState s)
{
    return tex.Sample(s, float2(0.0));
}

struct PCSamp
{
    DescriptorHandle<Texture2D> tex;
    DescriptorHandle<SamplerState> samp;
};

[shader("fragment")]
float4 sampMain(uniform PCSamp pc) : SV_Target
{
    return sampleWith(pc.tex, pc.samp);
}

// SAMP-DAG: OpCapability DescriptorHeapEXT
// SAMP-DAG: OpExtension "SPV_EXT_descriptor_heap"
// SAMP-DAG: OpUntypedAccessChainKHR {{.*}} %slang_samplerHeap
// SAMP-DAG: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap
// SAMP-DAG: %sampleWith = OpFunction
// SAMP-DAG: OpImageSampleImplicitLod
