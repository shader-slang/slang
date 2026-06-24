// Tests `-spirv-unified-descriptor-heap-stride`: when a single resource descriptor heap holds a
// mix of descriptor types (here an image and a buffer descriptor), every per-type runtime array
// must advertise one shared maximum `ArrayStride` so that `index * stride` lands at the same byte
// offset regardless of which type's access chain is used. The maximum is emitted symbolically as
// an `OpSpecConstantOp` chain `max(a, b) = Select(UGreaterThan(a, b), a, b)` over the per-type
// `OpConstantSizeOfEXT` values, because those sizes are device-defined and cannot be folded at
// compile time.

// With the flag: both heap arrays share one ArrayStrideIdEXT, computed as the symbolic max.
//TEST:SIMPLE(filecheck=UNIFIED): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -spirv-unified-descriptor-heap-stride

// Without the flag (control): each heap array carries its own per-type OpConstantSizeOfEXT, no max chain.
//TEST:SIMPLE(filecheck=DEFAULT): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT

// Explicit stride wins over the unified flag: a literal ArrayStride, no size-of, no max chain.
//TEST:SIMPLE(filecheck=OVERRIDE): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -spirv-resource-heap-stride 64 -spirv-unified-descriptor-heap-stride

// Single resource type: the degenerate max is just the one size-of, with no Select chain.
//TEST:SIMPLE(filecheck=SINGLE): -target spirv-asm -entry computeSingle -stage compute -capability spvDescriptorHeapEXT -spirv-unified-descriptor-heap-stride

// Both resource arrays decorate with the SAME id (decorations print before the constants section),
// and that shared id is the result of the max Select over the two per-type size-of values.
// UNIFIED: OpDecorateId %{{.*}} ArrayStrideIdEXT [[MAX:%[0-9]+]]
// UNIFIED: OpDecorateId %{{.*}} ArrayStrideIdEXT [[MAX]]
// UNIFIED: OpConstantSizeOfEXT
// UNIFIED: OpConstantSizeOfEXT
// UNIFIED: OpSpecConstantOp %bool UGreaterThan
// UNIFIED: [[MAX]] = OpSpecConstantOp %uint Select

// Control: per-type size-of for each array, and crucially no unified max chain anywhere in the
// module (each array points at its own size-of, never a shared Select/UGreaterThan maximum). The
// DEFAULT-NOT lines bracket the size-of checks so a stray Select is rejected before, between, and
// after them — robust against output reordering.
// DEFAULT-NOT: OpSpecConstantOp {{.*}} Select
// DEFAULT: OpConstantSizeOfEXT
// DEFAULT-NOT: OpSpecConstantOp {{.*}} Select
// DEFAULT: OpConstantSizeOfEXT
// DEFAULT-NOT: OpSpecConstantOp {{.*}} Select

// Override: both heap arrays use the explicit literal stride; the size-of/max path is bypassed.
// OVERRIDE: OpDecorate %{{.*}} ArrayStride 64
// OVERRIDE: OpDecorate %{{.*}} ArrayStride 64
// OVERRIDE-NOT: OpConstantSizeOfEXT
// OVERRIDE-NOT: OpSpecConstantOp {{.*}} Select

// Single resource type: the per-type max degenerates to the lone size-of, so the array's
// ArrayStrideIdEXT points straight at the OpConstantSizeOfEXT with no Select/UGreaterThan chain.
// SINGLE: OpDecorateId %{{.*}} ArrayStrideIdEXT [[S:%[0-9]+]]
// SINGLE: [[S]] = OpConstantSizeOfEXT
// SINGLE-NOT: OpSpecConstantOp {{.*}} Select

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(
    DescriptorHandle<Texture2D<float>> tex,
    DescriptorHandle<RWStructuredBuffer<uint>> buf)
{
    buf[0] = (uint)tex.Load(int3(0, 0, 0));
}

// Single resource type for the SINGLE variant: only a buffer descriptor is loaded, so the
// heap holds one per-type runtime array and the unified-stride path produces no max chain.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeSingle(DescriptorHandle<RWStructuredBuffer<uint>> buf)
{
    buf[0] = 1u;
}
