//TEST:SIMPLE: -target spirv -entry main -stage fragment -capability spvDescriptorHeapEXT -o 13074-static-resource-out-param.spv
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -capability spvDescriptorHeapEXT

// `main` first selects heap resource 0. The nested `loadThenSelectGlobalTexture` call must read that
// binding before changing the selection to heap resource 1, and the final read must use resource 1.
// The checks distinguish the two reads and prove their order: each read is a call to
// `loadGlobalTexture` with the selected heap index, so the first call must carry 0 and the second 1.
// The second call carrying 1 also proves the `out` update to `selectedTexture` was threaded through
// the helper calls: the final read uses the binding the nested helper selected.

//CHECK-LABEL: ; Function main
//CHECK: %[[BEFORE:[A-Za-z0-9_]+]] = OpFunctionCall %v4float %loadGlobalTexture %uint_0
//CHECK: %[[AFTER:[A-Za-z0-9_]+]] = OpFunctionCall %v4float %loadGlobalTexture %uint_1
//CHECK: OpFAdd %v4float %[[BEFORE]] %[[AFTER]]
//CHECK-LABEL: ; Function loadGlobalTexture
//CHECK: %[[INDEX:[A-Za-z0-9_]+]] = OpFunctionParameter %uint
//CHECK: OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %[[INDEX]]
//CHECK: OpImageFetch %v4float {{.*}} Lod

static Texture2D<float4> selectedTexture;

void selectTexture(out Texture2D<float4> result, uint index)
{
    result = ResourceDescriptorHeap[index];
}

[noinline]
void selectGlobalTexture(uint index)
{
    selectTexture(selectedTexture, index);
}

[noinline]
float4 loadGlobalTexture()
{
    return selectedTexture.Load(int3(0, 0, 0));
}

[noinline]
float4 loadThenSelectGlobalTexture(uint index)
{
    float4 result = loadGlobalTexture();
    selectGlobalTexture(index);
    return result;
}

[shader("fragment")]
float4 main() : SV_Target
{
    selectGlobalTexture(0);
    float4 before = loadThenSelectGlobalTexture(1);
    float4 after = loadGlobalTexture();
    return before + after;
}
