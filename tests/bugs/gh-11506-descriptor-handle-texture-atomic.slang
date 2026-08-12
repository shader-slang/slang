// Regression for shader-slang/slang#11506: an atomic operation on a texel of a
// `DescriptorHandle<RWTexture2D<uint>>` failed to compile with E41403
// ("invalid atomic destination") ONLY when `-capability spvDescriptorHeapEXT`
// was enabled. Under that capability `processImageSubscript` rewrites the
// `IRImageSubscript` atomic destination into an `IRSPIRVLoadTexelPointerFromHeap`
// (a pointer in `AddressSpace::Image`), which the deferred atomic-destination
// validator `isValidAtomicDest` did not recognize. The texel pointer emits as
// `OpUntypedImageTexelPointerEXT` and is atomicable, so this was purely a
// validator gap; a plain `RWTexture2D<uint>` keeps its `IRImageSubscript` and
// always validated. This test locks in that the heap-lowered texel pointer is
// accepted and the atomic lowers correctly.
//
// Note: `-skip-spirv-validation` is intentional. The emitted image type carries
// format `Unknown` for a bindless handle (no R32ui default is inferred for
// descriptor-heap textures), so spirv-val rejects the atomic with
// VUID-...-OpUntypedImageTexelPointerEXT-11416. That missing-format problem is a
// separate, pre-existing bug tracked by shader-slang/slang#11130 and affects
// `DescriptorHandle` atomics with or without the capability — it is orthogonal
// to the validator gap fixed here. Once #11130 lands, the format becomes R32ui
// and validation can be enabled; the opcodes checked below are format-agnostic.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry fragmentMain -stage fragment -emit-spirv-directly -profile spirv_1_6 -capability spvDescriptorHeapEXT -skip-spirv-validation

[shader("fragment")]
void fragmentMain(float4 position : SV_Position, DescriptorHandle<RWTexture2D<uint> > tex)
{
    int2 screenpos = int2(position.xy);
    // CHECK: OpUntypedImageTexelPointerEXT
    // CHECK: OpAtomicIAdd
    InterlockedAdd(tex[screenpos], 1);
}
