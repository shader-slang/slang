// `SubpassInput` is not NV-encodable: it declares `DescriptorKind.Texture` but lowers to
// `IRSubpassInputType`, not `IRTextureType`, so its `DescriptorHandle` is a `uint2` heap index, never a
// native `uint64` bindless handle. Under `spvBindlessTextureNV`+`spvDescriptorHeapEXT` both a
// directly-bound `.Handle` uniform and an explicit `ResourceDescriptorHeap[i]` index must therefore load
// from the EXT descriptor heap (not the NV conversion), and the explicit-index conversion must not be
// diagnosed (the diagnostic is only for native texture/sampler handles). See shader-slang/slang#13070.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage fragment -entry fragMain
//TEST:SIMPLE(filecheck=NONV):  -target spirv-asm -capability spvBindlessTextureNV+spvDescriptorHeapEXT -stage fragment -entry fragMain

[[vk::binding(0)]] RWStructuredBuffer<float> outputBuffer;
uniform uint gIndex;
uniform SubpassInput<float4>.Handle subpass;

[shader("fragment")]
float4 fragMain() : SV_Target
{
    // Directly-bound `.Handle` uniform -> loaded from the EXT heap (not an NV conversion).
    SubpassInput<float4> a = subpass;

    // Explicit heap index stored in a SubpassInput `.Handle`: SubpassInput is not NV-encodable, so this
    // is a valid `uint2` index conversion and must NOT be diagnosed (a native texture handle here would
    // be). If it were wrongly diagnosed, this SIMPLE test would fail to compile.
    SubpassInput<float4>.Handle h = ResourceDescriptorHeap[gIndex];
    SubpassInput<float4> b = h;

    return a.SubpassLoad() + b.SubpassLoad();
}

// The subpass-input descriptors are loaded from the EXT resource heap (untyped access chains into the
// variable decorated `BuiltIn ResourceHeapEXT`):
//CHECK-DAG: OpDecorate %slang_resourceHeap BuiltIn ResourceHeapEXT
//CHECK-DAG: OpUntypedAccessChainKHR {{.*}}%slang_resourceHeap

// No native NV conversion is emitted anywhere. The `OpEntryPoint` anchor precedes every function body,
// and this prefix has no following positive check, so the `-NOT` range runs from there to end-of-module.
//NONV: OpEntryPoint Fragment
//NONV-NOT: OpConvertUToImageNV
