// Negative test for shader-slang/slang#11568: a descriptor-heap resource-vs-sampler mismatch must
// be a compile error, not a silent runtime miscompile. Both conversion targets enforce the same
// rule, keyed off the same `__IDynamicResourceCastable<kind>` interface:
//   - a `DescriptorHandle<T>` (`.Handle`) target, via the two per-kind `extension`s.
//   - a concrete `Texture2D` / `SamplerState` target, via the per-type `isSampler` split.
// So a resource-heap handle cannot reach a sampler target and vice versa, in either case.
//
// E30019 is a semantic (type-checking) diagnostic, so it is reported module-wide regardless of the
// selected entry point — splitting into per-entry-point compiles (as `desc-heap-unsupported-target`
// does for the per-entry-point E36107 availability error) would not isolate the four cases. Instead
// the checks below are ORDERED and each pins the distinct `expected '<type>', got '<handle>'` pair
// of one assignment, so all four must independently error, in source order, for the test to pass.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -profile cs_6_6

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Handle target: a sampler-heap handle cannot build a resource (Texture2D) DescriptorHandle ...
    Texture2D.Handle bad1 = SamplerDescriptorHeap[tid.x];
    // ... and a resource-heap handle cannot build a sampler DescriptorHandle.
    SamplerState.Handle bad2 = ResourceDescriptorHeap[tid.y];

    // Concrete-type target: the same rule holds for conversion straight to a concrete resource /
    // sampler type.
    Texture2D bad3 = SamplerDescriptorHeap[tid.z];
    SamplerState bad4 = ResourceDescriptorHeap[tid.x];

    outputBuffer[0] = 0;
}

// Each of the four mismatched assignments must independently produce its own E30019. The got-type
// is the untyped heap handle keyed by which heap was indexed (`UntypedSamplerHandle` for
// `SamplerDescriptorHeap`, `UntypedResourceHandle` for `ResourceDescriptorHeap`); the expected-type
// distinguishes the `DescriptorHandle<T>` target from the bare concrete-type target. Anchoring the
// distinct pair per line means a regression that silently dropped enforcement for one of the two
// targets (e.g. `bad3`/`bad4` compiling clean) fails this test instead of passing on the surviving
// target's diagnostics, which a single un-anchored order-independent error match could not catch.
//
// bad1 — sampler heap -> resource handle target:
// CHECK: 30019
// CHECK: 'DescriptorHandle<Texture2D{{.*}}', got 'UntypedSamplerHandle'
// bad2 — resource heap -> sampler handle target:
// CHECK: 30019
// CHECK: 'DescriptorHandle<SamplerState>', got 'UntypedResourceHandle'
// bad3 — sampler heap -> concrete resource target:
// CHECK: 30019
// CHECK: 'Texture2D{{.*}}', got 'UntypedSamplerHandle'
// bad4 — resource heap -> concrete sampler target:
// CHECK: 30019
// CHECK: 'SamplerState', got 'UntypedResourceHandle'
