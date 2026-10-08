//TEST:SIMPLE: -target spirv -entry main -stage fragment -capability spvDescriptorHeapEXT -o 13074-static-resource-out-param.spv
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage fragment -capability spvDescriptorHeapEXT

//CHECK-LABEL: ; Function main
//CHECK: %[[ADDRESS0:[A-Za-z0-9_]+]] = OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %uint_0
//CHECK: %[[TEXTURE0:[A-Za-z0-9_]+]] = OpLoad {{.*}} %[[ADDRESS0]]
//CHECK: %[[BEFORE:[A-Za-z0-9_]+]] = OpFunctionCall %v4float %loadGlobalTexture %[[TEXTURE0]]
//CHECK: %[[ADDRESS1:[A-Za-z0-9_]+]] = OpUntypedAccessChainKHR {{.*}} %slang_resourceHeap %uint_1
//CHECK: %[[TEXTURE1:[A-Za-z0-9_]+]] = OpLoad {{.*}} %[[ADDRESS1]]
//CHECK: %[[AFTER:[A-Za-z0-9_]+]] = OpFunctionCall %v4float %loadGlobalTexture %[[TEXTURE1]]
//CHECK: OpFAdd %v4float %[[BEFORE]] %[[AFTER]]
//CHECK-LABEL: ; Function loadGlobalTexture
//CHECK: %[[PARAM:[A-Za-z0-9_]+]] = OpFunctionParameter
//CHECK: OpImageFetch %v4float %[[PARAM]]

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
