// Regression test for the "no untyped descriptor-heap handle survives to emit" invariant
// (shader-slang/slang#11568, PR #11798 review by csyonghe).
//
// On the default (optimized) path the untyped-handle wrap/unwrap pair is adjacent and peephole
// collapses it, so `UntypedResourceHandleType` never reaches emit anyway. Under `-O0`
// (`shouldPerformMinimumOptimizations`), `simplifyIR` — and therefore peephole — does not run
// (slang-emit.cpp runs `eliminateDeadCode` instead), so the handle would otherwise survive to emit.
// The unconditional `lowerUntypedResourceHandleToUInt` pass must still remove it; without that pass
// emit/layout hit `SLANG_UNEXPECTED("...should have been lowered to uint")`. So this compiling at all
// under `-O0` is the assertion: the pass ran and the type is gone.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6 -O0
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT -O0

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Route the handle through a local `.Handle` variable so the wrap (heap subscript) and unwrap
    // (conversion to the resource) are separated by a store/load — the non-adjacent shape peephole
    // does not collapse and that -O0 keeps intact until the lowering pass.
    Texture2D.Handle th = ResourceDescriptorHeap[tid.x];
    Texture2D.Handle sh;
    sh = th;
    Texture2D t = sh;

    SamplerState.Handle samph = SamplerDescriptorHeap[tid.y];
    SamplerState s = samph;

    outputBuffer[0] = t.SampleLevel(s, float2(0.0, 0.0), 0.0).x;
}

// The emitted HLSL still uses the native SM6.6 heap syntax (the handle lowered to a `uint` index
// that feeds the heap subscript), and compiles — proving the -O0 lowering path is intact.
// HLSL: ResourceDescriptorHeap[
// HLSL: SamplerDescriptorHeap[

// SPIR-V still declares and indexes the descriptor heap; reaching valid SPIR-V at -O0 proves the
// untyped handle type was lowered before emit rather than tripping the internal-error assert.
// SPV: OpExtension "SPV_EXT_descriptor_heap"
// SPV: OpUntypedAccessChainKHR
