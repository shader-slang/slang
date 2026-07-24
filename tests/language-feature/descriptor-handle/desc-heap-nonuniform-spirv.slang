// shader-slang/slang#12110: on the direct SPIR-V backend, a descriptor obtained through
// `ResourceDescriptorHeap[NonUniformResourceIndex(i)]` / `SamplerDescriptorHeap[...]` (or the
// equivalent `DescriptorHandle<T>` construction) dropped its `NonUniform` decoration, while a plain
// resource array (`Texture2D texArray[N]; texArray[NonUniformResourceIndex(i)]`) preserved it.
// Vulkan requires the decoration on the consumed resource operand for a wave-divergent descriptor
// index (VUID-RuntimeSpirv-None-10148); without it a driver may assume the index is dynamically
// uniform and return wrong results.
//
// Root cause: SPIR-V legalization lowers the heap subscript into a DescriptorHandle round-trip
// that buries the `NonUniformResourceIndex` wrapper inside a vector operand --
// `makeVector(NonUniformResourceIndex(i), 0)` -> `CastUInt2ToDescriptorHandle` ->
// `CastDescriptorHandleToUInt2` -> `swizzle(_, 0)` -> `getElement(heap, _)`. The float pass in
// `slang-ir-float-non-uniform-resource-index.cpp` bubbles the wrapper outward through the cast,
// swizzle, and getElement ops, but had no case for `MakeVector` nor `CastUInt2ToDescriptorHandle`,
// so the wrapper stalled at the `makeVector` (its only user) and never reached the `getElement`
// index that the decoration machinery consumes. Adding those two float-pass cases lets the wrapper
// float to the index, where the existing `getElement`->`getElementPtr` rewrite and
// `propagateNonUniformDecorations` re-attach `NonUniform`.

// Default heap lowering (no spvDescriptorHeapEXT): OpCapability RuntimeDescriptorArray + a
// `__slang_resource_heap` runtime array indexed via OpAccessChain -- this is the path that requires
// the NonUniform decoration and is what #12110 fixes.
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage compute -entry main
// The `DescriptorHandle<T>` construction spelling lowers through the identical round-trip.
//TEST:SIMPLE(filecheck=SPVHANDLE): -target spirv-asm -stage compute -entry mainHandle
// With spvDescriptorHeapEXT the subscript lowers to a heap-native SPIRVLoadDescriptorFromHeap /
// OpUntypedAccessChainKHR that does NOT round-trip through makeVector/CastUInt2ToDescriptorHandle,
// and per VK_EXT_descriptor_heap resource access is non-uniform by default -- so no NonUniform
// decoration is required (or emitted) there. These directives pin that expected difference for
// both spellings (subscript via `main`, DescriptorHandle<T> construction via `mainHandle`).
//TEST:SIMPLE(filecheck=SPVCAP): -target spirv-asm -stage compute -entry main -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=SPVCAPHANDLE): -target spirv-asm -stage compute -entry mainHandle -capability spvDescriptorHeapEXT
// The MakeVector float-pass case only floats the wrapper out when every non-wrapped lane is a
// constant; this entry pins the else-branch, where a hand-constructed handle has a non-constant
// sibling lane, so the wrapper is NOT floated and no NonUniform is emitted (see mainNonConstSibling).
//TEST:SIMPLE(filecheck=SPVNONCONST): -target spirv-asm -stage compute -entry mainNonConstSibling

RWStructuredBuffer<float> outputBuffer;
SamplerState samp;

// The `ResourceDescriptorHeap[...]` / `SamplerDescriptorHeap[...]` subscript spelling.
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Texture2D tex = ResourceDescriptorHeap[NonUniformResourceIndex(tid.x)];
    SamplerState s = SamplerDescriptorHeap[NonUniformResourceIndex(tid.y)];
    outputBuffer[0] = tex.SampleLevel(s, float2(0, 0), 0).x;
}

// Both heap accesses must be marked NonUniform, so the compiler declares the matching NonUniform
// capabilities and both descriptor access chains into `__slang_resource_heap` carry the decoration
// (the disassembler prints it as a trailing `; NonUniform` comment on the access-chain line). The
// texture heap (`%__slang_resource_heap`) is accessed before the sampler heap
// (`%__slang_resource_heap_0`), matching source order.
// SPV-DAG: OpCapability ShaderNonUniform
// SPV-DAG: OpCapability SampledImageArrayNonUniformIndexing
// SPV: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}} ; NonUniform
// SPV: OpAccessChain %{{.*}} %__slang_resource_heap_0 %{{.*}} ; NonUniform

// The `DescriptorHandle<T>` construction spelling: same round-trip, same required decoration.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainHandle(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), 0) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// SPVHANDLE-DAG: OpCapability ShaderNonUniform
// SPVHANDLE: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}} ; NonUniform

// With spvDescriptorHeapEXT the heap is native (DescriptorHeapEXT capability), access is
// non-uniform by default, and no NonUniform decoration is required. Pin the heap-native capability
// and assert the decoration is absent for both spellings, so the without-capability fix does not
// start emitting a redundant decoration here.
// SPVCAP: OpCapability DescriptorHeapEXT
// SPVCAP-NOT: NonUniform

// SPVCAPHANDLE: OpCapability DescriptorHeapEXT
// SPVCAPHANDLE-NOT: NonUniform

// Guard test for the MakeVector float-pass case: it only floats the wrapper out of a
// `makeVector(..., NonUniformResourceIndex(x), ...)` when every other lane is a constant. Here the
// handle is hand-constructed with a runtime (non-constant) second lane -- `uint2(NUR(tid.x), tid.y)`
// -- so the else-branch fires and the wrapper is left in place rather than marking the whole vector
// non-uniform. This is a deliberately conservative under-decoration: only the `.x` lane indexes the
// heap, but the pass declines to reason per-lane here to avoid over-decorating a genuinely uniform
// sibling in the general case. The common heap-subscript / `{ uint2(NUR(i), 0) }` forms always have
// a constant sibling, so they are unaffected (covered by SPV / SPVHANDLE above). If a future change
// inverts or drops the constant-lane guard, this `CHECK-NOT` fails, flagging the regression.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainNonConstSibling(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), tid.y) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// A lone CHECK-NOT scans the whole disassembly, so this asserts NonUniform appears nowhere (no
// ShaderNonUniform capability, no OpDecorate NonUniform, no inline `; NonUniform`) -- confirming the
// guard left the wrapper unfloated. slang-test independently fails the test if compilation errors,
// so a passing run also means the heap access was emitted.
// SPVNONCONST-NOT: NonUniform
