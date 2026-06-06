//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT -stage fragment -entry main
//TEST:SIMPLE(filecheck=SAMP):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT -stage fragment -entry sampMain
//TEST:SIMPLE(filecheck=BUF):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT -stage fragment -entry bufMain

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

// Inside the specialized callee, the reconstructed heap load reads from the
// heap builtin global (`%slang_resourceHeap`) — the access-chain base must NOT
// be a `uint` OpFunctionParameter, which is invalid SPIR-V. Strict-ordered
// CHECKs walk forward from the function header to bind the asserts to
// bad_function specifically:
// CHECK: %bad_function = OpFunction
// CHECK: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap
// CHECK: OpImageSampleImplicitLod

// ----------------------------------------------------------------------------
// End-to-end smoke for the sampler-heap variant. `DescriptorHandle<SamplerState>`
// also lowers to `__spirvLoadDescriptorFromHeap` but with `__spirvSamplerHeap()`
// as the heap operand. For a non-array `SamplerState` param,
// `isIllegalSPIRVParameterType` returns false, so the sampler does not flow
// through `ResourceParameterSpecializationCondition`; the sampler-heap load is
// emitted in the caller and the materialized sampler is passed by value. This
// SAMP variant locks in that the combined texture+sampler heap-EXT path emits
// without regressing — resource heap reached via the specialized callee,
// sampler heap reached via the caller, and an actual sample op present.
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

// ----------------------------------------------------------------------------
// Buffer-load-arg path. A `[noinline]` callee taking a deferable struct param,
// called with the result of `pc.sb[0]` where `pc.sb` is
// `DescriptorHandle<StructuredBuffer<BigStruct>>`, exercises the heap-load arm
// added to `FuncBufferLoadSpecializationCondition::doesParamWantSpecialization`.
// `BigStruct` is large enough that `isTypePreferrableToDeferLoad` returns true,
// so this is the entry point Site 4 was designed for: the access-chain root of
// the deferable arg is `kIROp_SPIRVLoadDescriptorFromHeap`, the new arm returns
// true at the root matcher, and the `getCallInfoForArg` /
// `getSpecializedValueForArg` recursion (re-using the heap global, parameterizing
// only the index) reconstructs the heap load inside the cloned callee.
// ----------------------------------------------------------------------------

struct BigStruct
{
    float4 a[8];
    float4 b[8];
    float4 c[8];
};

[noinline]
float useStruct(BigStruct s)
{
    return s.a[0].x + s.b[1].y + s.c[2].z;
}

struct PCBuf
{
    DescriptorHandle<StructuredBuffer<BigStruct>> sb;
};

[shader("fragment")]
float bufMain(uniform PCBuf pc) : SV_Target
{
    BigStruct v = pc.sb[0];
    return useStruct(v);
}

// BUF-DAG: OpCapability DescriptorHeapEXT
// BUF-DAG: OpExtension "SPV_EXT_descriptor_heap"
// Inside the specialized callee, the heap load is reconstructed in-block from
// the heap global, exactly like the texture path above:
// BUF: %useStruct = OpFunction
// BUF: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap
