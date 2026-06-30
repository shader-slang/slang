//TEST:SIMPLE(filecheck=METAL):-stage compute -entry computeMain -target metal

// Compile-only documentation pin for shader-slang/slang#8455.
//
// The runtime hang lived in the legacy `tools/gfx/renderer-shared.h::_unwrapParameterGroups`:
// the `Kind::Resource` arm used `break` (which only exited the switch) for any shape other
// than `SLANG_STRUCTURED_BUFFER`, and the enclosing `for(;;)` re-spun forever. The exact
// arm the bug rode is `Kind::Resource && shape != SLANG_STRUCTURED_BUFFER` — a typed
// `Buffer<T>` reports `SLANG_TEXTURE_BUFFER`, so a top-level `ParameterBlock<Buffer<T>>`
// reaches it directly (no `DescriptorHandle` wrapper required).
//
// slang-test render paths now flow through slang-rhi
// (`external/slang-rhi/src/shader-object.h`), whose copy of the helper is already correct,
// so the in-tree render-test cannot reproduce the hang. Runtime coverage for the legacy
// gfx library lives downstream. This test pins the language-side surface — i.e. that the
// non-StructuredBuffer Resource shape inside a `ParameterBlock` continues to compile —
// which is the only signal we can guard from this repo.

// METAL: computeMain
ParameterBlock<Buffer<uint>> param;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    outputBuffer[dtid.x] = param[dtid.x];
}
