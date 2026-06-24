// Tests `-spirv-unified-descriptor-heap-stride`: every resource descriptor-heap runtime array must
// advertise one shared `ArrayStrideIdEXT` so that `index * stride` lands at the same byte offset
// regardless of which descriptor type's access chain is used. Per the `VK_EXT_descriptor_heap`
// proposal (§"Shader Model 6.6 - SamplerHeap and ResourceHeap"), that stride is a FIXED canonical
// sequence: `max(sizeof(image descriptor), sizeof(buffer descriptor))`. The host packs the resource
// heap at this stride over every resource descriptor category, so the shader emits the same maximum
// regardless of which categories it actually uses -- it is NOT adaptive to the types present.
// Because `OpConstantSizeOfEXT` is device-defined, the max is symbolic:
// `max(a, b) = Select(UGreaterThan(a, b), a, b)`.

// With the flag: both heap arrays share one ArrayStrideIdEXT, the fixed image-vs-buffer max.
//TEST:SIMPLE(filecheck=UNIFIED): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -spirv-unified-descriptor-heap-stride

// Without the flag (control): each heap array carries its own per-type OpConstantSizeOfEXT, no max chain.
//TEST:SIMPLE(filecheck=DEFAULT): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT

// Conflict: the explicit literal stride and the unified maximum stride are mutually exclusive, so
// passing both is a hard error rather than silently letting one win.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CONFLICT): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -spirv-resource-heap-stride 64 -spirv-unified-descriptor-heap-stride

// Single resource category: a shader that uses ONLY a constant buffer must STILL advertise the
// fixed image-vs-buffer max (the host's heap layout is independent of shader contents), so the
// array's stride is the full Select chain, never a lone size-of.
//TEST:SIMPLE(filecheck=SINGLE): -target spirv-asm -entry computeSingle -stage compute -capability spvDescriptorHeapEXT -spirv-unified-descriptor-heap-stride

// Both resource arrays decorate with the SAME id (decorations print before the constants section),
// and that shared id is the fixed `max(image_size, buffer_size)` Select over the two canonical
// placeholder descriptor sizes, emitted regardless of the texture/buffer types the shader uses.
// UNIFIED: OpDecorateId %{{.*}} ArrayStrideIdEXT [[MAX:%[0-9]+]]
// UNIFIED: OpDecorateId %{{.*}} ArrayStrideIdEXT [[MAX]]
// UNIFIED: OpConstantSizeOfEXT
// UNIFIED: OpConstantSizeOfEXT
// UNIFIED: OpSpecConstantOp %bool UGreaterThan
// UNIFIED: [[MAX]] = OpSpecConstantOp %uint Select

// Control: per-type size-of for each array, and crucially no unified max chain anywhere in the
// module (each array points at its own size-of, never a shared Select/UGreaterThan maximum). The
// DEFAULT-NOT lines bracket the size-of checks so a stray Select is rejected before, between, and
// after them -- robust against output reordering.
// DEFAULT-NOT: OpSpecConstantOp {{.*}} Select
// DEFAULT: OpConstantSizeOfEXT
// DEFAULT-NOT: OpSpecConstantOp {{.*}} Select
// DEFAULT: OpConstantSizeOfEXT
// DEFAULT-NOT: OpSpecConstantOp {{.*}} Select

// Conflict: combining the explicit and unified stride options is diagnosed as an error.
// CONFLICT: error[E57006]
// CONFLICT: cannot be used together

// Single resource category: even though `computeSingle` uses only a ConstantBuffer, the unified
// stride is the fixed image-vs-buffer max -- so the placeholder image descriptor type is still
// emitted (`OpTypeImage`, which a buffer-only shader would not otherwise produce) and the array's
// ArrayStrideIdEXT points at the Select chain, never a bare OpConstantSizeOfEXT.
// SINGLE: OpDecorateId %{{.*}} ArrayStrideIdEXT [[S:%[0-9]+]]
// SINGLE: OpTypeImage %float 2D 2 0 0 1 Unknown
// SINGLE: OpConstantSizeOfEXT
// SINGLE: OpConstantSizeOfEXT
// SINGLE: OpSpecConstantOp %bool UGreaterThan
// SINGLE: [[S]] = OpSpecConstantOp %uint Select

struct MyStruct
{
    uint placeholder;
    uint value;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    DescriptorHandle<Texture2D<float4>> tex,
    DescriptorHandle<ConstantBuffer<MyStruct>> cbuffer,
    RWStructuredBuffer<uint> output)
{
    float4 texel = tex.Load(int3(0, 0, 0));
    output[0] = (uint)texel.x + (*cbuffer).value;
}

// Single resource category for the SINGLE variant: only a buffer descriptor is read from the heap,
// yet the unified-stride path still emits the full image+buffer max (no image type otherwise).
[shader("compute")]
[numthreads(1, 1, 1)]
void computeSingle(
    DescriptorHandle<ConstantBuffer<MyStruct>> cbuffer,
    RWStructuredBuffer<uint> output)
{
    output[0] = (*cbuffer).value;
}
