// Regression test for #13022. A descriptor-heap `ConstantBuffer<T>` is an untyped Uniform pointer,
// so every field/element address derived from it must be an `OpUntypedAccessChainKHR`, never a typed
// `OpAccessChain` on the untyped base. These entries use the bindless path
// `DescriptorHandle<ConstantBuffer<T>>` + `getDescriptorFromHandle(...)`, which lowers to the same
// untyped descriptor-heap pointer as the reporter's `ConstantBuffer<T> cb = ResourceDescriptorHeap[i]`.
// Each entry asserts that invariant. The SPIR-V 1.6 entries also assert no typed `OpTypePointer
// Uniform` at all (structured buffers use StorageBuffer there); the SPIR-V 1.3 entry, where structured
// buffers legitimately use a Uniform pointer, asserts only that no typed Uniform pointer to the `int`
// leaf (`OpTypePointer Uniform %int`) is produced. The 1.3 entry runs with `-skip-spirv-validation`
// because SPIR-V <=1.3 requires entry-point-interface variables to be Input/Output, which the
// resource-heap `UniformConstant` variable violates -- a pre-existing constraint unrelated to the
// untyped-access-chain behavior this test checks (the module fails validation only for that
// interface rule, not for the field-address lowering under test).

//TEST:SIMPLE(filecheck=ARRAY): -target spirv-asm -entry mainArray -stage compute -profile spirv_1_6+spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=ARRAY13): -target spirv-asm -entry mainArray -stage compute -profile spirv_1_3+spvDescriptorHeapEXT -skip-spirv-validation
//TEST:SIMPLE(filecheck=NESTEDCOPY): -target spirv-asm -entry mainNestedCopy -stage compute -profile spirv_1_6+spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=ARRSMALL): -target spirv-asm -entry mainArrSmall -stage compute -profile spirv_1_6+spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=ARRBIG): -target spirv-asm -entry mainArrBig -stage compute -profile spirv_1_6+spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=NESTED): -target spirv-asm -entry mainNested -stage compute -profile spirv_1_6+spvDescriptorHeapEXT

struct MaterialParams
{
    int albedo_index;
    int normal_index;
    int rma_index;
    int padding;
}

struct MaterialParamsBuffer
{
    MaterialParams materials[4];
}

struct Inner
{
    int a;
    int b;
}

struct Outer
{
    Inner material;
}

struct NestedArrayBuffer
{
    Outer entries[3];
}

struct SmallArray
{
    int vals[3];
}

struct BigArray
{
    int vals[20];
}

// Copying an array element into a local produces a single-element `copyLogical` on the untyped
// buffer pointer; its per-field addresses must be untyped access chains (struct-field lowering path).
[shader("compute")]
[NumThreads(1, 1, 1)]
void mainArray(
    uniform DescriptorHandle<ConstantBuffer<MaterialParamsBuffer>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output,
    uint3 tid : SV_DispatchThreadID)
{
    MaterialParams m = getDescriptorFromHandle(input).materials[tid.x & 3];
    getDescriptorFromHandle(output)[0] = uint(m.albedo_index + m.normal_index + m.rma_index);
}

// The copied element's struct has a nested struct, so lowering recurses; the intermediate address
// must keep the untyped flavor or the deeper access becomes a typed chain on an untyped base.
[shader("compute")]
[NumThreads(1, 1, 1)]
void mainNestedCopy(
    uniform DescriptorHandle<ConstantBuffer<NestedArrayBuffer>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output,
    uint3 tid : SV_DispatchThreadID)
{
    Outer o = getDescriptorFromHandle(input).entries[tid.x % 3];
    getDescriptorFromHandle(output)[0] = uint(o.material.a + o.material.b);
}

// Copy a whole array wholesale into a local, then mutate it so the copy cannot fold back to direct
// buffer indexing. This drives the array-element lowering path: <=16 elements takes the unrolled
// branch, so each element address must be an untyped access chain.
[shader("compute")]
[NumThreads(1, 1, 1)]
void mainArrSmall(
    uniform DescriptorHandle<ConstantBuffer<SmallArray>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output,
    uint3 tid : SV_DispatchThreadID)
{
    int local[3] = getDescriptorFromHandle(input).vals;
    local[tid.x % 3] += 7;
    getDescriptorFromHandle(output)[0] = uint(local[tid.x % 3]);
}

// Same as above with >16 elements, so the array copy takes the loop (`emitLoopBlocks`) branch; the
// element address built inside the loop must still be an untyped access chain.
[shader("compute")]
[NumThreads(1, 1, 1)]
void mainArrBig(
    uniform DescriptorHandle<ConstantBuffer<BigArray>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output,
    uint3 tid : SV_DispatchThreadID)
{
    int local[20] = getDescriptorFromHandle(input).vals;
    local[tid.x % 20] += 7;
    getDescriptorFromHandle(output)[0] = uint(local[tid.x % 20]);
}

// Control: a nested field read with no local copy, so the leaf is retyped in place by the
// descriptor-heap load's derived-address retyping in spirv-legalize (processFieldAddress).
[shader("compute")]
[NumThreads(1, 1, 1)]
void mainNested(
    uniform DescriptorHandle<ConstantBuffer<Outer>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    getDescriptorFromHandle(output)[0] =
        uint(getDescriptorFromHandle(input).material.a + getDescriptorFromHandle(input).material.b);
}

// ARRAY-NOT: OpTypePointer Uniform
// ARRAY: OpUntypedAccessChainKHR %{{.*}} %MaterialParams_std140
// ARRAY-NOT: OpTypePointer Uniform

// Same shape below SPIR-V 1.4, where copyLogical is lowered unconditionally. Structured buffers use a
// Uniform pointer pre-1.4, so this entry pins the bug's specific symptom (a typed Uniform pointer to
// the `int` leaf) rather than any `OpTypePointer Uniform`; `%int` does not match the legitimate
// structured-buffer `%uint`/`%RWStructuredBuffer` Uniform pointers.
// ARRAY13-NOT: OpTypePointer Uniform %int
// ARRAY13: OpUntypedAccessChainKHR %{{.*}} %MaterialParams_std140
// ARRAY13-NOT: OpTypePointer Uniform %int

// NESTEDCOPY-NOT: OpTypePointer Uniform
// NESTEDCOPY: OpUntypedAccessChainKHR %{{.*}} %Inner_std140
// NESTEDCOPY-NOT: OpTypePointer Uniform

// ARRSMALL-NOT: OpTypePointer Uniform
// ARRSMALL: OpUntypedAccessChainKHR %{{.*}} %_arr_int
// ARRSMALL-NOT: OpTypePointer Uniform

// ARRBIG-NOT: OpTypePointer Uniform
// ARRBIG: OpUntypedAccessChainKHR %{{.*}} %_arr_int
// ARRBIG-NOT: OpTypePointer Uniform

// NESTED-NOT: OpTypePointer Uniform
// NESTED: OpUntypedAccessChainKHR %{{.*}} %Inner_std140
// NESTED-NOT: OpTypePointer Uniform
