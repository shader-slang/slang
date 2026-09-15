// shader-slang/slang#12110: on the direct SPIR-V backend, a descriptor obtained through
// `ResourceDescriptorHeap[NonUniformResourceIndex(i)]` / `SamplerDescriptorHeap[...]` (or the
// equivalent `DescriptorHandle<T>` construction) carried no `NonUniform` decoration, while a plain
// resource array (`Texture2D texArray[N]; texArray[NonUniformResourceIndex(i)]`) preserved it.
// Vulkan requires the decoration on the consumed resource operand for a wave-divergent descriptor
// index (VUID-RuntimeSpirv-None-10148); without it a driver may assume the index is dynamically
// uniform and return wrong results.
//
// The heap subscript lowers into a DescriptorHandle representation round-trip that buries the
// `NonUniformResourceIndex` wrapper inside a vector operand --
// `makeVector(NonUniformResourceIndex(i), 0)` -> `CastUInt2ToDescriptorHandle` ->
// `CastDescriptorHandleToUInt2` -> `swizzle(_, 0)` -> `getElement(heap, _)`. While that round-trip
// survived, the wrapper stalled at the `makeVector` and never reached the `getElement` index that
// the decoration machinery consumes. Two peephole rules now fold it away -- the cast-pair fold
// `CastDescriptorHandleToUIntN(CastUIntNToDescriptorHandle(x)) -> x`
// (`slang-ir-peephole.cpp`, added by shader-slang/slang#12263) and the pre-existing
// `swizzle(makeVector(a, b), 0) -> a` -- leaving the plain
// `getElement(heap, NonUniformResourceIndex(i))` shape that the float pass in
// `slang-ir-float-non-uniform-resource-index.cpp` already handles, and where the existing
// `getElement`->`getElementPtr` rewrite and `propagateNonUniformDecorations` attach `NonUniform`.
//
// This test pins that resulting behavior so a regression in either fold, or in the float pass's
// handling of the collapsed shape, is caught here.

// Default heap lowering (no spvDescriptorHeapEXT): OpCapability RuntimeDescriptorArray + a
// `__slang_resource_heap` runtime array indexed via OpAccessChain -- this is the path that requires
// the NonUniform decoration.
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
// A hand-constructed handle whose sibling lane holds a runtime value: the lane that indexes the
// heap still carries the marker through to the access chain (see mainNonConstSibling).
//TEST:SIMPLE(filecheck=SPVNONCONST): -target spirv-asm -stage compute -entry mainNonConstSibling
// A uniform heap access (no NonUniformResourceIndex) must stay undecorated (see mainUniform).
//TEST:SIMPLE(filecheck=SPVUNIFORM): -target spirv-asm -stage compute -entry mainUniform

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
// capabilities and both descriptor access chains carry the decoration (slang-test appends it inline
// as `; NonUniform`). Both heaps are emitted from the same name hint, so the disassembler
// disambiguates one of them with a `_0` suffix; which variable wins the unsuffixed name is not
// tied to source order, so these are -DAG and match either heap.
// SPV-DAG: OpCapability ShaderNonUniform
// SPV-DAG: OpCapability SampledImageArrayNonUniformIndexing
// SPV-DAG: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}}NonUniform
// SPV-DAG: OpAccessChain %{{.*}} %__slang_resource_heap_0 %{{.*}}NonUniform

// The `DescriptorHandle<T>` construction spelling: same round-trip, same required decoration. This
// entry point takes its sampler from the module-scope `samp` rather than the sampler heap, so it
// exercises a single heap access; the texture is still a sampled image, so the same
// SampledImageArrayNonUniformIndexing capability is required as in `main`.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainHandle(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), 0) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// SPVHANDLE-DAG: OpCapability ShaderNonUniform
// SPVHANDLE-DAG: OpCapability SampledImageArrayNonUniformIndexing
// SPVHANDLE-DAG: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}}NonUniform

// With spvDescriptorHeapEXT the heap is native (DescriptorHeapEXT capability), access is
// non-uniform by default, and no NonUniform decoration is required. A CHECK-NOT's window is bounded
// by the adjacent positive CHECKs, so covering the whole module takes two of them: the first spans
// the top of the module through the capability fence (catching a spurious ShaderNonUniform
// capability), and the second spans from there to OpFunctionEnd (catching any decoration or
// inline-annotated instruction in the body). The two positive checks double as fences proving the
// heap-native path was actually taken and a function body was emitted.
// SPVCAP-NOT: NonUniform
// SPVCAP: OpCapability DescriptorHeapEXT
// SPVCAP-NOT: NonUniform
// SPVCAP: OpFunctionEnd

// SPVCAPHANDLE-NOT: NonUniform
// SPVCAPHANDLE: OpCapability DescriptorHeapEXT
// SPVCAPHANDLE-NOT: NonUniform
// SPVCAPHANDLE: OpFunctionEnd

// The handle here is hand-constructed with a runtime (non-constant) second lane --
// `uint2(NonUniformResourceIndex(tid.x), tid.y)` -- where only the `.x` lane indexes the heap. The
// folds collapse the vector, so the `.y` lane never reaches the descriptor access and is dead; what
// this pins is that the surviving `.x` lane still carries the marker to the heap access chain.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainNonConstSibling(uint3 tid : SV_DispatchThreadID)
{
    Texture2D.Handle th = { uint2(NonUniformResourceIndex(tid.x), tid.y) };
    Texture2D tex = th;
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// The heap access is decorated, and the decoration lands on the lane that actually indexes it: the
// `%[[IDX]]` binding ties the decorated lane-0 extract to the decorated access chain that consumes
// it, so a decoration landing on some other value would not satisfy these.
// SPVNONCONST-DAG: OpCapability ShaderNonUniform
// SPVNONCONST-DAG: %[[IDX:[a-zA-Z0-9_]+]] = OpCompositeExtract %uint %{{.*}} 0 {{.*}}NonUniform
// SPVNONCONST-DAG: OpAccessChain %{{.*}} %__slang_resource_heap %[[IDX]] {{.*}}NonUniform

// Control case: a heap access with no `NonUniformResourceIndex` is dynamically uniform, so nothing
// here may be decorated and the NonUniform capabilities must not be required. Over-propagation
// would decorate every heap access and pull in a capability a device may not support (the heap
// analogue of tests/spirv/nonuniform-constant-index-no-leak.slang). As above, a CHECK-NOT's window
// is bounded by the adjacent positive CHECKs, so the negatives are split around the access-chain
// fence: the first group covers the top of the module (including the OpCapability and decoration
// sections), and the second covers the rest of the body through OpFunctionEnd. The positive fences
// prove the heap access really is emitted, so this is not vacuous.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainUniform(uint3 tid : SV_DispatchThreadID)
{
    Texture2D tex = ResourceDescriptorHeap[0];
    outputBuffer[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// SPVUNIFORM-NOT: OpCapability ShaderNonUniform
// SPVUNIFORM-NOT: OpCapability SampledImageArrayNonUniformIndexing
// SPVUNIFORM-NOT: NonUniform
// SPVUNIFORM: OpAccessChain %{{.*}} %__slang_resource_heap %{{.*}}
// SPVUNIFORM-NOT: NonUniform
// SPVUNIFORM: OpFunctionEnd
